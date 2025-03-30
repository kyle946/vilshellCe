#include "common.h"

std::string common::replace(std::string resource_str, std::string sub_str, std::string new_str)
{
  std::string result_str = resource_str;
  for (std::string::size_type pos = 0; pos != std::string::npos; pos += new_str.length())
  {
    pos = result_str.find(sub_str, pos);
    if (pos != std::string::npos)
    {
      result_str.replace(pos, sub_str.length(), new_str);
    }
    else
    {
      break;
    }
  }
  return result_str;
}

std::string common::toAscii(std::string txt)
{
  std::string outtxt;
  if (txt.empty())
  {
    return outtxt;
  }
  if (txt.length() == 0)
  {
    return outtxt;
  }
  int strsize = txt.length();
  char *strs = new char[strsize + 1];
  memcpy(strs, txt.c_str(), strsize);
  for (int i = 0; i < strsize; i++)
  {
    int ch = strs[i];
    if (i > 0)
    {
      outtxt.append(",");
    }
    outtxt.append(std::to_string(ch));
  }
  delete[] strs;
  return outtxt;
}

std::vector<std::string> common::strSplit(std::string str, const char *split)
{
  std::vector<std::string> res;
  std::string::size_type pos1 = 0;
  while (pos1 != std::string::npos)
  {
    // npos is not 0, nor -1, it is the maximum value of size_type.
    pos1 = str.find(split);
    if (pos1 == std::string::npos)
    {
      res.push_back(str);
      break;
    }
    else
    {
      res.push_back(str.substr(0, pos1));
      str = str.substr(pos1 + strlen(split));
    }
  }
  return res;
}

std::string common::sprintf(const char *format, ...)
{
  va_list args, args1;
  va_start(args, format);
  va_copy(args1, args);
#ifdef _WIN32
  // int len = _vscprintf(format, args) + 1;
  std::string txt(1 + _vscprintf(format, args1), 0);
#else
  // int len = vsnprintf(NULL, 0, format, args) +1;
  std::string txt(1 + vsnprintf(nullptr, 0, format, args1), 0);
#endif
  va_end(args1);
  vsnprintf(&txt[0], txt.size(), format, args);
  va_end(args);
  return txt;
}

void common::getutc(uint64_t &sec1, std::string &timestr)
{
  time_t now = time(0);
  char *data = ctime(&now);
  sec1 = now;
  struct tm *local = gmtime(&now);

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

void common::getutc8(uint64_t &sec1, std::string &timestr)
{
  time_t now = time(0);
  now += 8 * 3600; // 增加8小时的时差
  sec1 = now;
  char *data = ctime(&now);
  struct tm *local = gmtime(&now);

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



  int common::readfile(std::string filepath, std::string &content)
  {
    FILE *f = _fopen(filepath, "rb+");
    if (f == nullptr)
    {
      return -1;
    }
    //---- 计算文件大小
    fseek(f, 0, SEEK_END); // 移到文件结尾
    uint64_t f_size = ftell(f);
    fseek(f, 0, SEEK_SET); // 移到指定处
    // 文件是否太大
    const uint64_t max_size = 1024 * 1024 * 30; // 30M
    if (f_size > max_size)
    {
      return -2; //---- 文件太大
    }
    char *buffer = new char[f_size];
    memset(buffer, 0, f_size);
    size_t readbyte = fread(buffer, 1, f_size, f);
    fclose(f);
    content.append(buffer, readbyte);
    delete[] buffer;
    return 1;
  }

  int common::writefile(std::string path, std::string &body)
  {
    try
    {
      FILE *f = _fopen(path, "wb+");
      if (f == nullptr)
      {
        throw -1;
      }
      size_t writebyte = fwrite(body.data(), 1, body.size(), f);
      fclose(f);
      return 1;
    }
    catch (int code)
    {
      return code;
    }
  }

// 去除首尾空白字符的函数
std::string common::trim(const std::string &str)
{
  std::string result = str;
  // 去除前导空白字符
  result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char c)
                                            { return !std::isspace(c); }));
  // 去除尾部空白字符
  result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char c)
                            { return !std::isspace(c); })
                   .base(),
               result.end());
  return result;
}

