
#include "rocket/common/run_time.h"

namespace rocket {

//  多线程的，每个线程有一个独有的该变量
thread_local RunTime* t_run_time = NULL; 

RunTime* RunTime::GetRunTime() {
  if (t_run_time) {
    return t_run_time;
  }
  t_run_time = new RunTime();
  return t_run_time;
}


RpcInterface* RunTime::getRpcInterface() {
  return m_rpc_interface;
}

}