#include <napi.h>
#include <node_api.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "./apphandle.h"
#include "./vilssh.h"

#include <vdlog.h>



Napi::Value test1 (const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  return env.Null();
}


// 初始化
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set("test1", Napi::Function::New(env, test1));
  apphandle::bindFunc(env, exports);
  vilssh::bindFunc(env, exports);
  return exports;
}

NODE_API_MODULE(vildll1, Init)