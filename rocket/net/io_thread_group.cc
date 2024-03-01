#include "rocket/net/io_thread_group.h"
#include "rocket/common/log.h"


namespace rocket {


//  初始化IO线程组
IOThreadGroup::IOThreadGroup(int size) : m_size(size) {
  m_io_thread_groups.resize(size);
  for (size_t i = 0; (int)i < size; ++i) {
    m_io_thread_groups[i] = new IOThread();
  }
}

IOThreadGroup::~IOThreadGroup() {

}

//  将每个线程组里的IO线程开启它们的Loop循环
void IOThreadGroup::start() {
  for (size_t i = 0; i < m_io_thread_groups.size(); ++i) {
    m_io_thread_groups[i]->start();
  }
}

//  阻塞各个线程
void IOThreadGroup::join() {
  for (size_t i = 0; i < m_io_thread_groups.size(); ++i) {
    m_io_thread_groups[i]->join();
  }
} 

//  通过轮询来进行获取，桉顺序获取，依次给不同的线程任务
IOThread* IOThreadGroup::getIOThread() {
  if (m_index == (int)m_io_thread_groups.size() || m_index == -1)  {
    m_index = 0;
  }
  return m_io_thread_groups[m_index++];
}

}