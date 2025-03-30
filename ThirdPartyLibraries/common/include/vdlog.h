#ifndef VDLOG_H
#define VDLOG_H


#ifdef _WIN32

    #if defined(_MSC_VER)
        #pragma comment(lib, "ws2_32.lib")
    #endif // _MSC_VER

    #include <winsock2.h>
    using socket_t = SOCKET;

#else // not _WIN32

#include <arpa/inet.h>

#ifndef _AIX
    #include <ifaddrs.h>
#endif

#include <net/if.h>
#include <netdb.h>  
#include <netinet/in.h>

#ifdef __linux__
    #include <resolv.h>
#endif

#include <netinet/tcp.h>
#include <csignal>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using socket_t = int;

#ifndef INVALID_SOCKET
    #define INVALID_SOCKET (-1)
#endif

#endif //_WIN32




#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <climits>
#include <condition_variable>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <locale>
#include <codecvt>
#include <string.h>



#include "spdlog/async.h"
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"



class vdlog {
private:
    vdlog();
    static vdlog *instance;
public:
    static vdlog *getInstance();

    /**
     * @brief 设置接收日志主机地址和端口
     * @param newHost
     * @param newPost
     */
    void setHost(const std::string &newHost, const int newPost);

    /**
     * @brief 设置spdlog标签和日志文件路径
     * @param label
     * @param exepath
     */
    void setSpdlog(const char *label, const char *exepath);

    /**
     * @brief Set the Debug Mode object
     * 
     * @param m 
     */
    void setDebugMode(const int m);

    /**
     * @brief 输出UDP调试日志
     * @param format
     */
    void debug(const char * format, ... );

    /**
     * @brief 输出UDP错误日志
     * @param format
     */
    void error(const char * format, ... );

    /**
     * @brief 输出UDP警告日志
     * @param format
     */
    void warn(const char * format, ... );

    /**
     * @brief 输出UDP日志
     * @param format
     */
    void info(const char * format, ... );

private:
    spdlog::logger *logobj=nullptr;
    std::string host;
    int port=0;
    void sendmsg(std::string type, std::string msg);
    int DebugMode = 0;
};



///=============================================================
///

inline vdlog *vdlog::instance=nullptr;

inline vdlog::vdlog()
{
    //
}

inline vdlog *vdlog::getInstance()
{
    if(instance==nullptr){
        instance=new vdlog();
    }
    return instance;
}

inline void vdlog::setHost(const std::string &newHost, const int newPost)
{
    host = newHost;
    port = newPost;
}

inline void vdlog::setSpdlog(const char *label, const char *exepath)
{
    //--输出控制台
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    //--输出文件
    char logpath[1024]={'\0'};
    sprintf(logpath, "%s/logs/%s.log", exepath, label);
    //
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logpath, 1024*1024*10, 10);
    file_sink->set_level(spdlog::level::trace);
    //--合并写入
    logobj=new spdlog::logger(label, {console_sink, file_sink});
    logobj->set_level(spdlog::level::debug);
    //--实时写入
    logobj->flush_on(spdlog::level::trace);
}

inline void vdlog::setDebugMode(const int m){
    this->DebugMode=m;
}

inline void vdlog::debug(const char *format, ...)
{
    //如果不是调试模式，不输出
    if(!this->DebugMode) return ;
    va_list args, args1;
    va_start(args, format);
    va_copy(args1, args);
#ifdef _WIN32
    //int len = _vscprintf(format, args) + 1;
    std::string txt(1 + _vscprintf(format, args1), 0);
#else
    //int len = vsnprintf(NULL, 0, format, args) +1;
    std::string txt(1 + vsnprintf(nullptr, 0, format, args1), 0);
#endif
    va_end (args1);
    vsnprintf(&txt[0], txt.size(), format, args);
    va_end (args);
    this->sendmsg("debug", txt);
    if(logobj!=nullptr){
        logobj->debug(txt.c_str());
    }
}


inline void vdlog::error(const char *format, ...)
{
    va_list args, args1;
    va_start(args, format);
    va_copy(args1, args);
#ifdef _WIN32
    //int len = _vscprintf(format, args) + 1;
    std::string txt(1 + _vscprintf(format, args1), 0);
#else
    //int len = vsnprintf(NULL, 0, format, args) +1;
    std::string txt(1 + vsnprintf(nullptr, 0, format, args1), 0);
#endif
    va_end (args1);
    vsnprintf(&txt[0], txt.size(), format, args);
    va_end (args);
    this->sendmsg("error", txt);
    if(logobj!=nullptr){
        logobj->error(txt.c_str());
    }
}


inline void vdlog::warn(const char *format, ...)
{
    va_list args, args1;
    va_start(args, format);
    va_copy(args1, args);
#ifdef _WIN32
    //int len = _vscprintf(format, args) + 1;
    std::string txt(1 + _vscprintf(format, args1), 0);
#else
    //int len = vsnprintf(NULL, 0, format, args) +1;
    std::string txt(1 + vsnprintf(nullptr, 0, format, args1), 0);
#endif
    va_end (args1);
    vsnprintf(&txt[0], txt.size(), format, args);
    va_end (args);
    this->sendmsg("warn", txt);
    if(logobj!=nullptr){
        logobj->warn(txt.c_str());
    }
}


inline void vdlog::info(const char *format, ...)
{
    //如果不是调试模式，不输出
    if(!this->DebugMode) return ;
    va_list args, args1;
    va_start(args, format);
    va_copy(args1, args);
#ifdef _WIN32
    //int len = _vscprintf(format, args) + 1;
    std::string txt(1 + _vscprintf(format, args1), 0);
#else
    //int len = vsnprintf(NULL, 0, format, args) +1;
    std::string txt(1 + vsnprintf(nullptr, 0, format, args1), 0);
#endif
    va_end (args1);
    vsnprintf(&txt[0], txt.size(), format, args);
    va_end (args);
    this->sendmsg("info", txt);
    if(logobj!=nullptr){
        logobj->info(txt.c_str());
    }
}


inline void vdlog::sendmsg(std::string type, std::string msg)
{

    if(this->port==0){
        return ;
    }

    int typeint=0;
    if(type=="warn"||type=="error"){
        if(type=="warn"){
            typeint=4;
        }
        if(type=="error"){
            typeint=5;
        }
    }else{
        if(this->DebugMode){
            if(type=="trace")
                typeint=1;
            if(type=="debug")
                typeint=2;
            if(type=="info"){
                typeint=3;
            }
        }else{
            return ;
        }
    }

    //--
    int msglength=msg.size();
    int int_size=sizeof(int);
    int bufsize=int_size+int_size+msglength;
    char *buffer=new char[bufsize];

    memset(buffer, 0, bufsize);
    memcpy(buffer, &typeint, int_size);
    memcpy(buffer+int_size, &msglength, int_size);
    memcpy(buffer+int_size+int_size, msg.data(), msglength);

    //--
    try {
        socket_t sockfd;
        struct sockaddr_in servaddr;
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            throw -1;
        }
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(this->port);
        servaddr.sin_addr.s_addr = inet_addr(this->host.c_str());
        sendto(sockfd, buffer, bufsize, 0, (sockaddr*)&servaddr, sizeof(servaddr));
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
    } catch (int code) {
        return ;
    }
}



#endif // VDLOG_H
