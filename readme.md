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

#### 2.4.1 TimerEvent 定时任务
```
参数
1. 指定时间点 arrive_time
2. interval, ms
3. is_repeated
4. is_cancled
5. task

函数
1. cancle()
2. cancleRepeated()
```

#### 2.4.2 Timer
定时器，他是一个TimerEvent的集合。
Timer 继承 FdEvent
```
addTimerEvent();
deleteTimerEvent();
onTimer();  //  当发生了IO时间之后，需要执行的方法
reserArriveTime()

multimap 存储 TimerEvent <key(arrivetime), TimerEvent>
```

#### 2.5 IO 线程
创建一个IO线程，他会执行：
1. 创建一个新线程（pthread_create）
2. 在新线程里面 创建一个Eventloop，完成初始化
3. 进行loop
```
class {
    pthread_t m_thread;
    pid_t m_threaqd_id;
    EventLoop event_loop;
}

```