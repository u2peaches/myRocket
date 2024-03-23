#ifndef ROCKET_NET_TCP_NET_ADDR_H
#define ROCKET_NET_TCP_NET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace rocket {

class NetAddr {
 public:
  typedef std::shared_ptr<NetAddr> s_ptr;

  virtual sockaddr* getSockAddr() = 0;  //  获取socket地址，存储了相关内容

  virtual socklen_t getSockLen() = 0;

  virtual int getFamily() = 0;  //  获得family，也就是协议类型

  virtual std::string toString() = 0;   //  将地址打印成字符串变量

  virtual bool checkValid() = 0;

};


//  此处继承一个抽象类的原因是因为多态的实现，当前该类是一个TCP的继承类，后续也可以构造UNIX通信的继承类
class IPNetAddr : public NetAddr {

 public:
  static bool CheckValid(const std::string& addr);
 public:
  
  IPNetAddr(const std::string& ip, uint16_t port);  //  通过IP和端口进行构造
  
  IPNetAddr(const std::string& addr);   //  直接提供点分十进制进行构造

  IPNetAddr(sockaddr_in addr);  //  用sockaddr_in进行构造

  sockaddr* getSockAddr();

  socklen_t getSockLen();

  int getFamily();

  std::string toString();

  bool checkValid();    //  检验IP与port等是否合法
 
 private:
  std::string m_ip;
  uint16_t m_port {0};

  sockaddr_in m_addr;

};

}

#endif