Napi::Object common::json2objectR(Napi::Env env, JsonObj &inJson)
{
  if (inJson.IsArray())
  {
    int _size = inJson.GetArraySize();
    Napi::Array outArr = Napi::Array::New(env, _size);
    for (int i = 0; i < _size; i++)
    {
      if (inJson[i].IsObject())
      {
        Napi::Object obj = json2objectR(env, inJson[i]);
        outArr[static_cast<uint32_t>(i)] = obj;
      }
      else if (inJson[i].IsArray())
      {
        Napi::Object obj = json2objectR(env, inJson[i]);
        outArr[static_cast<uint32_t>(i)] = obj;
      }
      else
      {
        int _val;
        if (inJson.Get(i, _val))
        {
          outArr[static_cast<uint32_t>(i)] = _val;
        }
        else
        {
          outArr[static_cast<uint32_t>(i)] = inJson(i);
        }
      }
    }
    return outArr;
  }
  else
  {
    Napi::Object outObj = Napi::Object::New(env);
    std::string _key;
    while (inJson.GetKey(_key))
    {
      if (inJson[_key].IsObject())
      {
        Napi::Object obj = json2objectR(env, inJson[_key]);
        outObj.Set(_key, obj);
      }
      else if (inJson[_key].IsArray())
      {
        Napi::Object obj = json2objectR(env, inJson[_key]);
        outObj.Set(_key, obj);
      }
      else
      {
        int _val;
        if (inJson.Get(_key, _val))
        {
          outObj.Set(_key, _val);
        }
        else
        {
          outObj.Set(_key, inJson(_key));
        }
      }
    }
    return outObj;
  }
}

Napi::Array common::jsonobj_to_napi_array(Napi::Env env, JsonObj &list1)
{
  Napi::Array list1obj = Napi::Array::New(env);
  for (int i = 0; i < list1.GetArraySize(); i++)
  {
    JsonObj &item = list1[i];
    // vdlog::getInstance()->info("key: %s ", item.ToString().c_str());
    Napi::Object iobj = Napi::Object::New(env);
    std::string _key;
    while (item.GetKey(_key))
    {
      int _val;
      if (item.Get(_key, _val))
      {
        iobj.Set(_key, Napi::Number::New(env, _val));
      }
      else
      {
        iobj.Set(_key, Napi::String::New(env, item(_key)));
      }
    }
    list1obj.Set(i, iobj);
  }
  return list1obj;
}

std::vector<std::string> common::napi_array_to_vectorstr(Napi::Env env, Napi::Object inObject)
{
  int len = 0;
  std::vector<std::string> list1;
  if (inObject.IsArray())
  {
    Napi::Array arr1 = inObject.As<Napi::Array>();
    len = arr1.Length();
    for (int i = 0; i < len; i++)
    {
      Napi::Value val = arr1[static_cast<uint32_t>(i)];
      if (val.IsString())
      {
        std::string ff = val.As<Napi::String>().Utf8Value();
        list1.push_back(ff);
      }
    }
  }
  return list1;
}

Napi::Object common::json2object(Napi::Env env, JsonObj &inJson)
{
  if (inJson.IsEmpty())
  {
    return Napi::Object();
  }
  if (!inJson.IsObject())
  {
    return Napi::Object();
  }
  Napi::Object outObject = json2objectR(env, inJson);
  return outObject;
}

