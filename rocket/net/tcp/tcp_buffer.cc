#include <memory>
#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_buffer.h"

namespace rocket {



TcpBuffer::TcpBuffer(int size) : m_size(size) {
  m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer() {

}

// 返回可读字节数
int TcpBuffer::readAble() {
  return m_write_index - m_read_index;
}

// 返回可写的字节数
int TcpBuffer::writeAble() {
  return m_buffer.size() - m_write_index;
}

int TcpBuffer::readIndex() {
  return m_read_index;
}

int TcpBuffer::writeIndex() {
  return m_write_index;
}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
  if (size > writeAble()) {
    // 调整 buffer 的大小，扩容
    int new_size = (int)(1.5 * (m_write_index + size));
    resizeBuffer(new_size);
  }
  memcpy(&m_buffer[m_write_index], buf, size);
  m_write_index += size; 
}


void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
  if (readAble() == 0) {
    return;
  }

  int read_size = readAble() > size ? size : readAble();    //  也是读两个值里面最少的值

  std::vector<char> tmp(read_size);
  memcpy(&tmp[0], &m_buffer[m_read_index], read_size);  // 读出数据

  re.swap(tmp); 
  m_read_index += read_size;

  adjustBuffer();   //  调整队列，整个队列已经读过的
}


void TcpBuffer::resizeBuffer(int new_size) {
  std::vector<char> tmp(new_size);
  int count = std::min(new_size, readAble());   //  用于将原本的数据拷贝过来，能拷贝的数据大小不能大于new_size，所以readAble与new_size相比较
  
  memcpy(&tmp[0], &m_buffer[m_read_index], count);  //  拷贝数据
  m_buffer.swap(tmp);

  m_read_index = 0;
  m_write_index = m_read_index + count; //  直接等于count？

}


void TcpBuffer::adjustBuffer() {
  if (m_read_index < int(m_buffer.size() / 3)) {
    return;
  }
  std::vector<char> buffer(m_buffer.size());
  int count = readAble();

  memcpy(&buffer[0], &m_buffer[m_read_index], count);   //  先将可读的内容保存在buffer里，从索引为0的位置开始
  m_buffer.swap(buffer);    //  将可读的内容放入原buffer，原buffer相对来说就是把原本可读的内容覆盖到了从索引0开始，也就是清空了已经读取过的数据
  m_read_index = 0;
  m_write_index = m_read_index + count;

  buffer.clear();
}

void TcpBuffer::moveReadIndex(int size) {
  size_t j = m_read_index + size;
  if (j >= m_buffer.size()) {
    ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
    return;
  }
  m_read_index = j;
  adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size) {
  size_t j = m_write_index + size;
  if (j >= m_buffer.size()) {
    ERRORLOG("moveWriteIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
    return;
  }
  m_write_index = j;
  adjustBuffer();

}

}