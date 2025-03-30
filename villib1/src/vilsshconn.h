#ifndef vilsshconn_H
#define vilsshconn_H

#include <napi.h>
#include <iostream>
#include <functional>
#include <memory>
#include <libssh/libssh.h>
#include <stdlib.h>

#include "./plugins/CJsonObject.h"
#include "./common.h"

#include <vdlog.h>

class vilsshconn
{
public:
  vilsshconn(vilsshconn *c);
  vilsshconn(std::string *host, int *port, std::string *user, std::string *pwd, std::string *privatekey, int *usepubkey);
  ~vilsshconn();
  int release();
  int reconn();

  ///
  int create(); 
  int connnect();
  int authhost();
  int verify_known_host();
  int verify_known_host_update();
  int auth_with_user();
  int auth_with_privatekey();
  int getconn();
  int get_connect_state();

  ///
  ssh_session session = nullptr;

  ///
  std::string *host = nullptr;
  int *port = nullptr;
  ///
  std::string *user = nullptr;
  std::string *pwd = nullptr;
  std::string *uid = nullptr;
  int *usepubkey = nullptr;
  std::string *privatekey = nullptr; 
  int connected = YXU_CONNECT_UNKNOWN;
  int connect_timeout = 5;
private:
  ssh_key pkey = NULL;
  int ServerAliveInterval = 30;
  int ServerAliveCountMax = 60;
};



#endif // vilsshconn_H