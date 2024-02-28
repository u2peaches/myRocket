#ifndef ROCKET_NET_TCP_TCP_BUFFER_H
#define ROCKET_NET_TCP_TCP_BUFFER_H

#include <vector>
#include <memory>

namespace rocket {

class TcpBuffer {

 public:

  typedef std::shared_ptr<TcpBuffer> s_ptr;

  TcpBuffer(int size);

  ~TcpBuffer();

  // 返回可读字节数
  int readAble();

  // 返回可写的字节数
  int writeAble();

  int readIndex();  //  读取index

  int writeIndex(); //  写入index

  void writeToBuffer(const char* buf, int size);    //  写入数据

  void readFromBuffer(std::vector<char>& re, int size); //  读取数据

  void resizeBuffer(int new_size);

  void adjustBuffer();

  void moveReadIndex(int size);

  void moveWriteIndex(int size);

 private:
  int m_read_index {0}; //  记录读取的位置
  int m_write_index {0};    //  记录写入的位置
  int m_size {0};   //  buffer容量

 public:
  std::vector<char> m_buffer;

};


}


#endif