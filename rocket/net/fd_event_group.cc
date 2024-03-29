#include "rocket/net/fd_event_group.h"
#include "rocket/common/mutex.h"
#include "rocket/common/log.h"

namespace rocket {


static FdEventGroup* g_fd_event_group = NULL;

//  设置一个唯一的全局的EventGroup，通过该函数可以获得，如果没创建则进行创建
FdEventGroup* FdEventGroup::GetFdEventGroup() {
  if (g_fd_event_group != NULL) {
    return g_fd_event_group;
  }

  g_fd_event_group = new FdEventGroup(128);
  return g_fd_event_group;
}

FdEventGroup::FdEventGroup(int size) :m_size(size) {
  for (int i = 0; i < m_size; i++) {
    m_fd_group.push_back(new FdEvent(i));
  }
}


FdEventGroup::~FdEventGroup() {
  for (int i = 0; i < m_size; ++i) {
    if (m_fd_group[i] != NULL) {
      delete m_fd_group[i];
      m_fd_group[i] = NULL;
    }
  }
}

//  获取一个事件，通过一个事件组来进行管理，如果事件数超过了已有的设置的上限，则进行扩容
FdEvent* FdEventGroup::getFdEvent(int fd) {
  ScopeMutex<Mutex> lock(m_mutex);
  if ((size_t) fd < m_fd_group.size()) {
    return m_fd_group[fd];
  }

  int new_size = int(fd * 1.5);
  //    扩容
  for (int i = m_fd_group.size(); i < new_size; ++i) {
    m_fd_group.push_back(new FdEvent(i));
  }
  return m_fd_group[fd];

}

}