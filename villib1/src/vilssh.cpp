
#include "./vilssh.h"




Napi::Value vilssh::get_conn(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (defconn == nullptr)
  {
    return Napi::Number::New(env, 0);
  }
  else
  {
    return Napi::Number::New(env, defconn->getconn());
  }
}



Napi::Value  vilssh::gethostinfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Object robj = Napi::Object::New(env);
  robj.Set("hostvilshelldir", this->hostvilshelldir);
  robj.Set("homeuserdir", this->homeuserdir);
  std::string hostlink = this->connjson("user")+"@"+this->connjson("host")+":"+this->connjson("port");
  robj.Set("hostlink", hostlink);
  return robj;
}


int vilssh::runscript(Napi::Env env, int code)
{
  int rc = 0;
  switch (code)
  {

    // 认证失败
  case YXU_AUTH_WITH_USER_FAILED:
  case YXU_AUTH_WITH_PRIVATE_KEY_FAILED:
  case YXU_IMPORT_PRIVATE_KEY_FAILED:
  case YXU_AUTH_WITH_PUBLIC_KEY_FAILED:
  case YXU_HOST_NOT_SUPPORT_PUBLIC_KEY_AUTH:
    rc = this->runscript(env, "reauth");
    break;

    // 连接失败
  case YXU_CONNECT_FAILED:     // 连接失败
  case YXU_FX_NO_CONNECTION:   // 没有建立连接
  case YXU_FX_CONNECTION_LOST: // 连接丢失
    rc = this->runscript(env, "reconn");
    break;

  case YXU_UNKNOWN:
  case YXU_SUCCESS:
  case YXU_EMPTY:
  case YXU_READ_SUCCESS:
    break;

    // 显示错误
  default:
    std::string _txt = common::sprintf("global.ievt.emit('showerror', %d)", code);
    Napi::String script = Napi::String::New(env, _txt.c_str());
    env.RunScript(script);
    break;
  }
  return rc;
}

int vilssh::runscript(Napi::Env env, std::string action, std::string txt)
{
  // vdlog::getInstance()->info("script action: %s", action.c_str());
  Napi::String script;
  if (action == "reauth")
  {
    script = Napi::String::New(env, "global.ievt.emit('reauth')");
  }
  else if (action == "reconn")
  {
    script = Napi::String::New(env, "global.ievt.emit('reconn')");
  }
  else if (action == "showload")
  {
    if (txt.empty())
    {
      script = Napi::String::New(env, "global.ievt.emit('showload')");
    }
    else
    {
      std::string _txt = common::sprintf("global.ievt.emit('showload','%s')", txt.c_str());
      script = Napi::String::New(env, _txt.c_str());
    }
  }
  else if (action == "closeload")
  {
    script = Napi::String::New(env, "global.ievt.emit('closeload')");
  }
  else if (action == "setuid")
  {
    script = Napi::String::New(env, "global.ievt.emit('setuid')");
  }
  env.RunScript(script);
  return 1;
}

int vilssh::geterror(ssh_session session, std::string label)
{
  int errcode = ssh_get_error_code(session);
  const char *errmsg = ssh_get_error(session);
  vdlog::getInstance()->error("(%s)Error: %s. Code: %d", label.c_str(), errmsg, errcode);
  return errcode;
}



/**
 * @brief 打开连接
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::openconn(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  auto finishedCall = [&promise, &env](int ret) -> Napi::Value
  {
    promise->Reject(Napi::Number::New(env, ret));
    return promise->Promise();
  };
  if (info.Length() < 1)
  {
    return finishedCall(YXU_PARAMETER_ERROR);
  }
  if (!info[0].IsObject())
  {
    return finishedCall(YXU_PARAMETER_ERROR);
  }
  // parsing parameters.
  Napi::Object conn_option_val = info[0].As<Napi::Object>();
  this->connjson = common::object2json(env, conn_option_val);

  std::string host = "";
  int port = 22;
  std::string user = "";
  std::string pwd = "";
  std::string privatekey = "";
  int usepubkey = 0;

  this->connjson.Get("host", host);
  this->connjson.Get("port", port);
  this->connjson.Get("user", user);
  this->connjson.Get("pwd", pwd);
  this->connjson.Get("usepubkey", usepubkey);
  this->connjson.Get("privatekey", privatekey);

  this->defconn = new vilsshconn(
      new std::string(host),
      new int(port),
      new std::string(user),
      new std::string(pwd),
      new std::string(privatekey),
      new int(usepubkey));
  int ret = this->defconn->create();
  if (ret != YXU_SUCCESS)
  {
    return finishedCall(ret);
  }


  ThreadFuns *stru = new ThreadFuns();
  stru->uuid = vdoc::uuid4();
  Napi::Function callback; // = info[1].As<Napi::Function>();
  stru->thread_function_1 = std::move(Napi::ThreadSafeFunction::New(env, callback, "openconn", 0, 1));
  stru->promise = promise;
  thFuncs[stru->uuid] = stru;


  auto call = [&](ThreadFuns *stru)
  {

    auto call_js = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
    {
      if (stru->state == YXU_SUCCESS)
      {
        stru->promise->Resolve(Napi::Number::New(env, stru->state));
      }
      else if (stru->state == YXU_KNOWN_HOSTS_UNKNOWN)
      {
        stru->promise->Resolve(Napi::Number::New(env, stru->state));
      }
      else
      {
        stru->promise->Reject(Napi::Number::New(env, stru->state));
      }

      std::string uuid = stru->uuid;
      delete stru;
      thFuncs.erase(uuid);
    };

    /// 2.
    int ret;
    ret = this->defconn->connnect();
    if (ret != YXU_SUCCESS)
    {
      stru->state = ret;
      stru->thread_function_1.BlockingCall(stru, call_js);
      return;
    }
    /// 3.
    ret = this->defconn->verify_known_host();
    if (ret != YXU_SUCCESS)
    {
      stru->state = ret;
      stru->thread_function_1.BlockingCall(stru, call_js);
      return;
    }
    /// 4.
    ret = this->defconn->authhost();
    if (ret != YXU_SUCCESS)
    {
      stru->state = ret;
      stru->thread_function_1.BlockingCall(stru, call_js);
      return;
    }
    ///
    stru->state = ret;
    stru->thread_function_1.BlockingCall(stru, call_js);
    return;
  };
  thFuncs[stru->uuid]->th = new std::thread(call, stru);
  return promise->Promise();
}

/**
 * @brief 连接成功之后 
 *
 * @param info
 * @return Napi::Value
 */
