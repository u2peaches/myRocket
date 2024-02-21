#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <map>
#include <string>
namespace rocket{

class  Config
{
private:
    /* data */
    

public:
    std::string m_log_level;
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char* xmlfile);
public:
    Config();
    Config(const char* xmlfile/* args */);
};



}

#endif