int common::object2jsonR(JsonObj &json, Napi::Object &inObject)
{
  Napi::Array names = inObject.GetPropertyNames();
  int len = 0;
  if (inObject.IsArray())
  {
    len = inObject.As<Napi::Array>().Length();
  }
  else
  {
    if (names.Length() == 0)
    {
      return 0;
    }
    len = names.Length();
  }
  for (int i = 0; i < len; i++)
  {
    if (inObject.IsArray())
    {
      Napi::Array arr1 = inObject.As<Napi::Array>();
      Napi::Value val = arr1[static_cast<uint32_t>(i)];
      if (val.IsNumber())
      {
        json.Add(val.As<Napi::Number>().Int64Value());
      }
      else if (val.IsString())
      {
        json.Add(val.As<Napi::String>().Utf8Value());
      }
      else if (val.IsBoolean())
      {
        json.Add(val.As<Napi::Boolean>().Value());
      }
      else if (val.IsBigInt())
      {
        json.Add(val.As<Napi::Number>().Int64Value());
      }
      else if (val.IsArray())
      {
        JsonObj arr;
        object2jsonR(arr, (Napi::Object &)val);
        json.Add(arr);
      }
      else if (val.IsFunction())
      {
        json.Add("Function");
      }
      else if (val.IsDataView())
      {
        json.Add("DataView");
      }
      else if (val.IsBuffer())
      {
        json.Add("Buffer");
      }
      else if (val.IsArrayBuffer())
      {
        json.Add("ArrayBuffer");
      }
      else if (val.IsObject())
      {
        JsonObj arr;
        object2jsonR(arr, (Napi::Object &)val);
        json.Add(arr);
      }
      else if (val.IsNull())
      {
        // json.Add(NULL);
      }
    }
    else
    {
      Napi::Value kval = names[static_cast<uint32_t>(i)];
      // Napi::Value kval = names[i];
      std::string key = kval.As<Napi::String>().Utf8Value();
      //----
      Napi::Value val = inObject.Get(key);
      if (val.IsNumber())
      {
        json.Add(key, val.As<Napi::Number>().Int64Value());
      }
      else if (val.IsString())
      {
        json.Add(key, val.As<Napi::String>().Utf8Value());
      }
      else if (val.IsBoolean())
      {
        json.Add(key, val.As<Napi::Boolean>().Value());
      }
      else if (val.IsBigInt())
      {
        json.Add(key, val.As<Napi::Number>().Int64Value());
      }
      else if (val.IsArray())
      {
        JsonObj arr;
        object2jsonR(arr, (Napi::Object &)val);
        json.Add(key, arr);
      }
      else if (val.IsFunction())
      {
        json.Add(key, "Function");
      }
      else if (val.IsDataView())
      {
        json.Add(key, "DataView");
      }
      else if (val.IsBuffer())
      {
        json.Add(key, "Buffer");
      }
      else if (val.IsArrayBuffer())
      {
        json.Add(key, "ArrayBuffer");
      }
      else if (val.IsObject())
      {
        JsonObj arr;
        object2jsonR(arr, (Napi::Object &)val);
        json.Add(key, arr);
      }
      else if (val.IsNull())
      {
        json.Add(key, "");
      }
    }
  }
  return 0;
}

JsonObj common::object2json(Napi::Env env, Napi::Object inObject)
{
  if (inObject.IsNull() || inObject.IsUndefined())
  {
    return JsonObj();
  }
  else
  {
    JsonObj outJson;
    object2jsonR(outJson, inObject);
    return outJson;
  }
}

#ifdef _WIN32
std::wstring common::string_to_wstring(std::string input)
{
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(input);
}
std::string common::wstring_to_string(const std::wstring source)
{
  return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(source);
}
#else
std::wstring common::string_to_wstring(std::string &str)
{
  return str;
}
std::string common::wstring_to_string(const std::wstring wstr)
{
  iconv_t conv = iconv_open("UTF-8", "WCHAR_T");
  if (conv == (iconv_t)-1)
  {
    perror("iconv_open");
    return std::string();
  }

  size_t inbytes = wstr.size() * sizeof(wchar_t);
  size_t outbytes = inbytes * 2;
  char *inbuf = reinterpret_cast<char *>(const_cast<wchar_t *>(wstr.data()));
  std::string result(outbytes, '\0');
  char *outbuf = &result[0];

  size_t ret = iconv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
  if (ret == (size_t)-1)
  {
    perror("iconv");
  }
  result.resize(result.size() - outbytes); // Adjust the result size

  iconv_close(conv);
  return result;
}
#endif

