#ifndef COMMON_H
#define COMMON_H

#include <napi.h>

#include <string>
#include <vector>
#include <regex>
#include <map>
#include <ctime>
#include <time.h>

#include "./plugins/CJsonObject.h"


#ifdef _WIN32
#include "./plugins/dirent2.h"
#include <codecvt>
#include <locale>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define DIRSEPARATOR "\\"
#else
#include <iconv.h> 
#include <cerrno> 
#include <cstring>
#include <sys/stat.h>
#define DIRSEPARATOR "/"
#endif


#include "./ystruct.h"
#include <vdlog.h>




class common
{
public:
  static std::string replace(std::string resource_str, std::string sub_str, std::string new_str);
  static std::string toAscii(std::string txt);
  static std::vector<std::string> strSplit(std::string str, const char *split);
  static std::string sprintf(const char *format, ...);
  static std::string common::trim(const std::string &str);
  static Napi::Object json2object(Napi::Env env, JsonObj &inJson);
  static int object2jsonR(JsonObj &json, Napi::Object &inObject);
  static JsonObj object2json(Napi::Env env, Napi::Object inObject);
  static Napi::Object json2objectR(Napi::Env env, JsonObj &inJson);
  static Napi::Array jsonobj_to_napi_array(Napi::Env env, JsonObj &list1);
  static std::vector<std::string> napi_array_to_vectorstr(Napi::Env env, Napi::Object inObject);
  static void getutc(uint64_t &sec1, std::string &timestr);
  static void getutc8(uint64_t &sec1, std::string &timestr);

  static int readfile(std::string filepath, std::string &content);
  static int writefile(std::string filepath, std::string &content);

#ifdef _WIN32
    static std::wstring string_to_wstring(std::string input);
    static std::string wstring_to_string(const std::wstring source);
#else
    static std::wstring string_to_wstring(std::string& str);
    static std::string wstring_to_string(const std::wstring wstr);
#endif

    static FILE *_fopen(std::string name, std::string type);
    static void _rmfile(std::string name);
    static void _mkdir(std::string name);
    static struct fstat1 stat1(std::string path);
};

#endif // COMMON_H
