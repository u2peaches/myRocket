#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include<string>
#include<queue>
#include<memory>
#include "rocket/common/config.h"
#include "rocket/common/mutex.h"

namespace rocket{

template<typename... Args>
std::string formatString(const char* str, Args&&... args){
    int size = snprintf(nullptr, 0, str, args...);  //  判断arg...可变参数根据str格式形成的新格式的字符串长度
    std::string result;
    if(size > 0){
        result.resize(size);
        snprintf(&result[0], size+1, str, args...); //  如果长度大于0，则将数据result[0]的位置开始写入；
    }
    return result;
}


#define DEBUGLOG(str, ...)\
    if(rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug){  \
        rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Debug).toString()  \
         + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
        rocket::Logger::GetGlobalLogger()->log();   \
    }

#define INFOLOG(str, ...)\
    if(rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info){  \
        rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Info).toString()   \
         + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
        rocket::Logger::GetGlobalLogger()->log();   \
    }

#define ERRORLOG(str, ...)\
    if(rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error){  \
        rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Error).toString()->toString()  \
         + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
        rocket::Logger::GetGlobalLogger()->log();   \
    }

enum LogLevel{
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};

class Logger{
    public:

        typedef std::shared_ptr<Logger> s_ptr;
        Logger(LogLevel level): m_set_level(level){}
        LogLevel getLogLevel() const;
        void pushLog(const std::string& msg);
        void log();

    public:
        static Logger* GetGlobalLogger();
        static void InitGlobalLogger();

    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;
        Mutex m_mutex;
};


std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string& log_level);


class LogEvent{
    public:

        //  初始化日志级别
        LogEvent(LogLevel level) : m_level(level){}

        //  获取文件名
        std::string getFileName() const{
            return m_file_name;
        }

        //  获取日志级别
        LogLevel getLogLevel()  const{
            return m_level;
        }


        std::string toString();

    private:
        std::string m_file_name;    //  文件名
        int32_t m_file_line;    //  行号
        int32_t m_pid;  //  进程号
        int32_t m_thread_id;    //  线程号

        LogLevel m_level;   //  日志级别
        Logger::s_ptr m_logger;

};


}

#endif