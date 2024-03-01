#include <sys/timerfd.h>
#include <string.h>
#include "rocket/net/timer.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket {


Timer::Timer() : FdEvent() {


  m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC); //  创建一个timerfd
  DEBUGLOG("timer fd=%d", m_fd);

  // 把 fd 可读事件放到了 eventloop 上监听
  listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));  //  将该IN_EVENT状态发生时的回调函数绑定上
}

Timer::~Timer() {
}

//  回调函数
void Timer::onTimer() {
  // 处理缓冲区数据，防止下一次继续触发可读事件
  DEBUGLOG("ontimer");
  char buf[8];
  while(1) {
    if ((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
      break;
    }
  }

  // 执行定时任务
  int64_t now = getNowMs();

  std::vector<TimerEvent::s_ptr> tmps;
  std::vector<std::pair<int64_t, std::function<void()>>> tasks;

  ScopeMutex<Mutex> lock(m_mutex);
  auto it = m_pending_events.begin();

  //  multimap是可以进行自动排序的，因此可以直接从前往后遍历，如果遍历到未达到限定时间的事件之后就可以直接退出了
  for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
    if ((*it).first <= now) { //  说明已经达到限定时间
      if (!(*it).second->isCancled()) { //  如果该事件没有被去除
        tmps.push_back((*it).second); //  记录遍历过的Event
        tasks.push_back(std::make_pair((*it).second->getArriveTime(), (*it).second->getCallBack()));  //  加入处理队列
      }
    } else {
      break;
    }
  }

  m_pending_events.erase(m_pending_events.begin(), it); //  删除已经达到限定时间的事件
  lock.unlock();


  // 需要把允许重复的Event 再次添加进去
  for (auto i = tmps.begin(); i != tmps.end(); ++i) {
    if ((*i)->isRepeated()) {
      // 调整 arriveTime
      (*i)->resetArriveTime();
      addTimerEvent(*i);
    }
  }

  resetArriveTime();  //  执行了一次Ontime后，需要设定下一次OnTime什么时候执行，取决与定时事件组中第一个事件的到达时间

  for (auto i: tasks) { //  处理事件
    if (i.second) {
      i.second();
    }
  }

}

void Timer::resetArriveTime() {
  ScopeMutex<Mutex> lock(m_mutex);
  auto tmp = m_pending_events;
  lock.unlock();

  if (tmp.size() == 0) {
    return;
  }

  int64_t now = getNowMs();

  auto it = tmp.begin();
  int64_t inteval = 0;
  if (it->second->getArriveTime() > now) {
    inteval = it->second->getArriveTime() - now;
  } else {
    inteval = 100;
  }

  timespec ts;
  memset(&ts, 0, sizeof(ts));
  ts.tv_sec = inteval / 1000;
  ts.tv_nsec = (inteval % 1000) * 1000000;

  itimerspec value;
  memset(&value, 0, sizeof(value));
  value.it_value = ts;

  int rt = timerfd_settime(m_fd, 0, &value, NULL);  //  value指定的新的超时时间，0表示相对时间，NULL位置不为NULL表示设置之前的超时时间
  if (rt != 0) {
    ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno));
  }
  // DEBUGLOG("timer reset to %lld", now + inteval);

}

void Timer::addTimerEvent(TimerEvent::s_ptr event) {
  bool is_reset_timerfd = false;

  ScopeMutex<Mutex> lock(m_mutex);
  if (m_pending_events.empty()) {
    is_reset_timerfd = true;
  } else {
    auto it = m_pending_events.begin();
    if ((*it).second->getArriveTime() > event->getArriveTime()) { //  如果新加入的该事件的到达事件比其他事件靠前，则需要resetArriveTime
      is_reset_timerfd = true;
    }
  }
  m_pending_events.emplace(event->getArriveTime(), event);
  lock.unlock();

  if (is_reset_timerfd) {
    resetArriveTime();
  }


}

//  删除一个定时任务
void Timer::deleteTimerEvent(TimerEvent::s_ptr event) {
  event->setCancled(true);

  ScopeMutex<Mutex> lock(m_mutex);

  auto begin = m_pending_events.lower_bound(event->getArriveTime());
  auto end = m_pending_events.upper_bound(event->getArriveTime());

  auto it = begin;
  for (it = begin; it != end; ++it) {
    if (it->second == event) {
      break;
    }
  }

  if (it != end) {
    m_pending_events.erase(it);
  }
  lock.unlock();

  DEBUGLOG("success delete TimerEvent at arrive time %lld", event->getArriveTime());

}




  
}