Napi::Value vilssh::afterSuccessfulConnection(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  auto finishedCall = [&promise, &env](int ret) -> Napi::Value
  {
    promise->Reject(Napi::Number::New(env, ret));
    return promise->Promise();
  };

  ThreadFuns *stru = new ThreadFuns();
  stru->uuid = vdoc::uuid4();
  Napi::Function callback; // = info[1].As<Napi::Function>();
  stru->thread_function_1 = std::move(Napi::ThreadSafeFunction::New(env, callback, "afterSuccessfulConnection", 0, 1));
  stru->promise = promise;
  thFuncs[stru->uuid] = stru;

  auto thread_function = [&](ThreadFuns *stru)
  {
    auto JavaScriptCallback = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
    {
      if (stru->state == YXU_SUCCESS)
      {
        stru->promise->Resolve(Napi::Number::New(env, stru->state));
      }
      else
      {
        stru->promise->Reject(Napi::Number::New(env, stru->state));
      }

      std::string uuid = stru->uuid;
      delete stru;
      thFuncs.erase(uuid);
    };
    std::string response;
    std::shared_ptr<vilsshconn> execCmdConn = std::make_shared<vilsshconn>(this->defconn);
    int ret = this->execCmd("id -u", response, execCmdConn.get());
    if(ret==YXU_SUCCESS)
    {
      *this->defconn->uid = common::trim(response);
      response.clear();
      ret = this->execCmd("cd ~; pwd", response, execCmdConn.get());
      vdlog::getInstance()->info("uid: %s", this->defconn->uid->c_str());
    }
    if(ret==YXU_SUCCESS)
    {
      this->homeuserdir = common::trim(response);
      response.clear();
      this->hostvilshelldir = homeuserdir + "/." + this->appname;
      this->execCmd("mkdir -m 777 -p "+this->hostvilshelldir, response, execCmdConn.get());
      vdlog::getInstance()->info("homeuserdir: %s", this->homeuserdir.c_str());
      vdlog::getInstance()->info("hostvilshelldir: %s", this->hostvilshelldir.c_str());
    }
    stru->state = ret;
    stru->thread_function_1.BlockingCall(stru, JavaScriptCallback);
  };
  thFuncs[stru->uuid]->th = new std::thread(thread_function, stru);
  return promise->Promise();
}


/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::reauth(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    // parsing parameters.
    std::string _pwd = info[0].As<Napi::String>().Utf8Value();

    ThreadFuns *stru = new ThreadFuns();
    stru->uuid = vdoc::uuid4();
    Napi::Function callback; // = info[1].As<Napi::Function>();
    stru->thread_function_1 = std::move(Napi::ThreadSafeFunction::New(env, callback, "reauth", 0, 1));
    stru->promise = promise;
    thFuncs[stru->uuid] = stru;

    this->defconn->pwd->clear();
    this->defconn->pwd->append(_pwd);
    /// 1.
    int ret = this->defconn->create();
    if (ret != YXU_SUCCESS)
    {
      throw ret;
    }

    auto call = [&](ThreadFuns *stru)
    {
      auto call_js = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        if (stru->state == YXU_SUCCESS)
        {
          stru->promise->Resolve(Napi::Number::New(env, stru->state));
        }
        else if (stru->state == YXU_KNOWN_HOSTS_UNKNOWN)
        {
          stru->promise->Resolve(Napi::Number::New(env, stru->state));
        }
        else
        {
          stru->promise->Reject(Napi::Number::New(env, stru->state));
        }

        std::string uuid = stru->uuid;
        delete stru;
        thFuncs.erase(uuid);
      };

      /// 2.
      int ret;
      ret = this->defconn->connnect();
      if (ret != YXU_SUCCESS)
      {
        stru->state = ret;
        stru->thread_function_1.BlockingCall(stru, call_js);
        return;
      }
      /// 3.
      ret = this->defconn->verify_known_host();
      if (ret != YXU_SUCCESS)
      {
        stru->state = ret;
        stru->thread_function_1.BlockingCall(stru, call_js);
        return;
      }
      /// 4.
      ret = this->defconn->authhost();
      if (ret != YXU_SUCCESS)
      {
        stru->state = ret;
        stru->thread_function_1.BlockingCall(stru, call_js);
        return;
      }
      ///
      stru->state = ret;
      stru->thread_function_1.BlockingCall(stru, call_js);
      return;
    };
    thFuncs[stru->uuid]->th = new std::thread(call, stru);
  }
  catch (int err)
  {
    promise->Reject(Napi::Number::New(env, err));
  }
  return promise->Promise();
}


