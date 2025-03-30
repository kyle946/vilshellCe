#ifndef vilssh_H
#define vilssh_H

#include <napi.h>
#include <iostream>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
#include <libssh/libssh.h>
#include <ctime>
#include <chrono>

#include "./plugins/uuid4.h"
#include "./common.h"
#include "./yerrcode.h"
#include "./ystruct.h"
#include "./vilsshconn.h"
#include "./ychannel.h"
#include "./ysftp.h"

#include <vdlog.h>




class vilssh : public Napi::ObjectWrap<vilssh>
{

public:
  vilssh(const Napi::CallbackInfo &info);
  ~vilssh();
  static Napi::Value bindFunc(Napi::Env env, Napi::Object ex);
  
  std::string homeuserdir = "";
  std::string hostvilshelldir = "";

private:
  const char *appname = "vilshellCe";
  const char *defaultlabel = "default";
  vilsshconn *defconn = nullptr;
  bool reconnecting = false;

  std::map<std::string, ThreadFuns *> thFuncs;
  std::map<std::string, ThreadFunsSftp *> thFuncsSftp;
  JsonObj connjson;
  Napi::Value get_conn(const Napi::CallbackInfo &info);
  Napi::Value gethostinfo(const Napi::CallbackInfo &info);

public:
  int runscript(Napi::Env env, int code);
  int runscript(Napi::Env env, std::string action, std::string txt = "");
  int geterror(ssh_session session, std::string label = "");

  Napi::Value openconn(const Napi::CallbackInfo &info);
  Napi::Value afterSuccessfulConnection(const Napi::CallbackInfo &info);
  Napi::Value reauth(const Napi::CallbackInfo &info);
  Napi::Value reconn(const Napi::CallbackInfo &info);
  Napi::Value releaseconn(const Napi::CallbackInfo &info);
  Napi::Value hostupdate(const Napi::CallbackInfo &info);
  Napi::Value chanelnew(const Napi::CallbackInfo &info);
  Napi::Value chanelclose(const Napi::CallbackInfo &info);
  Napi::Value chanelwrite(const Napi::CallbackInfo &info);
  Napi::Value chanelresize(const Napi::CallbackInfo &info);
  Napi::Value exists_dir(const Napi::CallbackInfo &info);
  Napi::Value _mkdir(const Napi::CallbackInfo &info);
  Napi::Value _rmdir(const Napi::CallbackInfo &info);
  Napi::Value _rmfile(const Napi::CallbackInfo &info);
  Napi::Value _rename(const Napi::CallbackInfo &info);
  Napi::Value listdir(const Napi::CallbackInfo &info);  
  int execCmd(std::string command, std::string &response, vilsshconn *execCmdConn=NULL);
  Napi::Value execCmd(const Napi::CallbackInfo &info);
  Napi::Value uploadfiles(const Napi::CallbackInfo &info);
  Napi::Value downloadfiles(const Napi::CallbackInfo &info);
  Napi::Value stop_transfer(const Napi::CallbackInfo &info);
  Napi::Value readfilecheck(const Napi::CallbackInfo &info);
  Napi::Value readPermission(const Napi::CallbackInfo &info);
  Napi::Value readfile(const Napi::CallbackInfo &info);
  Napi::Value writefile(const Napi::CallbackInfo &info);
  Napi::Value WritePermissionDir(const Napi::CallbackInfo &info);
  Napi::Value find_service_file(const Napi::CallbackInfo &info);
  Napi::Value get_filetype_and_permission(const Napi::CallbackInfo &info);
  Napi::Value getutc(const Napi::CallbackInfo &info);
  Napi::Value getutc8(const Napi::CallbackInfo &info);
};



inline vilssh::vilssh(const Napi::CallbackInfo &info) : Napi::ObjectWrap<vilssh>(info)
{
}



inline vilssh::~vilssh()
{
  vdlog::getInstance()->warn("enf of vilssh.(destroy)");
}



inline Napi::Value vilssh::bindFunc(Napi::Env env, Napi::Object exports)
{
  auto propertys = {
      InstanceAccessor<&vilssh::get_conn>("conn"),
      InstanceAccessor<&vilssh::gethostinfo>("hostinfo"),
      InstanceMethod<&vilssh::openconn>("openconn", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::afterSuccessfulConnection>("afterSuccessfulConnection", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::reauth>("reauth", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::reconn>("reconn", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::releaseconn>("releaseconn", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::hostupdate>("hostupdate", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::chanelnew>("chanelnew", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::chanelclose>("chanelclose", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::chanelwrite>("chanelwrite", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::chanelresize>("chanelresize", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::exists_dir>("exists_dir", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::WritePermissionDir>("WritePermissionDir", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::find_service_file>("find_service_file", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::get_filetype_and_permission>("get_filetype_and_permission", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      //--
      InstanceMethod<&vilssh::_mkdir>("_mkdir", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::_rmdir>("_rmdir", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::_rmfile>("_rmfile", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::_rename>("_rename", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::execCmd>("execCmd", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::listdir>("listdir", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::uploadfiles>("uploadfiles", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::downloadfiles>("downloadfiles", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::stop_transfer>("stop_transfer", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::readfilecheck>("readfilecheck", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::readPermission>("readPermission", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::readfile>("readfile", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::writefile>("writefile", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::getutc>("getutc", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      InstanceMethod<&vilssh::getutc8>("getutc8", static_cast<napi_property_attributes>(napi_writable | napi_configurable))
  };

  //--------------------
  Napi::Function func = DefineClass(env, "vilssh", propertys);
  Napi::FunctionReference *constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData<Napi::FunctionReference>(constructor);
  exports.Set("vilssh", func);
  return exports;
}




#endif // vilssh_H