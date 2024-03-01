#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include <map>
#include <queue>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/rpc/rpc_dispatcher.h"

namespace rocket {

enum TcpState {
  NotConnected = 1,
  Connected = 2,
  HalfClosing = 3,
  Closed = 4,
};


enum TcpConnectionType {
  TcpConnectionByServer = 1,  // 作为服务端使用，代表跟对端客户端的连接
  TcpConnectionByClient = 2,  // 作为客户端使用，代表跟对赌服务端的连接
};

class TcpConnection {
 public:

  typedef std::shared_ptr<TcpConnection> s_ptr;


 public:
  TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, TcpConnectionType type = TcpConnectionByServer);

  ~TcpConnection();

  void onRead();

  void excute();

  void onWrite();

  void setState(const TcpState state);  //  修改状态

  TcpState getState();  //  获取状态

  void clear();

  int getFd();

  // 服务器主动关闭连接
  void shutdown();

  //  设置当前主体类型，服务器还是客户端
  void setConnectionType(TcpConnectionType type);

  // 启动监听可写事件
  void listenWrite();

  // 启动监听可读事件
  void listenRead();

  void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

  void pushReadMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);

  NetAddr::s_ptr getLocalAddr();

  NetAddr::s_ptr getPeerAddr();

  void reply(std::vector<AbstractProtocol::s_ptr>& replay_messages);

 private:

  EventLoop* m_event_loop {NULL};   // 代表持有该连接的 IO 线程

  NetAddr::s_ptr m_local_addr;  //  本地的地址
  NetAddr::s_ptr m_peer_addr;   //  目标连接端的地址

  TcpBuffer::s_ptr m_in_buffer;   // 接收缓冲区
  TcpBuffer::s_ptr m_out_buffer;  // 发送缓冲区

  FdEvent* m_fd_event {NULL};   //

  AbstractCoder* m_coder {NULL};

  TcpState m_state;

  int m_fd {0}; //  表示是哪个connection，是哪个客户端

  TcpConnectionType m_connection_type {TcpConnectionByServer};

  // std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>
  //  保证先进先出的顺序性，当发送两条消息时，先发第一条再发第二条
  std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;

  // key 为 msg_id，不用保证顺序性
  std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;
  
};

}

#endif