/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::reconn(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  auto finishedCall = [&promise, &env](int ret) -> Napi::Value
  {
    promise->Reject(Napi::Number::New(env, ret));
    return promise->Promise();
  };

  if (this->reconnecting)
  {
    return finishedCall(YXU_EMPTY);
  }

  this->reconnecting = true;

  std::string host = "";
  int port = 22;
  std::string user = "";
  std::string pwd = "";
  std::string privatekey = "";
  int usepubkey = 0;

  this->connjson.Get("host", host);
  this->connjson.Get("port", port);
  this->connjson.Get("user", user);
  this->connjson.Get("pwd", pwd);
  this->connjson.Get("usepubkey", usepubkey);
  this->connjson.Get("privatekey", privatekey);

  this->defconn = new vilsshconn(
      new std::string(host),
      new int(port),
      new std::string(user),
      new std::string(pwd),
      new std::string(privatekey),
      new int(usepubkey));

  int ret = this->defconn->create();
  if (ret != YXU_SUCCESS)
  {
    return finishedCall(ret);
  }

  ThreadFuns *stru = new ThreadFuns();
  stru->uuid = vdoc::uuid4();
  Napi::Function callback; // = info[1].As<Napi::Function>();
  stru->thread_function_1 = std::move(Napi::ThreadSafeFunction::New(env, callback, "reconn", 0, 1));
  stru->promise = promise;
  thFuncs[stru->uuid] = stru;


  auto call = [&](ThreadFuns *stru)
  {

    auto call_js = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
    {
      this->reconnecting = false;
      if (stru->state == YXU_SUCCESS)
      {
        stru->promise->Resolve(Napi::Number::New(env, stru->state));
      }
      else if (stru->state == YXU_KNOWN_HOSTS_UNKNOWN)
      {
        stru->promise->Resolve(Napi::Number::New(env, stru->state));
      }
      else
      {
        stru->promise->Reject(Napi::Number::New(env, stru->state));
      }

      std::string uuid = stru->uuid;
      delete stru;
      thFuncs.erase(uuid);
    };


    int ret;
    ret = this->defconn->connnect();
    if (ret != YXU_SUCCESS)
    {
      stru->state = ret;
      stru->thread_function_1.BlockingCall(stru, call_js);
      return;
    }

    ret = this->defconn->verify_known_host();
    if (ret != YXU_SUCCESS)
    {
      stru->state = ret;
      stru->thread_function_1.BlockingCall(stru, call_js);
      return;
    }

    ret = this->defconn->authhost();
    if (ret != YXU_SUCCESS)
    {
      stru->state = ret;
      stru->thread_function_1.BlockingCall(stru, call_js);
      return;
    }

    stru->state = ret;
    stru->thread_function_1.BlockingCall(stru, call_js);
    return;
  };
  thFuncs[stru->uuid]->th = new std::thread(call, stru);
  return promise->Promise();
}


/**
 * @brief 
 *
 * @param info
 * @return Napi::Value
 */
Napi::Value vilssh::releaseconn(const Napi::CallbackInfo &info)
{
  return Napi::Value();
}


/**
 * @brief 
 *
 * @param info
 * @return Napi::Value
 */
Napi::Value vilssh::hostupdate(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  int ret = this->defconn->verify_known_host_update();
  if (ret != YXU_SUCCESS)
  {
    return Napi::Number::New(env, ret);
  }
  else
  {
    return Napi::Number::New(env, YXU_SUCCESS);
  }
}



/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::chanelnew(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 5)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[1].IsNumber())
      throw YXU_PARAMETER_ERROR;
    if (!info[2].IsNumber())
      throw YXU_PARAMETER_ERROR;
    if (!info[3].IsFunction())
      throw YXU_PARAMETER_ERROR;
    if (!info[4].IsFunction()){
      throw YXU_PARAMETER_ERROR;
    }


    std::string label = "channel:"+info[0].As<Napi::String>().Utf8Value();
    int pty_cols = info[1].As<Napi::Number>().Int32Value();
    int pty_rows = info[2].As<Napi::Number>().Int32Value();
    Napi::Function callback1 = info[3].As<Napi::Function>();
    Napi::Function callback2 = info[4].As<Napi::Function>();

    if (thFuncs.count(label) > 0)
    {
      throw YXU_OPERATION_FAILED;
    }

    ychannel *chl = new ychannel();
    ThreadFuns *stru = new ThreadFuns();
    stru->ptr = (void *)chl;
    stru->uuid = label;
    stru->thread_function_1 = std::move(Napi::ThreadSafeFunction::New(env, callback1, "chanelnew", 0, 1));
    stru->thread_function_2 = std::move(Napi::ThreadSafeFunction::New(env, callback2, "chanelnew", 0, 1));
    thFuncs[label] = stru;


    auto call = [&](ThreadFuns *stru)
    {
      auto chl = (ychannel *)stru->ptr;
      auto calljs = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        jsCallback({Napi::String::New(env, stru->data)});
        stru->data.clear();
      };
      auto calljs_fail = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        jsCallback({Napi::Number::New(env, stru->state)});
        this->runscript(env, stru->state);
      };

      int rc = chl->generate(this->defconn, pty_cols, pty_rows);
      if (rc == YXU_SUCCESS)
      {
        stru->state = YXU_SUCCESS;
        stru->thread_function_2.BlockingCall(stru, calljs_fail);
        //--
        auto callread = [&](char *buffer, int size)
        {
          if (size < 0)
          {
            stru->state = YXU_READ_CHANNEL_FAILED;
            stru->thread_function_2.BlockingCall(stru, calljs_fail);
          }
          if (size > 0)
          {
            stru->data.append(buffer, size);
            stru->state = YXU_READ_SUCCESS;
            stru->thread_function_1.BlockingCall(stru, calljs);
          }
        };
        chl->readdata(callread);
      }
      else
      {
        stru->state = rc;
        stru->thread_function_2.BlockingCall(stru, calljs_fail);
      }
    };
    stru->th = new std::thread(call, stru);
  }
  catch (int err)
  {
    return Napi::Number::New(env, err);
  }
  return Napi::Number::New(env, YXU_SUCCESS);
}



/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::chanelclose(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string label = "channel:"+info[0].As<Napi::String>().Utf8Value();
    {
      if (thFuncs.count(label) == 0)
      {
        throw YXU_OPERATION_FAILED;
      }
      if(thFuncs.count(label)==0){
        throw YXU_UNKNOWN;
      }
      if(thFuncs[label]==NULL){
        throw YXU_UNKNOWN;
      }
      if(thFuncs[label]->ptr==NULL){
        throw YXU_UNKNOWN;
      }
      ThreadFuns *stru = thFuncs[label];
      if (stru == NULL)
      {
        throw YXU_OPERATION_FAILED;
      }
      ychannel *chl = (ychannel *)stru->ptr;
      chl->channelclose();
      delete chl;
      chl = NULL;
      stru->ptr = NULL;

      delete stru;
      stru = NULL;
      thFuncs.erase(label);
    }
    return Napi::Number::New(env, YXU_SUCCESS);
  }
  catch (int err)
  {
    return Napi::Number::New(env, err);
  }
}


