#ifndef ROCKET_COMMON_RUN_TIME_H
#define ROCKET_COMMON_RUN_TIME_H


#include <string>

namespace rocket {

class RpcInterface;

//  用来记录RPC在运行过程中储存的一些关键信息
class RunTime {
 public:
  RpcInterface* getRpcInterface();

 public:
  static RunTime* GetRunTime();


 public:
  std::string m_msgid;
  std::string m_method_name;
  RpcInterface* m_rpc_interface {NULL};

};

}


#endif