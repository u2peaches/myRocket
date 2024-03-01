
#ifndef ROCKET_NET_FDEVENT_H
#define ROCKET_NET_FDEVENT_H

#include <functional>
#include <sys/epoll.h>

namespace rocket {


class FdEvent {
 public:
  enum TriggerEvent {
    IN_EVENT = EPOLLIN,
    OUT_EVENT = EPOLLOUT,
    ERROR_EVENT = EPOLLERR,
  };

  FdEvent(int fd);

  FdEvent();

  ~FdEvent();

  void setNonBlock(); //  设置m_fd类型是非阻塞的

  std::function<void()> handler(TriggerEvent event_type);

  void listen(TriggerEvent event_type, std::function<void()> callback, std::function<void()> error_callback = nullptr); //  对m_listen_event进行信号的绑定，同时赋值回调函数

  // 取消监听
  void cancle(TriggerEvent event_type); //  取消监听对应类型的事件

  int getFd() const {
    return m_fd;
  }

  epoll_event getEpollEvent() {
    return m_listen_events; //  处理好的事件会被用来存进内核事件表中，用于后续的epoll_wait，m_listen_events
  }


 protected:
  int m_fd {-1};

  epoll_event m_listen_events;

  std::function<void()> m_read_callback {nullptr};
  std::function<void()> m_write_callback {nullptr};
  std::function<void()> m_error_callback {nullptr};

};

}

#endif