/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::chanelwrite(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 2)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[1].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string label = "channel:"+info[0].As<Napi::String>().Utf8Value();
    std::string txt = info[1].As<Napi::String>().Utf8Value();
    //--
    if(thFuncs.count(label)==0){
      throw YXU_UNKNOWN;
    }
    if(thFuncs[label]==NULL){
      throw YXU_UNKNOWN;
    }
    if(thFuncs[label]->ptr==NULL){
      throw YXU_UNKNOWN;
    }
    ychannel *channel = (ychannel *)thFuncs[label]->ptr;
    int rc = channel->writedata(txt);
    if (rc == YXU_SUCCESS)
    {
      return Napi::Number::New(env, YXU_SUCCESS);
    }
    else
    {
      return Napi::Number::New(env, YXU_OPERATION_FAILED);
    }
  }
  catch (int err)
  {
    return Napi::Number::New(env, err);
  }
  return Napi::Number::New(env, YXU_SUCCESS);
}


Napi::Value vilssh::chanelresize(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 3)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[1].IsNumber())
      throw YXU_PARAMETER_ERROR;
    if (!info[2].IsNumber())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string label = "channel:"+info[0].As<Napi::String>().Utf8Value();
    int pty_cols = info[1].As<Napi::Number>().Int32Value();
    int pty_rows = info[2].As<Napi::Number>().Int32Value();

    ychannel *channel = (ychannel *)thFuncs[label]->ptr;
    if(channel==nullptr){
      throw YXU_OPEN_CHANNEL_FAILED;
    }
    int rc = channel->resize(pty_cols, pty_rows);
    if (rc != YXU_SUCCESS)
    {
      throw YXU_OPEN_CHANNEL_FAILED;
    }
  }
  catch (int err)
  {
    return Napi::Number::New(env, err);
  }
  return Napi::Number::New(env, YXU_SUCCESS);
}


/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::exists_dir(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _path = info[0].As<Napi::String>().Utf8Value();
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    int ret = defsftp->exists_dir(_path);
    return Napi::Number::New(env, ret);
  }
  catch (int err)
  {
    return Napi::Number::New(env, YXU_OPERATION_FAILED);
  }
}


