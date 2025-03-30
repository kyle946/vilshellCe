#ifndef APPHANDLE_H
#define APPHANDLE_H


#include <napi.h>
#include <iostream>
#include <functional>
#include <memory>
#include <uv.h>

#include "./plugins/CJsonObject.h"
#include "./plugins/uuid4.h"
#include "./common.h"


#ifdef _WIN32
  #include <windows.h>
  #include <shlobj.h>
  #include <iostream>
  #pragma comment(lib, "shell32.lib") // 链接 Shell32.lib
#else
  #include <pwd.h>
  #include <unistd.h>
#endif


#include <vdlog.h>

#include <iostream>
#include <cstdlib>




class apphandle : public Napi::ObjectWrap<apphandle>
{
public:
  static Napi::Value bindFunc(Napi::Env env, Napi::Object ex);
  apphandle(const Napi::CallbackInfo &info);

  std::string appname = "vilshellCe";
  std::string userhome = ""; 
  std::string vilshelldir = "";
  Napi::Env env = nullptr;
  std::map<std::string , JsonObj> values;

private:
  std::string getHomeDirectoryVilshell();
  std::string getHomeDirectory();
  Napi::Value getappinfo(const Napi::CallbackInfo &info);

public:
  Napi::Value writefile(const Napi::CallbackInfo &info);
  Napi::Value readfile(const Napi::CallbackInfo &info);
  Napi::Value sectodate(const Napi::CallbackInfo &info);
  Napi::Value getuuid(const Napi::CallbackInfo &info);
  Napi::Value getHomeDirectory(const Napi::CallbackInfo &info);
  Napi::Value setlog(const Napi::CallbackInfo &info);
  Napi::Value logerr(const Napi::CallbackInfo &info);
  Napi::Value loginfo(const Napi::CallbackInfo &info);
};




inline Napi::Value apphandle::bindFunc(Napi::Env env, Napi::Object exports)
{
  std::initializer_list<Napi::ClassPropertyDescriptor<apphandle>> propertys = {
    InstanceAccessor<&apphandle::getappinfo>("appinfo"),
    InstanceMethod<&apphandle::sectodate>("sectodate", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    InstanceMethod<&apphandle::getuuid>("getuuid", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    InstanceMethod<&apphandle::writefile>("writefile", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    InstanceMethod<&apphandle::readfile>("readfile", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    InstanceMethod<&apphandle::sectodate>("sectodate", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    InstanceMethod<&apphandle::getHomeDirectory>("getHomeDirectory", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    InstanceMethod<&apphandle::logerr>("logerr", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    InstanceMethod<&apphandle::loginfo>("loginfo", static_cast<napi_property_attributes>(napi_writable | napi_configurable))
  };

  Napi::Function func = DefineClass(env, "apphandle", propertys);
  auto constructor = Napi::Persistent(func); // 使用栈分配
  env.SetInstanceData(&constructor); // 存储引用
  exports.Set(Napi::String::New(env, "apphandle"), func); // 显式指定键类型
  return exports;

}


inline apphandle::apphandle(const Napi::CallbackInfo &info) : Napi::ObjectWrap<apphandle>(info)
{
  this->env = info.Env();
  this->userhome = this->getHomeDirectory();
  this->vilshelldir = this->getHomeDirectoryVilshell();
}

#endif