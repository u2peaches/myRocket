#ifndef ROCKET_NET_IO_THREAD_GROUP_H
#define ROCKET_NET_IO_THREAD_GROUP_H

#include <vector>
#include "rocket/common/log.h"
#include "rocket/net/io_thread.h"



namespace rocket {

//  对IO线程的封装
class IOThreadGroup {

 public:
  IOThreadGroup(int size);

  ~IOThreadGroup();

  void start(); //  控制所有IO线程的loop循环的开始

  void join();

  IOThread* getIOThread();  //  从当前IO组里获取一个可用的IO线程

 private:

  int m_size {0};
  std::vector<IOThread*> m_io_thread_groups;    //  存储IOThread的数组

  int m_index {0};

};

}


#endif