/**
 * @brief 创建目录
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::_mkdir(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  try
  {
    if (info.Length() < 1){
      throw YXU_PARAMETER_ERROR;
    }
    if (!info[0].IsString()){
      throw YXU_PARAMETER_ERROR;
    }
    // 传入参数
    ThreadFuns *stru = new ThreadFuns();
    stru->uuid = vdoc::uuid4();
    stru->data = info[0].As<Napi::String>().Utf8Value();
    Napi::Function callback; // = info[1].As<Napi::Function>();
    stru->thread_function_1 = Napi::ThreadSafeFunction::New(env, callback, "reauth", 0, 1);
    stru->promise = promise;
    thFuncs[stru->uuid] = stru;
    //开始多线程
    thFuncs[stru->uuid]->th = new std::thread([&](ThreadFuns *stru)
    {
      auto JavaScriptCallbackFun = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        if (stru->state == YXU_SUCCESS)
        {
          stru->promise->Resolve(Napi::Number::New(env, stru->state));
        }
        else
        {
          stru->promise->Reject(Napi::Number::New(env, stru->state));
        }
        // 释放线程资源
        std::string uuid = stru->uuid;
        delete stru;
        thFuncs.erase(uuid);
      };
      /// start job ===================================
      std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
      stru->state = defsftp->generate();
      if(stru->state == YXU_SUCCESS){
        defsftp->_mkdir(stru->data);
      }
      stru->thread_function_1.BlockingCall(stru, JavaScriptCallbackFun);
      return;
      /// end job
    }, stru);
  }
  catch (int err)
  {
    promise->Reject(Napi::Number::New(env, err));
  }
  return promise->Promise();
}


/**
 * @brief 删除目录
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::_rmdir(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  try
  {
    if (info.Length() < 1){
      throw YXU_PARAMETER_ERROR;
    }
    if (!info[0].IsString()){
      throw YXU_PARAMETER_ERROR;
    }
    // 传入参数
    ThreadFuns *stru = new ThreadFuns();
    stru->uuid = vdoc::uuid4();
    stru->data = info[0].As<Napi::String>().Utf8Value();
    Napi::Function callback; // = info[1].As<Napi::Function>();
    stru->thread_function_1 = Napi::ThreadSafeFunction::New(env, callback, "reauth", 0, 1);
    stru->promise = promise;
    thFuncs[stru->uuid] = stru;
    //开始多线程
    thFuncs[stru->uuid]->th = new std::thread([&](ThreadFuns *stru)
    {
      auto JavaScriptCallbackFun = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        if (stru->state == YXU_SUCCESS)
        {
          stru->promise->Resolve(Napi::Number::New(env, stru->state));
        }
        else
        {
          stru->promise->Reject(Napi::Number::New(env, stru->state));
        }
        // 释放线程资源
        std::string uuid = stru->uuid;
        delete stru;
        thFuncs.erase(uuid);
      };
      /// start job ===================================
      std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
      stru->state = defsftp->generate();
      if(stru->state == YXU_SUCCESS){
        defsftp->_rmdir(stru->data);
      }
      stru->thread_function_1.BlockingCall(stru, JavaScriptCallbackFun);
      return;
      /// end job
    }, stru);
  }
  catch (int err)
  {
    promise->Reject(Napi::Number::New(env, err));
  }
  return promise->Promise();
}


/**
 * 
 * @brief 删除文件
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::_rmfile(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _path = info[0].As<Napi::String>().Utf8Value();
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    int ret = defsftp->_rmfile(_path);
    return Napi::Number::New(env, ret);
  }
  catch (int err)
  {
    return Napi::Number::New(env, YXU_OPERATION_FAILED);
  }
}


/**
 * @brief 重命名文件夹
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::_rename(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 2)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString() || !info[1].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _ori = info[0].As<Napi::String>().Utf8Value();
    std::string _new = info[1].As<Napi::String>().Utf8Value();
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    int ret = defsftp->_rename(_ori, _new);
    return Napi::Number::New(env, ret);
  }
  catch (int err)
  {
    return Napi::Number::New(env, YXU_OPERATION_FAILED);
  }
}

/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
int vilssh::execCmd(std::string cammand, std::string &response, vilsshconn *execCmdConn)
{
  int response_status;
  {
    //是不是已经连接成功了，避免重复连接
    if(execCmdConn->connected!=YXU_CONNECT_AUTH){
      response_status = execCmdConn->create();
      if(response_status==YXU_SUCCESS) {
        response_status = execCmdConn->connnect();
      }
      if(response_status==YXU_SUCCESS) {
        response_status = execCmdConn->authhost();
      }
      if(response_status!=YXU_SUCCESS) {
        return response_status;
      }
    }
ssh_again_label22:
    /// 创建 channel
    ssh_channel channel = ssh_channel_new(execCmdConn->session);
    if (channel == NULL)
    {
      int errcode = this->geterror(execCmdConn->session, "ssh_channel_new");
      response_status = YXU_CREATE_CHANNEL_FAILED;
      return response_status;
    }
    /// 打开Session
    int rc = ssh_channel_open_session(channel);
    if(rc==SSH_OK){
      // Done.
    }
    else if(rc==SSH_AGAIN){
      ssh_channel_close(channel);
      ssh_channel_send_eof(channel);
      ssh_channel_free(channel);
      channel=NULL;;
      goto ssh_again_label22;
    }
    else
    {
      int errcode = this->geterror(execCmdConn->session, "ssh_channel_open_session");
      response_status =  YXU_CONNECT_FAILED;
      return response_status;
    }
    /// 执行命令
    rc = ssh_channel_request_exec(channel, cammand.c_str());
    if(rc==SSH_OK){
      // Done.
    }
    else if(rc==SSH_AGAIN){
      ssh_channel_close(channel);
      ssh_channel_send_eof(channel);
      ssh_channel_free(channel);
      channel=NULL;
      goto ssh_again_label22;
    }
    else
    {
      ssh_channel_close(channel);
      ssh_channel_send_eof(channel);
      ssh_channel_free(channel);
      channel=NULL;
      goto ssh_again_label22;
      int errcode = this->geterror(execCmdConn->session, "ssh_channel_request_exec");
      response_status =  YXU_CONNECT_FAILED;
      return response_status;
    }

    /// 读取数据
    char buffer[256];
    int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0)
    {
      response.append(buffer, nbytes);
      nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
      response_status =  YXU_SUCCESS;
    }
    if (nbytes < 0)
    {
      ssh_channel_close(channel);
      ssh_channel_send_eof(channel);
      ssh_channel_free(channel);
      channel=NULL;
      int errcode = this->geterror(execCmdConn->session, "ssh_channel_read");
      response_status =  YXU_SSH_READ_FAILED;
      return response_status;
    }

    /// 读取错误
    std::string errmsg;
    char errbuffer[256];
    int errnbytes = ssh_channel_read(channel, errbuffer, sizeof(errbuffer), 1);
    while (errnbytes > 0)
    {
      errmsg.append(errbuffer, errnbytes);
      errnbytes = ssh_channel_read(channel, errbuffer, sizeof(errbuffer), 1);
    }
    if(!errmsg.empty()){
      vdlog::getInstance()->error("%s", errmsg.c_str());
      response.clear();
      response.append(errmsg);
      ssh_channel_close(channel);
      ssh_channel_send_eof(channel);
      ssh_channel_free(channel);
      channel=NULL;
      response_status =  YXU_RUN_COMMAND_FAILED;
    }
    ///
    ssh_channel_close(channel);
    ssh_channel_send_eof(channel);
    ssh_channel_free(channel);
  }
  return response_status;
}


/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::execCmd(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  try
  {
    if (info.Length() < 1){
      throw YXU_PARAMETER_ERROR;
    }
    if (!info[0].IsString()){
      throw YXU_PARAMETER_ERROR;
    }

    ThreadFuns *stru = new ThreadFuns();
    stru->uuid = vdoc::uuid4();
    stru->data = info[0].As<Napi::String>().Utf8Value();
    Napi::Function callback; // = info[1].As<Napi::Function>();
    stru->thread_function_1 = Napi::ThreadSafeFunction::New(env, callback, "reauth", 0, 1);
    stru->promise = promise;
    thFuncs[stru->uuid] = stru;

    thFuncs[stru->uuid]->th = new std::thread([&](ThreadFuns *stru)
    {

      auto JavaScriptCallbackFun = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        if (stru->state == YXU_SUCCESS)
        {
          stru->promise->Resolve(Napi::String::New(env, stru->response));
        }
        else
        {
          stru->promise->Reject(Napi::Number::New(env, stru->state));
        }

        std::string uuid = stru->uuid;
        delete stru;
        thFuncs.erase(uuid);
      };

      std::shared_ptr<vilsshconn> execCmdConn = std::make_shared<vilsshconn>(this->defconn);
      stru->state = this->execCmd(stru->data, stru->response, execCmdConn.get());
      stru->thread_function_1.BlockingCall(stru, JavaScriptCallbackFun);
      return;

    }, stru);
  }
  catch (int err)
  {
    promise->Reject(Napi::Number::New(env, err));
  }
  return promise->Promise();
}



/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::listdir(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  try
  {
    if (info.Length() < 1){
      throw YXU_PARAMETER_ERROR;
    }
    if (!info[0].IsString()){
      throw YXU_PARAMETER_ERROR;
    }
    struct tmpstr1 {
      std::string path;
      JsonObj arr;
    };

    ThreadFuns *stru = new ThreadFuns();
    stru->uuid = vdoc::uuid4();
    stru->ptr = new tmpstr1;
    ((struct tmpstr1 *)stru->ptr)->path = info[0].As<Napi::String>().Utf8Value();
    Napi::Function callback; // = info[1].As<Napi::Function>();
    stru->thread_function_1 = Napi::ThreadSafeFunction::New(env, callback, "reauth", 0, 1);
    stru->promise = promise;
    thFuncs[stru->uuid] = stru;

    thFuncs[stru->uuid]->th = new std::thread([&](ThreadFuns *stru)
    {
      auto JavaScriptCallbackFun = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        if (stru->state == YXU_SUCCESS)
        {
          Napi::Object robj = common::jsonobj_to_napi_array(env, ((struct tmpstr1 *)stru->ptr)->arr);
          stru->promise->Resolve(robj);
        }
        else
        {
          stru->promise->Reject(Napi::Number::New(env, stru->state));
        }

        std::string uuid = stru->uuid;
        delete stru;
        thFuncs.erase(uuid);
      };

      std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
      defsftp->generate();
      stru->state = defsftp->listdir(((struct tmpstr1 *)stru->ptr)->path, ((struct tmpstr1 *)stru->ptr)->arr);
      stru->thread_function_1.BlockingCall(stru, JavaScriptCallbackFun);
      return;

    }, stru);
  }
  catch (int err)
  {
    promise->Reject(Napi::Number::New(env, err));
  }
  return promise->Promise();
}



Napi::Value vilssh::uploadfiles(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 6)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[1].IsObject())
      throw YXU_PARAMETER_ERROR;
    if (!info[2].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[3].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[4].IsFunction())
      throw YXU_PARAMETER_ERROR;
    if (!info[5].IsFunction()){
      throw YXU_PARAMETER_ERROR;
    }
    //--
    std::string uuidstr = info[0].As<Napi::String>().Utf8Value();
    Napi::Object filesobj = info[1].As<Napi::Object>();
    std::string source_path = info[2].As<Napi::String>().Utf8Value();
    std::string destination_path = info[3].As<Napi::String>().Utf8Value();
    Napi::Function ptr_progress = info[4].As<Napi::Function>();
    Napi::Function ptr_numbers = info[5].As<Napi::Function>();

    //--
    ThreadFunsSftp *stru = new ThreadFunsSftp();
    stru->uuid = uuidstr;
    stru->files = common::napi_array_to_vectorstr(env, filesobj);
    stru->source_path = source_path;
    stru->destination_path = destination_path;
    stru->fun1 = Napi::ThreadSafeFunction::New(env, ptr_progress, "uploadfiles", 0, 1);
    stru->fun2 = Napi::ThreadSafeFunction::New(env, ptr_numbers, "uploadfiles", 0, 1);
    thFuncsSftp[stru->uuid] = stru;

    auto call = [&](ThreadFunsSftp *stru)
    {
      //--
      auto ptr_progress_call_js = [&](Napi::Env env, Napi::Function jsCallback, ThreadFunsSftp *stru)
      {
        Napi::Object robj = Napi::Object::New(env);
        robj.Set("name", stru->ptr_progress_name);
        robj.Set("size", stru->ptr_progress_size);
        robj.Set("count", stru->ptr_progress_count);
        robj.Set("code", stru->response_code);
        jsCallback.Call({robj});
      };
      auto ptr_numbers_call_js = [&](Napi::Env env, Napi::Function jsCallback, ThreadFunsSftp *stru)
      {
        Napi::Object robj = Napi::Object::New(env);
        robj.Set("num", stru->ptr_numbers_num);
        robj.Set("count", stru->ptr_numbers_count);
        robj.Set("code", stru->response_code);
        jsCallback.Call({robj});
        if (stru->ptr_numbers_num == stru->ptr_numbers_count)
        {

          if (stru == nullptr)
          {
            return;
          }
          std::string uuid = stru->uuid;
          if (thFuncsSftp.count(uuid) == 0)
          {
            return;
          }
          if (stru->sftpobj != nullptr)
          {
            delete stru->sftpobj;
            stru->sftpobj = nullptr;
          }
          delete stru;
          thFuncsSftp.erase(uuid);
        }
      };

      //--
      auto ptr_progress_call = [&](std::string filename, int _size, int _count)
      {
        stru->ptr_progress_name = filename;
        stru->ptr_progress_size = _size;
        stru->ptr_progress_count = _count;
        stru->fun1.BlockingCall(stru, ptr_progress_call_js);
      };
      auto ptr_numbers_call = [&](int _num, int _count, int code)
      {
        stru->ptr_numbers_num = _num;
        stru->ptr_numbers_count = _count;
        stru->response_code = code;
        stru->fun2.BlockingCall(stru, ptr_numbers_call_js);
      };

      stru->sftpobj = new ysftp(this->defconn);
      int rc = stru->sftpobj->generate();
      if (rc == YXU_SUCCESS)
      {
        stru->sftpobj->transferlock = &stru->transferlock;
        stru->sftpobj->stoptransfer = &stru->stoptransfer;
        stru->sftpobj->uploadfiles(stru->files, stru->source_path, stru->destination_path, ptr_progress_call, ptr_numbers_call);
      }
      else
      {
        stru->response_code = rc;
        //--
        stru->ptr_progress_name = "";
        stru->ptr_progress_size = -1;
        stru->ptr_progress_count = -1;
        stru->fun1.BlockingCall(stru, ptr_progress_call_js);
        //--
        stru->ptr_numbers_num = -1;
        stru->ptr_numbers_count = -1;
        stru->fun2.BlockingCall(stru, ptr_numbers_call_js);
      }
    };
    thFuncsSftp[stru->uuid]->th = new std::thread(call, stru);
  }
  catch (int err)
  {
    return Napi::Number::New(env, err);
  }
  return Napi::Number::New(env, YXU_SUCCESS);
}


Napi::Value vilssh::downloadfiles(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 6)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[1].IsObject())
      throw YXU_PARAMETER_ERROR;
    if (!info[2].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[3].IsString())
      throw YXU_PARAMETER_ERROR;
    if (!info[4].IsFunction())
      throw YXU_PARAMETER_ERROR;
    if (!info[5].IsFunction())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string uuidstr = info[0].As<Napi::String>().Utf8Value();
    Napi::Object filesobj = info[1].As<Napi::Object>();
    std::string source_path = info[2].As<Napi::String>().Utf8Value();
    std::string destination_path = info[3].As<Napi::String>().Utf8Value();
    Napi::Function ptr_progress = info[4].As<Napi::Function>();
    Napi::Function ptr_numbers = info[5].As<Napi::Function>();
    //--
    ThreadFunsSftp *stru = new ThreadFunsSftp();
    stru->uuid = uuidstr;
    stru->files = common::napi_array_to_vectorstr(env, filesobj);
    stru->source_path = source_path;
    stru->destination_path = destination_path;
    stru->fun1 = Napi::ThreadSafeFunction::New(env, ptr_progress, "downloadfiles", 0, 1);
    stru->fun2 = Napi::ThreadSafeFunction::New(env, ptr_numbers, "downloadfiles", 0, 1);
    thFuncsSftp[stru->uuid] = stru;

    auto call = [&](ThreadFunsSftp *stru)
    {
      //--
      auto ptr_progress_call_js = [&](Napi::Env env, Napi::Function jsCallback, ThreadFunsSftp *stru)
      {
        Napi::Object robj = Napi::Object::New(env);
        robj.Set("name", stru->ptr_progress_name);
        robj.Set("size", stru->ptr_progress_size);
        robj.Set("count", stru->ptr_progress_count);
        jsCallback.Call({robj});
      };
      auto ptr_numbers_call_js = [&](Napi::Env env, Napi::Function jsCallback, ThreadFunsSftp *stru)
      {
        Napi::Object robj = Napi::Object::New(env);
        robj.Set("num", stru->ptr_numbers_num);
        robj.Set("count", stru->ptr_numbers_count);
        robj.Set("code", stru->response_code);
        jsCallback.Call({robj});
        if (stru->ptr_numbers_num == stru->ptr_numbers_count)
        {

          if (stru == nullptr)
          {
            return;
          }
          std::string uuid = stru->uuid;
          if (thFuncsSftp.count(uuid) == 0)
          {
            return;
          }
          if (stru->sftpobj != nullptr)
          {
            delete stru->sftpobj;
            stru->sftpobj = nullptr;
          }
          delete stru;
          thFuncsSftp.erase(uuid);
        }
      };

      //--
      auto ptr_progress_call = [&](std::string filename, int _size, int _count)
      {
	      uint64_t timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()); // 获取时间戳（毫秒）
        stru->ptr_progress_name = filename;
        stru->ptr_progress_size = _size;
        stru->ptr_progress_count = _count;
        stru->fun1.BlockingCall(stru, ptr_progress_call_js); // ptr_progress
      };
      auto ptr_numbers_call = [&](int _num, int _count, int code)
      {
        stru->ptr_numbers_num = _num;
        stru->ptr_numbers_count = _count;
        stru->response_code = code;
        stru->fun2.BlockingCall(stru, ptr_numbers_call_js); // ptr_numbers
      };

      stru->sftpobj = new ysftp(this->defconn);
      int rc = stru->sftpobj->generate();
      if (rc == YXU_SUCCESS)
      {
        stru->sftpobj->transferlock = &stru->transferlock;
        stru->sftpobj->stoptransfer = &stru->stoptransfer;
        stru->sftpobj->downloadfiles(stru->files, stru->source_path, stru->destination_path, ptr_progress_call, ptr_numbers_call);
      }
      else
      {
        stru->response_code = rc;
        //--
        stru->ptr_progress_name = "";
        stru->ptr_progress_size = -1;
        stru->ptr_progress_count = -1;
        stru->fun1.BlockingCall(stru, ptr_progress_call_js);
        //--
        stru->ptr_numbers_num = -1;
        stru->ptr_numbers_count = -1;
        stru->fun2.BlockingCall(stru, ptr_numbers_call_js);
      }
    };
    thFuncsSftp[stru->uuid]->th = new std::thread(call, stru);
  }
  catch (int err)
  {
    return Napi::Number::New(env, err);
  }
  return Napi::Number::New(env, YXU_SUCCESS);
}


Napi::Value vilssh::stop_transfer(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
    {
      throw YXU_PARAMETER_ERROR;
    }
    std::string _uuid = info[0].As<Napi::String>().Utf8Value();
    if (thFuncsSftp.count(_uuid) == 1)
    {
      ThreadFunsSftp *stru = thFuncsSftp[_uuid];
      if (stru != NULL)
      {
        stru->transferlock.lock();
        stru->stoptransfer = 1;
        stru->transferlock.unlock();
      }
    }
    return Napi::Number::New(env, YXU_SUCCESS);
  }
  catch (int err)
  {
    return Napi::Number::New(env, YXU_OPERATION_FAILED);
  }
}

Napi::Value vilssh::readfilecheck(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _path = info[0].As<Napi::String>().Utf8Value();
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    int ret = defsftp->_readfilecheck(_path);
    return Napi::Number::New(env, ret);
  }
  catch (int err)
  {
    return Napi::Number::New(env, YXU_OPERATION_FAILED);
  }
}


Napi::Value vilssh::readPermission(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _path = info[0].As<Napi::String>().Utf8Value();
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    int ret = defsftp->_readPermission(_path);
    return Napi::Number::New(env, ret);
  }
  catch (int err)
  {
    return Napi::Number::New(env, YXU_OPERATION_FAILED);
  }
}


Napi::Value vilssh::readfile(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _path = info[0].As<Napi::String>().Utf8Value();
    std::string _content;
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    int ret = defsftp->_readfile(_path, _content);
    // this->runscript(env, ret);
    Napi::Object robj = Napi::Object::New(env);
    robj.Set("code", ret);
    robj.Set("content", _content);
    return robj;
  }
  catch (int err)
  {
    Napi::Object robj = Napi::Object::New(env);
    robj.Set("code", Napi::Number::New(env, YXU_OPERATION_FAILED));
    robj.Set("content", "");
    return robj;
  }
}

Napi::Value vilssh::writefile(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 2)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString() || !info[1].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _path = info[0].As<Napi::String>().Utf8Value();
    std::string _content = info[1].As<Napi::String>().Utf8Value();
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    int ret = defsftp->_writefile(_path, _content);
    return Napi::Number::New(env, ret);
  }
  catch (int err)
  {
    return Napi::Number::New(env, YXU_OPERATION_FAILED);
  }
}


/**
 * @brief 检测目录是否有写权限
 *
 * @param info
 * @return Napi::Value
 */
