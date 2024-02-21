#include <iostream>
#include <pthread.h>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
void* fun(void*){
    DEBUGLOG("debug is thread in %s", "fun");
    INFOLOG("info is thread in %s", "fun");
    return NULL;
}

int main(){

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    pthread_t thread;
    pthread_create(&thread, NULL, &fun, NULL);

    DEBUGLOG("test debug log %s", "11");
    INFOLOG("test info log %s", "11");
    // std::cout<<0<<std::endl;

    pthread_join(thread, NULL);
    return 0;
}