#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"



namespace rocket {

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {

  init(); 

  INFOLOG("rocket TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
}

TcpServer::~TcpServer() {
  if (m_main_event_loop) {
    delete m_main_event_loop;
    m_main_event_loop = NULL;
  }
  if (m_io_thread_group) {
    delete m_io_thread_group;
    m_io_thread_group = NULL; 
  }
  if (m_listen_fd_event) {
    delete m_listen_fd_event;
    m_listen_fd_event = NULL;
  }
}


void TcpServer::init() {

  m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr); //  make_shared适用于初始化shared_ptr

  m_main_event_loop = EventLoop::GetCurrentEventLoop(); //  获取当前主进程的EventLoop，EventLoop在整个进程中只存在一个
  m_io_thread_group = new IOThreadGroup(Config::GetGlobalConfig()->m_io_threads); //  获取对应的数量

  m_listen_fd_event = new FdEvent(m_acceptor->getListenFd()); //  获取FdEvent
  m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));  //  将Accept函数绑定在IN_EVENT标志上
  
  m_main_event_loop->addEpollEvent(m_listen_fd_event);  //  将m_listen_fd_event事件加入event_loop的监听列表，当事件发生时

  m_clear_client_timer_event = std::make_shared<TimerEvent>(5000, true, std::bind(&TcpServer::ClearClientTimerFunc, this)); //  生成定时事件加入定时事件组中
  m_main_event_loop->addTimerEvent(m_clear_client_timer_event); //  定时清理已关闭或者废除的连接、文件描述符

}

//  当有新客户端连接之后需要执行获取客户端连接，获取一个IO子线程，利用客户端连接和IO子线程构造对应的connection连接，保存状态
void TcpServer::onAccept() {
  auto re = m_acceptor->accept();   //  返回的是一个pair对象，第一个值为client_fd，第二个值为其地址
  int client_fd = re.first;
  NetAddr::s_ptr peer_addr = re.second;

  m_client_counts++;
  
  // 把 cleintfd 添加到任意 IO 线程里面
  IOThread* io_thread = m_io_thread_group->getIOThread();   //  从IO线程组中获取一个IO线程
  TcpConnection::s_ptr connetion = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr, m_local_addr); //  构造connection
  connetion->setState(Connected); //  设置其连接状态

  m_client.insert(connetion); //  保存起来

  INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

void TcpServer::start() {
  m_io_thread_group->start();   //  开启所有子线程的循环
  m_main_event_loop->loop();    //  开启主线程的循环
}


void TcpServer::ClearClientTimerFunc() {
  auto it = m_client.begin();
  for (it = m_client.begin(); it != m_client.end(); ) {
    // TcpConnection::ptr s_conn = i.second;
		// DebugLog << "state = " << s_conn->getState();
    if ((*it) != nullptr && (*it).use_count() > 0 && (*it)->getState() == Closed) {
      // need to delete TcpConnection
      DEBUGLOG("TcpConection [fd:%d] will delete, state=%d", (*it)->getFd(), (*it)->getState());
      it = m_client.erase(it);
    } else {
      it++;
    }
	
  }

}

}