Napi::Value vilssh::WritePermissionDir(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  try
  {
    if (info.Length() < 1)
      throw YXU_PARAMETER_ERROR;
    if (!info[0].IsString())
      throw YXU_PARAMETER_ERROR;
    //--
    std::string _path = info[0].As<Napi::String>().Utf8Value();
    std::string _uuidstr = vdoc::uuid4();
    std::string _file = _path + _uuidstr;
    std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
    defsftp->generate();
    sftp_file file = sftp_open(defsftp->sftp, _file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (file == NULL)
    {
      int err = defsftp->geterror();
      vdlog::getInstance()->error("(sftp_open)Error: Code: %d", err);
      throw err;
    }
    else
    {
      sftp_unlink(defsftp->sftp, _file.c_str());
      return Napi::Number::New(env, YXU_SUCCESS);
    }
  }
  catch (int err)
  {
    return Napi::Number::New(env, err);
  }
}

Napi::Value vilssh::find_service_file(const Napi::CallbackInfo &info)
{
  return Napi::Value();
}




/**
 * @brief 获取文件的类型、以及读写权限
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::get_filetype_and_permission(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::shared_ptr<Napi::Promise::Deferred> promise = std::make_shared<Napi::Promise::Deferred>(env);
  try
  {
    if (info.Length() < 1){
      throw YXU_PARAMETER_ERROR;
    }
    if (!info[0].IsString()){
      throw YXU_PARAMETER_ERROR;
    }
    struct tmpstr1 {
      std::string path;
      ysftp_file_attr1 f_attr;
      int code;
    };

    ThreadFuns *stru = new ThreadFuns();
    stru->uuid = vdoc::uuid4();
    stru->ptr = new tmpstr1;
    ((struct tmpstr1 *)stru->ptr)->path = info[0].As<Napi::String>().Utf8Value();
    Napi::Function callback; // = info[1].As<Napi::Function>();
    stru->thread_function_1 = Napi::ThreadSafeFunction::New(env, callback, "reauth", 0, 1);
    stru->promise = promise;
    thFuncs[stru->uuid] = stru;

    thFuncs[stru->uuid]->th = new std::thread([&](ThreadFuns *stru)
    {
      auto JavaScriptCallbackFun = [&](Napi::Env env, Napi::Function jsCallback, ThreadFuns *stru)
      {
        if (stru->state == YXU_SUCCESS)
        {
          struct tmpstr1 *str1 = (struct tmpstr1 *)stru->ptr;
          int code = str1->code;
          Napi::Object robj = Napi::Object::New(env);
          robj.Set("code", code);
          robj.Set("linkfilepath", str1->f_attr.linkfilepath);
          robj.Set("filetype", str1->f_attr.filetype);
          robj.Set("islink", str1->f_attr.islink);
          robj.Set("isRead", str1->f_attr.isRead);
          robj.Set("isWrite", str1->f_attr.isWrite);
          robj.Set("filepath", str1->f_attr.filepath);
          stru->promise->Resolve(robj);
        }
        else
        {
          stru->promise->Reject(Napi::Number::New(env, stru->state));
        }

        std::string uuid = stru->uuid;
        delete stru;
        thFuncs.erase(uuid);
      };

      std::shared_ptr<ysftp> defsftp = std::make_shared<ysftp>(this->defconn);
      stru->state = defsftp->generate();
      if(stru->state==YXU_SUCCESS){
        sftp_attributes attr=NULL;
        struct tmpstr1 *str1 = (struct tmpstr1 *)stru->ptr;
        int code = defsftp->get_filetype_and_permission(str1->path, this->defconn->uid->c_str(), attr, str1->f_attr);
        str1->code = code;
      }
      stru->thread_function_1.BlockingCall(stru, JavaScriptCallbackFun);

    }, stru);
  }
  catch (int err)
  {
    promise->Reject(Napi::Number::New(env, err));
  }
  return promise->Promise();
}


/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::getutc(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::string timestr;
  uint64_t timestamp;
  common::getutc(timestamp, timestr);
  Napi::Object outobj = Napi::Object::New(env);
  outobj.Set("timestamp", Napi::Number::New(env, timestamp));
  outobj.Set("timestr", Napi::String::New(env, timestr));
  return outobj;
}

/**
 * @brief 
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value vilssh::getutc8(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::string timestr;
  uint64_t timestamp;
  common::getutc8(timestamp, timestr);
  Napi::Object outobj = Napi::Object::New(env);
  outobj.Set("timestamp", Napi::Number::New(env, timestamp));
  outobj.Set("timestr", Napi::String::New(env, timestr));
  return outobj;
}

