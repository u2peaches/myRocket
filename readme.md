#### 1 日志模块开发

日志模块：
```
1. 日志级别
2. 打印到文件，支持日期命名，以及日志的滚动
3. c 格式化风控（相比于C++的更方便，防止拼接的时候麻烦）
4. 线程安全
```

LogLevel：
```
Debug
Info
Error
```

LogEvent:
```
文件名、行号
MsgNo
进程号
Thread id
日期，以及时间，精确到ms
自定义消息
```

日志格式
```
[%y-%m-%d %H:%H:%S.%MS]\t[pid:thread_id]\t[file_name:line][%msg]
```

Logget 日志器
1. 提供打印日志的方法
2. 设置日志输出的路径