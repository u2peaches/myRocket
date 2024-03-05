#ifndef ROCKET_NET_CODER_TINYPB_PROTOCOL_H
#define ROCKET_NET_CODER_TINYPB_PROTOCOL_H 

#include <string>
#include "rocket/net/coder/abstract_protocol.h"

namespace rocket {

struct TinyPBProtocol : public AbstractProtocol {
 public:
  TinyPBProtocol(){}
  ~TinyPBProtocol() {}

 public:
  static char PB_START; //  开始符
  static char PB_END;   //  结束符

 public:
  int32_t m_pk_len {0}; //  整包长度
  int32_t m_msg_id_len {0}; //  msg_id长度
  // msg_id 继承父类

  int32_t m_method_name_len {0};    //  方法名长度
  std::string m_method_name;    //  方法名
  int32_t m_err_code {0};   //  错误码
  int32_t m_err_info_len {0};   //  错误信息长度
  std::string m_err_info;   //  错误信息
  std::string m_pb_data;    //  Protobuf序列化数据，长度可以通过其他的计算出来
  int32_t m_check_sum {0};  //  校验码

  bool parse_success {false};   //  

};



}


#endif