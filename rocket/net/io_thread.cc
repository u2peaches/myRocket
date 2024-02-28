
#include <pthread.h>
#include <assert.h>
#include "rocket/net/io_thread.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"


namespace rocket {

IOThread::IOThread() {  

  int rt = sem_init(&m_init_semaphore, 0, 0);   //  信号量的初始化
  assert(rt == 0);

  rt = sem_init(&m_start_semaphore, 0, 0);
  assert(rt == 0);

  pthread_create(&m_thread, NULL, &IOThread::Main, this);   //  创建一个线程，this是作为参数传递给Main

  // wait, 直到新线程执行完 Main 函数的前置。如果不等的话，会导致thread的属性没有初始化完，出现错误
  sem_wait(&m_init_semaphore);  //  等到该值不为0

  DEBUGLOG("IOThread [%d] create success", m_thread_id);
}
  
IOThread::~IOThread() {

  m_event_loop->stop();
  sem_destroy(&m_init_semaphore);
  sem_destroy(&m_start_semaphore);

  pthread_join(m_thread, NULL);

  if (m_event_loop) {
    delete m_event_loop;
    m_event_loop = NULL;
  }
}


void* IOThread::Main(void* arg) {
  IOThread* thread = static_cast<IOThread*> (arg);  //  将参数转换出来

  thread->m_event_loop = new EventLoop();
  thread->m_thread_id = getThreadId();


  // 唤醒等待的线程
  sem_post(&thread->m_init_semaphore);

  // 让IO 线程等待，直到我们主动的启动

  DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);

  sem_wait(&thread->m_start_semaphore); //  为了让loop变得更加可控，可能会希望在loop循环进入前我们能够事先往其中加入FdEvent
  DEBUGLOG("IOThread %d start loop ", thread->m_thread_id);
  thread->m_event_loop->loop();

  DEBUGLOG("IOThread %d end loop ", thread->m_thread_id);

  return NULL;

}


EventLoop* IOThread::getEventLoop() {
  return m_event_loop;
}

//
void IOThread::start() {
  DEBUGLOG("Now invoke IOThread %d", m_thread_id);
  sem_post(&m_start_semaphore);
}


//  等待IO线程正常结束
void IOThread::join() {
  pthread_join(m_thread, NULL);
}



}