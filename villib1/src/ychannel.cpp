#include "./ychannel.h"


int ychannel::generate(vilsshconn *c)
{
  conn = new vilsshconn(c);
  int rc = this->conn->create();
  if (rc != YXU_SUCCESS)
  {
    return YXU_GENERATE_CHANNEL_FAILED;
  }
  channel = ssh_channel_new(this->conn->session);
  if (channel == NULL)
  {
    vdlog::getInstance()->error("generate 3: %s", ssh_get_error(this->conn->session));
    return YXU_GENERATE_CHANNEL_FAILED;
  }
  //--
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return YXU_OPEN_CHANNEL_FAILED;
  }
  return YXU_SUCCESS;
}

int ychannel::generate(vilsshconn *c, int pty_cols, int pty_rows)
{
  conn = new vilsshconn(c);
  /// 1.
  int rc = this->conn->create();
  if (rc != YXU_SUCCESS)
  {
    return rc;
  }
  /// 2.
  int ret;
  ret=this->conn->connnect();
  if (ret != YXU_SUCCESS)
  {
    return ret;
  }
  /// 3.
  ret = this->conn->verify_known_host();
  if(ret!=YXU_SUCCESS){
    return ret;
  }
  /// 4.
  ret = this->conn->authhost();
  if(ret!=YXU_SUCCESS){
    return ret;
  }

  channel = ssh_channel_new(this->conn->session);
  if (channel == NULL)
  {
    vdlog::getInstance()->error("generate 3: %s", ssh_get_error(this->conn->session));
    return YXU_GENERATE_CHANNEL_FAILED;
  }
  //--
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return YXU_OPEN_CHANNEL_FAILED;
  }
  //--
  rc = ssh_channel_request_pty_size(channel, "xterm", pty_cols, pty_rows);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return YXU_OPEN_CHANNEL_FAILED;
  }
  //--
  rc = ssh_channel_request_shell(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return YXU_OPEN_CHANNEL_FAILED;
  }
  return YXU_SUCCESS;
}

int ychannel::readdata(std::function<void(char *buffer, int size)> ptr)
{
  try
  {
    char buffer[256];
    std::string txt;
    while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel))
    {
      int nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0);
      if (nbytes < 0){
        vdlog::getInstance()->error("(ychannel::readdata)Error: %s. Code: %d", ssh_get_error(this->conn->session), ssh_get_error_code(this->conn->session));
        txt.clear();
        ptr("",-1);
        return YXU_READ_CHANNEL_FAILED;
      }
      if (nbytes > 0)
      {
        txt.append(buffer, nbytes);
      }
      if(nbytes == 0){
        if(!txt.empty()){
          if(ptr!=nullptr) ptr(txt.data(), txt.size());
          txt.clear();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    }
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    vdlog::getInstance()->info("ychannel readdata error.");
  }
  
  return YXU_SUCCESS;
}

int ychannel::writedata(std::string txt)
{
  int nwritten = ssh_channel_write(channel, txt.c_str(), txt.size());
  if (nwritten == txt.size())
  {
    return YXU_SUCCESS;
  }
  else
  {
    return YXU_WRITE_CHANNEL_FAILED;
  }
}

int ychannel::channelclose()
{
  vdlog::getInstance()->info("channelclose");
  if (channel != NULL)
  {
    ssh_channel_close(channel);
    ssh_channel_send_eof(channel);
    ssh_channel_free(channel);
  }
  if(this->conn!=nullptr){
    delete this->conn;
  }
  return YXU_SUCCESS;
}

int ychannel::resize(int pty_cols, int pty_rows)
{
  int rc = ssh_channel_change_pty_size(channel, pty_cols, pty_rows);
  if (rc != SSH_OK)
  {
    throw YXU_OPEN_CHANNEL_FAILED;
  }
  return YXU_SUCCESS;
}
