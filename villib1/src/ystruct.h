#ifndef ystruct_H
#define ystruct_H

#include <napi.h>
#include <iostream>
#include <functional>
#include <memory>
#include <libssh/libssh.h>
#include <mutex>

#include "./plugins/CJsonObject.h"
#include "./common.h"
#include "./yerrcode.h"


#include <vdlog.h>

// 多线程Promise异步请求的数据结构
class ThreadFuns
{
public:
  ThreadFuns();
  ~ThreadFuns();
  //--
  std::thread *th = NULL;                            // 多线程函数的线程对象
  std::string uuid;                                     // 保存在MAP结构的ID
  Napi::ThreadSafeFunction thread_function_1 = NULL;    // 线程安全函数
  Napi::ThreadSafeFunction thread_function_2 = NULL;    // 线程安全函数
  std::shared_ptr<Napi::Promise::Deferred> promise;     // 线程安全函数的promise对象
  int state = 0;                              //  状态码
  std::string response = "";                       //  响应数据
  std::string data;                                     // 附加数据
  void *ptr = NULL;                                    // 附加数据
};
inline ThreadFuns::ThreadFuns() {
}
inline ThreadFuns::~ThreadFuns()
{
  // fun.Release();
  if(thread_function_1 != NULL){
    thread_function_1.Release();
  }
  if(thread_function_2 != NULL){
    thread_function_2.Release();
  }
  if(ptr != NULL){
    delete ptr;
    ptr = NULL;
  }
  if (th != NULL)
  {
    th->join();
    delete th;
    vdlog::getInstance()->info("thread end.");
  }
}



class ysftp;
class ThreadFunsSftp
{
public:
  ThreadFunsSftp();
  ~ThreadFunsSftp();
  std::thread *th = nullptr; // 多线程函数的线程对象
  std::string uuid;          // 保存在MAP结构的ID
  std::vector<std::string> files;
  std::string source_path;
  std::string destination_path;
  Napi::ThreadSafeFunction fun1; // 线程安全函数
  Napi::ThreadSafeFunction fun2; // 线程安全函数
  //--
  std::string ptr_progress_name;
  int ptr_progress_size;
  int ptr_progress_count;
  //--
  int ptr_numbers_num;
  int ptr_numbers_count;
  //--
  int stoptransfer = 0;
  std::mutex transferlock;
  //--
  ysftp *sftpobj = nullptr;
  int response_code = YXU_EMPTY;
};
inline ThreadFunsSftp::ThreadFunsSftp() {}
inline ThreadFunsSftp::~ThreadFunsSftp()
{
  fun1.Release();
  fun2.Release();
  if (th != nullptr)
  {
    // std::cout<<"Release the thread."<<std::endl;
    th->join();
    delete th;
  }
}


struct fstat1
{
  int size = 0;          // 文件大小，单位字节
  std::string name = ""; // 文件名
  int gid = 0;           // 所属组ID
  int uid = 0;           // 所属用户ID
  int atime = 0;         // 访问时间
  int ctime = 0;         // 创建时间
  int mtime = 0;         // 修改时间
  char type[4] = {'\0'}; // 文件类型
  int read = 1;          // 读权限
  int write = 0;         // 写权限
  int exec = 0;          // 执行权限
};




#endif // ystruct_H