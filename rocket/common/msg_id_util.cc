#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "rocket/common/msg_id_util.h"
#include "rocket/common/log.h"

namespace rocket {


static int g_msg_id_length = 20;
static int g_random_fd = -1;

static thread_local std::string t_msg_id_no;
static thread_local std::string t_max_msg_id_no;    //  设置最大msg_id的number，此处时number

std::string MsgIDUtil::GenMsgID() {
  if (t_msg_id_no.empty() || t_msg_id_no == t_max_msg_id_no) {  //  类似于Tcp的序列号，当现在的number为空（还没开始）或者已经等于最大的值时，进行重置
    if (g_random_fd == -1) {
      g_random_fd = open("/dev/urandom", O_RDONLY); //  打开随机文件获取随机文件句柄
    }
    std::string res(g_msg_id_length, 0);
    if ((read(g_random_fd, &res[0], g_msg_id_length)) != g_msg_id_length) { //  在随机文件中进行读取数据，如果读出来的长度不是预期值，则返回错误
      ERRORLOG("read form /dev/urandom error");
      return "";
    }
    for (int i = 0; i < g_msg_id_length; ++i) { //  每次获取一个字节
      uint8_t x = ((uint8_t)(res[i])) % 10;
      res[i] = x + '0';
      t_max_msg_id_no += "9";
    }
    t_msg_id_no = res;
  } else {
    size_t i = t_msg_id_no.length() - 1;
    while (t_msg_id_no[i] == '9' && i >= 0) {   //  相当于模拟一个整数，然后让改整数依次增大
      i--;
    }
    if (i >= 0) {
      t_msg_id_no[i] += 1;
      for (size_t j = i + 1; j < t_msg_id_no.length(); ++j) {
        t_msg_id_no[j] = '0';
      }
    }
  }

  return t_msg_id_no;   //  返回一个number

}

}
