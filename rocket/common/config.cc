#include <tinyxml/tinyxml.h>
#include <rocket/common/config.h>

//  类似与内联函数，获取parent的第一个子对象
#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name);   \
if(!name##_node){   \
    printf("Start rocket server error, failed to read node [%s]\n", #name);    \
    exit(0);    \
}   \

//  类似内联函数，获取对应name的tag里的参数
#define READ_STR_FROM_XML_NODE(name, parent)    \
TiXmlElement* name##_node = log_node->FirstChildElement(#name); \
if(!name##_node || !name##_node->GetText()){    \
    printf("Start rocket server error, failed to read node %s\n", #name); \
    exit(0);    \
}   \
std::string name##_str = std::string(name##_node->GetText());   \

namespace rocket{

    static Config* g_config = NULL;

    //  返回一个全局的静态对象
    Config* Config::GetGlobalConfig(){
        return g_config;
    }

    //  设置Config对象
    void Config::SetGlobalConfig(const char* xmlfile){
        if(g_config==NULL){
            g_config = new Config(xmlfile);
        }
    }

    //  对xml文件进行分级拆分，获取所需内容
    Config::Config(const char* xmlfile){
        TiXmlDocument* xml_document = new TiXmlDocument();  //  声明一个TiXmlDocument对象

        bool rt = xml_document->LoadFile(xmlfile);
        if(!rt){
            printf("Start rocket server error, failed to read config file %s\n, error info[%s]", xmlfile, xml_document->ErrorDesc());
            exit(0);
        }

        READ_XML_NODE(root, xml_document);

        READ_XML_NODE(log, root_node);

        READ_STR_FROM_XML_NODE(log_level, log_node);

        m_log_level = log_level_str;



    }
}