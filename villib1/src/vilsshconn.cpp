#include "./vilsshconn.h"
#include "./yerrcode.h"

vilsshconn::vilsshconn(vilsshconn *c)
{
  this->host = c->host;
  this->port = c->port;
  this->user = c->user;
  this->pwd = c->pwd;
  this->usepubkey = c->usepubkey;
  this->privatekey = c->privatekey;
  this->uid = c->uid;
}

vilsshconn::vilsshconn(std::string *host, int *port, std::string *user, std::string *pwd, std::string *privatekey, int *usepubkey)
{
  this->host = host;
  this->port = port;
  this->user = user;
  this->pwd = pwd;
  this->usepubkey = usepubkey;
  this->privatekey = privatekey;
  this->uid = new std::string("0");
}

vilsshconn::~vilsshconn()
{
  release();
}

int vilsshconn::release()
{
  this->connected = YXU_CONNECT_UNKNOWN;
  if (session != nullptr)
  {
    ssh_disconnect(session);
    ssh_free(session);
    vdlog::getInstance()->warn("end of ssh session.");
  }
  if (pkey != NULL)
  {
    ssh_key_free(pkey);
  }
  return 1;
}

int vilsshconn::reconn()
{
  ssh_disconnect(session);
  // ssh_free(session);
  int ret;
  ret = this->create();
  if (ret != YXU_SUCCESS)
  {
    return ret;
  }
  ret = this->connnect();
  if (ret != YXU_SUCCESS)
  {
    return ret;
  }
  ret = this->authhost();
  return ret;
}

int vilsshconn::create()
{
  session = ssh_new();
  if (session == NULL)
  {
    return YXU_CREATE_SESSION_FAILED;
  }
  ssh_options_set(session, SSH_OPTIONS_HOST, this->host->c_str());
  ssh_options_set(session, SSH_OPTIONS_PORT, this->port);
  ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &connect_timeout);
  this->connected = YXU_CONNECT_CREATED_SESSION;
  return YXU_SUCCESS;
}

int vilsshconn::connnect()
{
  int rc = ssh_connect(session);
  if (rc != SSH_OK)
  {
    vdlog::getInstance()->error("connect failed: %s", ssh_get_error(session));
    return YXU_CONNECT_FAILED;
  }
  this->connected = YXU_CONNECT_CONNECTED;
  return YXU_SUCCESS;
}

int vilsshconn::authhost()
{
  int ret;
  if (*this->usepubkey == 1)
  {
    ret = this->auth_with_privatekey();
  }
  else
  {
    ret = this->auth_with_user();
  }
  return ret;
}

int vilsshconn::verify_known_host()
{
  enum ssh_known_hosts_e state;
  unsigned char *hash = NULL;
  ssh_key srv_pubkey = NULL;
  size_t hlen;
  char buf[10];
  char *hexa;
  char *p;
  int cmp;
  int rc;

  rc = ssh_get_server_publickey(session, &srv_pubkey);
  if (rc < 0)
  {
    return YXU_GET_SERVER_PUBLIC_KEY_FAILED;
  }

  rc = ssh_get_publickey_hash(srv_pubkey,
                              SSH_PUBLICKEY_HASH_SHA1,
                              &hash,
                              &hlen);
  ssh_key_free(srv_pubkey);
  if (rc < 0)
  {
    return YXU_GET_SERVER_PUBLIC_KEY_FAILED;
  }

  state = ssh_session_is_known_server(session);
  switch (state)
  {
  case SSH_KNOWN_HOSTS_OK:
    /* OK */

    break;
  case SSH_KNOWN_HOSTS_CHANGED:
    vdlog::getInstance()->error("Host key for server changed, For security reasons, connection will be stopped.");
    // fprintf(stderr, "Host key for server changed: it is now:\n");
    // ssh_print_hexa("Public key hash", hash, hlen);
    // fprintf(stderr, "For security reasons, connection will be stopped\n");
    ssh_clean_pubkey_hash(&hash);

    return YXU_KNOWN_HOSTS_CHANGED;
  case SSH_KNOWN_HOSTS_OTHER:
    vdlog::getInstance()->error("The host key for this server was not found but an other type of key exists.");
    ssh_clean_pubkey_hash(&hash);

    return YXU_KNOWN_HOSTS_OTHER;
  case SSH_KNOWN_HOSTS_NOT_FOUND:
    vdlog::getInstance()->error("Could not find known host file.");
  case SSH_KNOWN_HOSTS_UNKNOWN:
    vdlog::getInstance()->info("The server is unknown. Do you trust the host key?");
    return YXU_KNOWN_HOSTS_UNKNOWN;
  case SSH_KNOWN_HOSTS_ERROR:
    vdlog::getInstance()->error("Error %s", ssh_get_error(session));
    ssh_clean_pubkey_hash(&hash);
    return YXU_KNOWN_HOSTS_ERROR;
  }

  ssh_clean_pubkey_hash(&hash);
  return YXU_SUCCESS;
}

int vilsshconn::verify_known_host_update()
{
  int rc = ssh_session_update_known_hosts(session);
  if (rc < 0)
  {
    vdlog::getInstance()->error("Error %s", strerror(errno));
    return YXU_UPDATE_KNOWN_HOST_FAILED;
  }
  return verify_known_host();
}

