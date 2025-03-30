#ifndef ychannel_H
#define ychannel_H

#include <napi.h>
#include <iostream>
#include <functional>
#include <memory>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <stdlib.h>
#include <uv.h>


#include "./yerrcode.h"
#include "./ystruct.h"
#include "./plugins/CJsonObject.h"
#include "./common.h"
#include "./vilsshconn.h"

#include <vdlog.h>

class ychannel
{
public:
  ychannel();
  ~ychannel();

  //--
  int generate(vilsshconn *c);
  int generate(vilsshconn *c, int pty_cols, int pty_rows);
  int readdata(std::function<void(char *buffer, int size)> ptr = nullptr);
  int writedata(std::string txt);
  int channelclose();
  int resize(int pty_cols, int pty_rows);

public:
  vilsshconn *conn = NULL;
  ssh_channel channel = NULL;
public:
  int state;
  int pty_cols;
  int pty_rows;
  Napi::ThreadSafeFunction callback1;
  Napi::ThreadSafeFunction callback2;
  std::string uvbuffer;
};

inline ychannel::ychannel()
{
}

inline ychannel::~ychannel()
{
}


#endif // ychannel_H