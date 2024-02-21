#include <sys/time.h>
#include <sstream>
#include <stdio.h> 
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"


namespace rocket{


static Logger* g_logger = NULL;

// 用于管理全局静态变量，只有第一次调用的时候才进行该变量的动态构建，以后调用该函数的时候都直接获取到第一次调用创建的对象
Logger* Logger::GetGlobalLogger(){  
    return g_logger;

}

void Logger::InitGlobalLogger(){
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    printf("init log level [%s]\n", LogLevelToString(global_log_level).c_str());
    g_logger = new Logger(global_log_level);
}

//  日志等级数字转文本
std::string LogLevelToString(LogLevel Level){
    switch (Level)
    {
    case Debug:
        return "DEBUG";
    case Info:
        return "INFO";
    case Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

//  日志等级数字转文本
LogLevel StringToLogLevel(const std::string& log_level){
    if(log_level == "DEBUG"){
        return Debug;
    }
    else if(log_level == "INFO"){
        return Info;
    }
    else if(log_level == "ERROR"){
        return Error;
    }
    else{
        return Unknown;
    }
}


std::string LogEvent::toString(){
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);   //  获取当前的时间，timeval结构体里存储着以秒为单位或是毫秒为单位的

    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);   //  获取本地时间格式存储在now_time_t中

    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);   //  按照第三个参数的字符串格式，将now_time_t中的内容尽心改变，存储在buf中
    std::string time_str(buf);

    int ms = now_time.tv_usec * 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();


    std::stringstream ss;

    ss<<"[" << LogLevelToString(m_level) <<"]\t"
    << "[" << time_str <<"]\t"
    << "[" << m_pid << ":" << m_thread_id << "]\t";

    return ss.str();
}

LogLevel Logger::getLogLevel() const{
    return m_set_level;
}

void Logger::pushLog(const std::string& msg){
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(msg);
    lock.unlock();
}

void Logger::log(){
    ScopeMutex<Mutex> lock(m_mutex);
    std::queue<std::string> tmp = m_buffer;
    m_buffer.swap(tmp);
    while(!tmp.empty()){
        std::string msg = tmp.front();
        tmp.pop();
        printf("%s", msg.c_str());
    }
}
}