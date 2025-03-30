#ifndef ysftp_H
#define ysftp_H

#include <napi.h>
#include <iostream>
#include <functional>
#include <memory>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <stdlib.h>
#include <stack>
#include <ctime>
#include <chrono>



#include "./yerrcode.h"
#include "./ystruct.h"
#include "./plugins/CJsonObject.h"
#include "./common.h"
#include "./ychannel.h"
#include "./vilsshconn.h"

#include <vdlog.h>

#define MAX_XFER_BUF_SIZE 16438

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002

#define O_CREAT 0x0100
#define O_TRUNC 0x0200
#define O_EXCL 0x0400

#define S_IFREG 0x8000  /* regular */
#define S_IREAD 0x0100  /* read permission, owner */
#define S_IWRITE 0x0080 /* write permission, owner */
#define S_IEXEC 0x0040  /* execute/search permission, owner */
#define S_IRWXU S_IREAD | S_IWRITE | S_IEXEC



struct ysftp_file_attr1 {
  std::string linkfilepath;
  std::string filetype;
  int islink;
  int isRead;
  int isWrite;
  std::string filepath;
};

using map1 = std::map<std::string, std::string>;

class ysftp
{
public:
  ysftp(vilsshconn *c);
  ~ysftp();

  //--
  int generate();
  int release();
  int listdir(std::string path, JsonObj &arr);

  // 上传文件到服务器
  void uploadfiles(std::vector<std::string> files,
                   std::string source_path,
                   std::string destination_path,
                   std::function<void(std::string filename, int size, int count)> ptr_progress,
                   std::function<void(int num, int nums, int code)> ptr_numbers);
  int uploadfiles_recursion_folder_local(std::string _dir, int mode);

  // 下载文件到本地
  void downloadfiles(std::vector<std::string> files,
                     std::string source_path,
                     std::string destination_path,
                     std::function<void(std::string filename, int size, int count)> ptr_progress,
                     std::function<void(int num, int nums, int code)> ptr_numbers);
  int downloadfiles_recursion_folder_server(std::string _dir, int mode);
  int downloadfiles_recursion_folder_server2(std::string _dir, int mode);

  // 判断文件是否存在
  int exists_dir(std::string _path);

  // 文件夹及文件操作
  int _mkdir(std::string _path);
  int _rmdir(std::string _path);
  int _rmfile(std::string _path);
  int _rename(std::string _ori, std::string _new);

  // 读写服务器文件
  int _readfilecheck(std::string _path);
  int _readPermission(std::string _path);
  int _readfile(std::string _path, std::string &_content);
  int _writefile(std::string _path, std::string &_content);
  
  //
  int geterror();
  int getpermission(std::string uid, std::string file, sftp_attributes &_attr);
  int get_filetype_and_permission(std::string file, std::string uid, sftp_attributes &_attr, ysftp_file_attr1 &f_attr);

public:
  sftp_session sftp = nullptr;
  vilsshconn *conn = nullptr;

  int *stoptransfer;
  std::mutex *transferlock = nullptr;

  sftp_dir dir;
  std::vector<map1> serverfiles; // 待下载的服务器文件列表
  std::vector<map1> localfiles;  // 待上传的本地文件列表
};

inline ysftp::ysftp(vilsshconn *c)
{
  this->conn = new vilsshconn(c);
}

inline ysftp::~ysftp()
{
  release();
}


#endif //