int vilsshconn::auth_with_user()
{
  int rc = ssh_userauth_password(session, this->user->c_str(), this->pwd->c_str());
  if (rc == SSH_AUTH_SUCCESS)
  {
    this->connected = YXU_CONNECT_AUTH;
    return YXU_SUCCESS;
  }
  else
  {
    if (rc == SSH_AUTH_DENIED)
    {
      // 验证失败：使用其他方法
    }
    else if (rc == SSH_AUTH_PARTIAL)
    {
      // 你已经被部分认证了，你还得用另一种方法
    }
    else if (rc == SSH_AUTH_AGAIN)
    {
      // 在非阻塞模式下，你必须稍后再调用它。
    }
    else if (rc == SSH_AUTH_ERROR)
    {
      // 发生了严重的错误。
    }
    else
    {
      // 验证失败
    }
    vdlog::getInstance()->error("Authentication failed(%d): %s", rc, ssh_get_error(session));
    return YXU_AUTH_WITH_USER_FAILED;
  }
}

int vilsshconn::auth_with_privatekey()
{
  // 1、导入私钥
  auto autocall = [](const char *prompt, char *buf, size_t len, int echo, int verify, void *userdata)
  {
    return 0;
  };
  char *authdata;
  int rc1 = ssh_pki_import_privkey_file(this->privatekey->c_str(), this->pwd->c_str(), autocall, authdata, &pkey);
  if (rc1 != SSH_OK)
  {
    return YXU_IMPORT_PRIVATE_KEY_FAILED;
  }

  // 2、从私钥提取公钥
  ssh_key pubkey;
  int rc2 = ssh_pki_export_privkey_to_pubkey(pkey, &pubkey);
  if (rc2 != SSH_OK)
  {
    vdlog::getInstance()->error("import public key failed: %s", ssh_get_error(session));
    ssh_key_free(pubkey);
    return YXU_AUTH_WITH_PUBLIC_KEY_FAILED;
  }

  // 3、验证服务器是否支持公钥认证
  int rc3 = ssh_userauth_try_publickey(session, this->user->c_str(), pkey);
  if (rc3 != SSH_AUTH_SUCCESS)
  {
    ssh_key_free(pubkey);
    return YXU_HOST_NOT_SUPPORT_PUBLIC_KEY_AUTH;
  }

  // 4、开始身份验证
  int rc = ssh_userauth_publickey(session, this->user->c_str(), pkey);
  if (rc == SSH_AUTH_SUCCESS)
  {
    this->connected = YXU_CONNECT_AUTH;
    return YXU_SUCCESS;
  }
  else
  {
    if (rc == SSH_AUTH_DENIED)
    {
      // 验证失败：使用其他方法
    }
    else if (rc == SSH_AUTH_PARTIAL)
    {
      // 你已经被部分认证了，你还得用另一种方法
    }
    else if (rc == SSH_AUTH_AGAIN)
    {
      // 在非阻塞模式下，你必须稍后再调用它。
    }
    else if (rc == SSH_AUTH_ERROR)
    {
      // 发生了严重的错误。
    }
    else
    {
      // 验证失败
    }
    vdlog::getInstance()->error("Authentication failed(%d): %s", rc, ssh_get_error(session));
    ssh_key_free(pubkey);
    return YXU_AUTH_WITH_PRIVATE_KEY_FAILED;
  }
}

int vilsshconn::getconn()
{
  return this->connected;
}

int vilsshconn::get_connect_state()
{
try_again:
  int rc;
  int blocking_mode = ssh_is_blocking(this->session);
  if (blocking_mode == 1)
  {
    // blocking;
    // vdlog::getInstance()->info("get_connect_state, session is blocking", rc);
  }
  else if (blocking_mode == 0)
  {
    // nonblocking
    // vdlog::getInstance()->info("get_connect_state, session is nonblocking", rc);
  }
  rc = ssh_send_ignore(this->session, "0");
  if (rc == SSH_OK)
  {
    // vdlog::getInstance()->info("get_connect_state, ssh_send_ignore, %d", rc);
  }
  else
  {
    // vdlog::getInstance()->error("(get_connect_state,ssh_send_ignore)Error: %s. Code: %d", ssh_get_error(this->session), ssh_get_error_code(this->session));
    return YXU_CONNECT_FAILED;
  }
  rc = ssh_blocking_flush(this->session, 500);
  if (rc == SSH_OK)
  {
    // vdlog::getInstance()->info("get_connect_state, ssh_blocking_flush, %d", rc);
  }
  else if (rc == SSH_AGAIN)
  {
    // vdlog::getInstance()->info("get_connect_state, SSH_AGAIN, %d", rc);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    goto try_again;
  }
  else
  {
    // vdlog::getInstance()->error("(get_connect_state,ssh_blocking_flush)Error: %s. Code: %d", ssh_get_error(this->session), ssh_get_error_code(this->session));
    return YXU_CONNECT_FAILED;
  }
  int connstat = ssh_get_status(this->session);
  ssh_set_blocking(this->session, blocking_mode);
  if ((connstat & SSH_CLOSED) == SSH_CLOSED || (connstat & SSH_CLOSED_ERROR) == SSH_CLOSED_ERROR)
  {
    vdlog::getInstance()->error("(get_connect_state,ssh_get_status)Error: %s. Code: %d", ssh_get_error(this->session), ssh_get_error_code(this->session));
    return YXU_CONNECT_FAILED;
  }
  else
  {
    vdlog::getInstance()->info("get_connect_state, ssh_get_status, %d", connstat);
    return YXU_SUCCESS;
  }
}
