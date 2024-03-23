#include <vector>
#include <string.h>
#include <arpa/inet.h>
#include "rocket/net/coder/tinypb_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/common/util.h"
#include "rocket/common/log.h"

namespace rocket {

// 将 message 对象转化为字节流，写入到 buffer
void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {
  for (auto &i : messages) {
    std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(i);
    int len = 0;
    const char* buf = encodeTinyPB(msg, len);   //  返回字节流
    if (buf != NULL && len != 0) {
      out_buffer->writeToBuffer(buf, len);
    }
    if (buf) {
      free((void*)buf); //  释放掉内存，通过malloc申请的
      buf = NULL;
    }
  }
}

// 将 buffer 里面的字节流转换为 message 对象
void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {
  while(1) {
    // 遍历 buffer，找到 PB_START，找到之后，解析出整包的长度。然后得到结束符的位置，判断是否为 PB_END
    //  情况1：知道开始符，就知道结束符（可以通过整包长度估算），如果找到估算的位置不是结束符，则第一个开始符可能只是一个随机的字节流，继续寻找开始符
    //  情况2：估算的结束符所在的位置超过了当前的数据长度，说明可能TCP报文断开了，就跳过，等接收到下一个报文的时候再继续读
    std::vector<char> tmp = buffer->m_buffer;
    int start_index = buffer->readIndex();
    int end_index = -1;

    int pk_len = 0;
    bool parse_success = false;
    int i = 0;
    for (i = start_index; i < buffer->writeIndex(); ++i) {
      if (tmp[i] == TinyPBProtocol::PB_START) {
        // 读下去四个字节。由于是网络字节序，需要转为主机字节序  
        if (i + 1 < buffer->writeIndex()) {
          pk_len = getInt32FromNetByte(&tmp[i+1]);
          DEBUGLOG("get pk_len = %d", pk_len);

          // 结束符的索引
          int j = i + pk_len - 1;
          if (j >= buffer->writeIndex()) {  //  说明预估的结束符的位置超出了可读数据，说明发生了截断可能在后面的包中，继续往后遍历是否有开始符，如果遇到有的情况下，怎么办？直接跳出当前的状态，在下一次读取的时候，重新进行读写。
            continue;
          }
          if (tmp[j] == TinyPBProtocol::PB_END) {   //  如果预估结束符正确，表明找到了一个包，记录下在缓冲区中的区间
            start_index = i;
            end_index = j;
            parse_success = true;
            break;
          }
          
        }
      }
    }

    if (i >= buffer->writeIndex()) {    //  判断是否读完，没有找到TinyPB包，退出循环
      DEBUGLOG("decode end, read all buffer data");
      return;
    }

    if (parse_success) {    //  找到了包进行处理
      buffer->moveReadIndex(end_index - start_index + 1);   //  表明我们已经读完了这个包，事先把读的index移到包后面，防止后面我们读到重复数据
      std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>(); 
      message->m_pk_len = pk_len;

        //  记录msg_id的长度的开始索引
      int msg_id_len_index = start_index + sizeof(char) + sizeof(message->m_pk_len);
      if (msg_id_len_index >= end_index) {
        message->parse_success = false;
        ERRORLOG("parse error, msg_id_len_index[%d] >= end_index[%d]", msg_id_len_index, end_index);
        continue;
      }
      message->m_msg_id_len = getInt32FromNetByte(&tmp[msg_id_len_index]);
      DEBUGLOG("parse msg_id_len=%d", message->m_msg_id_len);

        //  记录msg_id的开始索引
      int msg_id_index = msg_id_len_index + sizeof(message->m_msg_id_len);
      
      char msg_id[100] = {0};
      memcpy(&msg_id[0], &tmp[msg_id_index], message->m_msg_id_len);
      message->m_msg_id = std::string(msg_id);
      DEBUGLOG("parse msg_id=%s", message->m_msg_id.c_str());

      int method_name_len_index = msg_id_index + message->m_msg_id_len;
      if (method_name_len_index >= end_index) {
        message->parse_success = false;
        ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]", method_name_len_index, end_index);
        continue;
      }
      message->m_method_name_len = getInt32FromNetByte(&tmp[method_name_len_index]);

      int method_name_index = method_name_len_index + sizeof(message->m_method_name_len);
      char method_name[512] = {0};
      memcpy(&method_name[0], &tmp[method_name_index], message->m_method_name_len);
      message->m_method_name = std::string(method_name);
      DEBUGLOG("parse method_name=%s", message->m_method_name.c_str());

      int err_code_index = method_name_index + message->m_method_name_len;
      if (err_code_index >= end_index) {
        message->parse_success = false;
        ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]", err_code_index, end_index);
        continue;
      }
      message->m_err_code = getInt32FromNetByte(&tmp[err_code_index]);


      int error_info_len_index = err_code_index + sizeof(message->m_err_code);
      if (error_info_len_index >= end_index) {
        message->parse_success = false;
        ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", error_info_len_index, end_index);
        continue;
      }
      message->m_err_info_len = getInt32FromNetByte(&tmp[error_info_len_index]);

      int err_info_index = error_info_len_index + sizeof(message->m_err_info_len);
      char error_info[512] = {0};
      memcpy(&error_info[0], &tmp[err_info_index], message->m_err_info_len);
      message->m_err_info = std::string(error_info);
      DEBUGLOG("parse error_info=%s", message->m_err_info.c_str());

      int pb_data_len = message->m_pk_len - message->m_method_name_len - message->m_msg_id_len - message->m_err_info_len - 2 - 24;

      int pd_data_index = err_info_index + message->m_err_info_len;
      message->m_pb_data = std::string(&tmp[pd_data_index], pb_data_len);

      // 这里校验和去解析，未写
      message->parse_success = true;

      out_messages.push_back(message);
    }

  }


}