FILE *common::_fopen(std::string name, std::string type)
{
#ifdef _WIN32
  return _wfopen(string_to_wstring(std::string(name)).data(), string_to_wstring(std::string(type)).data());
#else
  return fopen(name.c_str(), type.c_str());
#endif
}

void common::_rmfile(std::string name)
{
#ifdef _WIN32
  _wremove(string_to_wstring(std::string(name)).data());
#else
  remove(name.c_str());
#endif
}

void common::_mkdir(std::string name)
{
#ifdef _WIN32
  _wmkdir(string_to_wstring(std::string(name)).data());
#else
  mkdir(name.c_str(), 0777);
#endif
}


fstat1 common::stat1(std::string path)
{
  std::string filename;
  fstat1 buf;
#ifdef _WIN32
  struct _stat buffer;
  _wstat(string_to_wstring(std::string(path)).data(), &buffer);

  // 判断文件类型
  if ((buffer.st_mode & _S_IFDIR) == _S_IFDIR)
    strcpy(buf.type, "004");
  else if ((buffer.st_mode & _S_IFREG) == _S_IFREG)
    strcpy(buf.type, "010");
  else if ((buffer.st_mode & _S_IFIFO) == _S_IFIFO)
    strcpy(buf.type, "001");
  else if ((buffer.st_mode & _S_IFCHR) == _S_IFCHR)
    strcpy(buf.type, "002");
  else
    strcpy(buf.type, "017");

  // 判断权限
  if ((buffer.st_mode & _S_IREAD) == _S_IREAD)
    buf.read = 1;
  if ((buffer.st_mode & _S_IWRITE) == _S_IWRITE)
    buf.write = 1;
  if ((buffer.st_mode & _S_IEXEC) == _S_IEXEC)
    buf.exec = 1;

  // 取时间
  buf.atime = buffer.st_atime;
  buf.mtime = buffer.st_mtime;
  buf.ctime = buffer.st_ctime;
  buf.size = buffer.st_size;

  // 取文件名
  if (path.find("\\") != std::string::npos)
  {
    std::string::size_type iPos = path.find_last_of('\\') + 1;
    filename = path.substr(iPos, path.length() - iPos);
  }
  else
  {
    filename = path;
  }
  //--
  buf.name = filename;
  buf.gid = buffer.st_gid;
  buf.uid = buffer.st_uid;
#else
  struct stat buffer;
  stat(path.c_str(), &buffer);

  // 判断文件类型
  if ((buffer.st_mode & S_IFDIR) == S_IFDIR)
    strcpy(buf.type, "004");
  else if ((buffer.st_mode & S_IFREG) == S_IFREG)
    strcpy(buf.type, "010");
  else if ((buffer.st_mode & S_IFIFO) == S_IFIFO)
    strcpy(buf.type, "001");
  else if ((buffer.st_mode & S_IFCHR) == S_IFCHR)
    strcpy(buf.type, "002");
  else
    strcpy(buf.type, "017");

  // 判断权限
  if ((buffer.st_mode & S_IREAD) == S_IREAD)
    buf.read = 1;
  if ((buffer.st_mode & S_IWRITE) == S_IWRITE)
    buf.write = 1;
  if ((buffer.st_mode & S_IEXEC) == S_IEXEC)
    buf.exec = 1;

  // 取时间
  buf.atime = buffer.st_atime;
  buf.mtime = buffer.st_mtime;
  buf.ctime = buffer.st_ctime;
  buf.size = buffer.st_size;

  // 取文件名
  if (path.find("/") != std::string::npos)
  {
    std::string::size_type iPos = path.find_last_of('/') + 1;
    filename = path.substr(iPos, path.length() - iPos);
  }
  else
  {
    filename = path;
  }
  //--
  buf.name = filename;
  buf.gid = buffer.st_gid;
  buf.uid = buffer.st_uid;
#endif
  return buf;
}