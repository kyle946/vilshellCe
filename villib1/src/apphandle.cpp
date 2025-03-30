
#include <napi.h>
#include "apphandle.h"






/**
 * @brief 获取用户家目录
 * 
 * @param info 
 * @return Napi::Value 
 */
std::string apphandle::getHomeDirectory()
{
  std::string homedir="";
  try
  {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path))) {
      homedir = path;
    } else {
      homedir = "";
    }
#else
    const char* homeDir = std::getenv("HOME");
    if (homeDir != nullptr) {
      return std::string(homeDir);
    } else {
      struct passwd* pw = getpwuid(getuid());
      if (pw != nullptr) {
        homedir = std::string(pw->pw_dir);
      } else {
        homedir = "";
      }
    }
#endif
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }
  return homedir;
}



Napi::Value apphandle::getHomeDirectory(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::string homedir=this->getHomeDirectory();
  return Napi::String::New(env, homedir);
}



/**
 * @brief 
 * 
 * @return std::string 
 */
std::string apphandle::getHomeDirectoryVilshell(){
  std::string homedir = this->getHomeDirectory();
#ifdef _WIN32
  homedir.append(DIRSEPARATOR);
  homedir.append(".");
  homedir.append(this->appname);
  common::_mkdir(homedir);
#else
  homedir.append(DIRSEPARATOR);
  homedir.append(".");
  homedir.append(this->appname);
  common::_mkdir(homedir);
#endif
  return homedir;
}


Napi::Value apphandle::getappinfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Object inobj = Napi::Object::New(env);
  inobj.Set("appname", this->appname); 
  inobj.Set("userhome", this->userhome);
  inobj.Set("vilshelldir", this->vilshelldir);
  return inobj;
}



/**
 * @brief 
 *
 * @param {string}  filepath
 * @param {string}  content
 * @return Napi::Value
 */
Napi::Value apphandle::writefile(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (info.Length() < 2)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[0].IsString())
  {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  std::string filepath = info[0].As<Napi::String>().Utf8Value();
  std::string content = info[1].As<Napi::String>().Utf8Value();

  int result = common::writefile(filepath, content);
  if (result != 1)
  {
    Napi::TypeError::New(env, "Error writing file..").ThrowAsJavaScriptException();
    return env.Null();
  }
  else
  {
    return Napi::Number::New(env, 1);
  }
}


/**
 * @brief 
 *
 * @param info
 * @return Napi::Value
 */
Napi::Value apphandle::readfile(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[0].IsString())
  {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string filepath = info[0].As<Napi::String>().Utf8Value();
  std::string content;
  int result = common::readfile(filepath, content);
  if (result != 1)
  {
    Napi::TypeError::New(env, "Error reading file..").ThrowAsJavaScriptException();
    return env.Null();
  }
  else
  {
    std::string body;
    body.append(content.c_str());
    return Napi::String::New(env, body);
  }
}


/**
 * @brief 时间戳转字符串
 * 
 * @param {number}  timestamp
 * @param {number}  _zone   输入0或者8，0表示UTC，8表示中国时间
 * @return Napi::Value 
 */
Napi::Value apphandle::sectodate(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (info.Length() < 2)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[0].IsNumber())
  {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[1].IsNumber())
  {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  uint64_t timestamp = info[0].As<Napi::Number>().Int64Value();
  int _zone = info[1].As<Napi::Number>().Int32Value();
  uint64_t sec1;
  if(_zone==0){
    sec1 = timestamp;
  }
  else{
    sec1 = timestamp;
    sec1 += _zone * 3600;
  }
  std::string timestr;
  struct tm *local = gmtime((const time_t *)&sec1);
  {
    int year = local->tm_year + 1900; // 自 1900 起的年数
    int mon = local->tm_mon + 1;      // 就是现在的11月
    int mday = local->tm_mday;        // 日
    int hour = local->tm_hour;        // 时
    int min = local->tm_min;          // 分
    int sec = local->tm_sec;          // 秒
    char str2[32] = {'\0'};
    sprintf(str2, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, mon, mday, hour, min, sec);
    timestr = str2;
  }
  return Napi::String::New(env, timestr);
}



Napi::Value apphandle::getuuid(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::string uuidstr = vdoc::uuid4();
  return Napi::String::New(env, uuidstr);
}


Napi::Value apphandle::setlog(const Napi::CallbackInfo &info)
{
  vdlog::getInstance()->setSpdlog("vilshellCe", "./");
  vdlog::getInstance()->setHost("127.0.0.1", 2222);
  vdlog::getInstance()->setDebugMode(1);
  return Napi::Value();
}


Napi::Value apphandle::logerr(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if(info.Length()==1){
    std::string content = info[0].As<Napi::String>().Utf8Value();
    vdlog::getInstance()->error(content.c_str());
  } else if(info.Length()==2){
    std::string label = info[0].As<Napi::String>().Utf8Value();
    std::string content = info[1].As<Napi::String>().Utf8Value();
    vdlog::getInstance()->error("%s: %s", label.c_str(), content.c_str());
  }
  return env.Null();
}


Napi::Value apphandle::loginfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if(info.Length()==1){
    std::string content = info[0].As<Napi::String>().Utf8Value();
    vdlog::getInstance()->info(content.c_str());
  } else if(info.Length()==2){
    std::string label = info[0].As<Napi::String>().Utf8Value();
    std::string content = info[1].As<Napi::String>().Utf8Value();
    vdlog::getInstance()->info("%s: %s", label.c_str(), content.c_str());
  }
  return env.Null();
}