const char* TinyPBCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len) {
  if (message->m_msg_id.empty()) {
    message->m_msg_id = "123456789";
  }
  DEBUGLOG("msg_id = %s", message->m_msg_id.c_str());
  
  int pk_len = 2 + 24 + message->m_msg_id.length() + message->m_method_name.length() + message->m_err_info.length() + message->m_pb_data.length();
  DEBUGLOG("pk_len = %", pk_len);

  char* buf = reinterpret_cast<char*>(malloc(pk_len));  //  申请一段内存空间
  char* tmp = buf;  //  tmp移动指针，用于给buf对应位置赋值对应的值

  *tmp = TinyPBProtocol::PB_START;
  tmp++;

  int32_t pk_len_net = htonl(pk_len);
  memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
  tmp += sizeof(pk_len_net);

  int msg_id_len = message->m_msg_id.length();
  int32_t msg_id_len_net = htonl(msg_id_len);
  memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
  tmp += sizeof(msg_id_len_net);

  if (!message->m_msg_id.empty()) {
    memcpy(tmp, &(message->m_msg_id[0]), msg_id_len);
    tmp += msg_id_len;
  }

  int method_name_len = message->m_method_name.length();
  int32_t method_name_len_net = htonl(method_name_len);
  memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
  tmp += sizeof(method_name_len_net);

  if (!message->m_method_name.empty()) {
    memcpy(tmp, &(message->m_method_name[0]), method_name_len);
    tmp += method_name_len;
  }

  int32_t err_code_net = htonl(message->m_err_code);
  memcpy(tmp, &err_code_net, sizeof(err_code_net));
  tmp += sizeof(err_code_net);

  int err_info_len = message->m_err_info.length();
  int32_t err_info_len_net = htonl(err_info_len);
  memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
  tmp += sizeof(err_info_len_net);

  if (!message->m_err_info.empty()) {
    memcpy(tmp, &(message->m_err_info[0]), err_info_len);
    tmp += err_info_len;
  }

  if (!message->m_pb_data.empty()) {
    memcpy(tmp, &(message->m_pb_data[0]), message->m_pb_data.length());
    tmp += message->m_pb_data.length();
  }

  int32_t check_sum_net = htonl(1);
  memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
  tmp += sizeof(check_sum_net);

  *tmp = TinyPBProtocol::PB_END;

    //  修改原内容
  message->m_pk_len = pk_len;
  message->m_msg_id_len = msg_id_len;
  message->m_method_name_len = method_name_len;
  message->m_err_info_len = err_info_len;
  message->parse_success = true;
  len = pk_len; //  返回长度，传递的是引用

  DEBUGLOG("encode message[%s] success", message->m_msg_id.c_str());

  return buf;
}


}