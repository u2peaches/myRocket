#ifndef ROCKER_NET_RPC_RPC_CONTROLLER_H
#define ROCKER_NET_RPC_RPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h> //  引入以下的google::protobuf::Closure
#include <string>

#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"

namespace rocket {

class RpcController : public google::protobuf::RpcController {

 public:
  RpcController() { INFOLOG("RpcController"); } 
  ~RpcController() { INFOLOG("~RpcController"); } 


  //    清楚之前的状态，将参数重新赋值为初始值
  void Reset();

  bool Failed() const;  //  判断是否调用成功

  std::string ErrorText() const;    //  返回错误信息

  void StartCancel();   //  取消

  void SetFailed(const std::string& reason);    //  设置失败信息

  bool IsCanceled() const;  //  判断是否取消

  void NotifyOnCancel(google::protobuf::Closure* callback); //  
    //  以上几个函数是继承父类的一个纯虚函数，需要进行实现


  void SetError(int32_t error_code, const std::string error_info);

  int32_t GetErrorCode();

  std::string GetErrorInfo();

  void SetMsgId(const std::string& msg_id);

  std::string GetMsgId();

  void SetLocalAddr(NetAddr::s_ptr addr);

  void SetPeerAddr(NetAddr::s_ptr addr);

  NetAddr::s_ptr GetLocalAddr();

  NetAddr::s_ptr GetPeerAddr();

  void SetTimeout(int timeout);

  int GetTimeout();

  bool Finished();

  void SetFinished(bool value);
 
 private:
  int32_t m_error_code {0};
  std::string m_error_info;
  std::string m_msg_id;

  bool m_is_failed {false};
  bool m_is_cancled {false};
  bool m_is_finished {false};

  NetAddr::s_ptr m_local_addr;
  NetAddr::s_ptr m_peer_addr;

  int m_timeout {1000};   // ms，超时时间

};

}


#endif