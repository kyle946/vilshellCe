#ifndef curlget_h
#define curlget_h



#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <functional>

struct curlgetstr
{
  char *response;
  size_t size;
};

class curlget
{
public:
  void setcookiefile(std::string file);
  void setheaders(std::vector<std::string> headers);
  void setcookies(std::string cookies);
  std::vector<std::string> getcookies();
  int get(std::string urlstr, int &response_status, std::string &response_body);

private:
  struct curlgetstr _write_data = {};
  static size_t _write(void *data, size_t size, size_t nmemb, void *dd);
  //--
  long connect_timeout = 3;
  long response_code;
  std::vector<std::string> response_cookies;
  //--
  std::string cookiefile;
  std::string cookies;
  std::vector<std::string> headers;
};

inline void curlget::setcookiefile(std::string file)
{
  this->cookiefile=file;
}

inline void curlget::setheaders(std::vector<std::string> headers)
{
  this->headers=headers;
}

inline void curlget::setcookies(std::string cookies)
{
  this->cookies=cookies;
}

inline std::vector<std::string> curlget::getcookies()
{
  return response_cookies;
}

/**
 * @brief
 *
 * @param urlstr
 * @param jsonString
 * @param cookies   example:  "tool=curl; fun=yes;"
 * @param headers
 * @param response_status
 * @param response_body
 * @return int
 */
inline int curlget::get( std::string urlstr, int &response_status, std::string &response_body)
{
  try
  {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
      curl_easy_setopt(curl, CURLOPT_URL, urlstr.c_str());
      /* cookie */
      curl_easy_setopt(curl, CURLOPT_COOKIE, this->cookies.c_str());
      if(!this->cookiefile.empty()){
        #ifdef _WIN32
        #else
        #endif
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, this->cookiefile.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, this->cookiefile.c_str());
      }
#ifdef SKIP_PEER_VERIFICATION
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
      // 设置head
      curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
      curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
      
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&_write_data);
      // 开始请求
      res = curl_easy_perform(curl);
      if (res != CURLE_OK)
      {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        response_body = curl_easy_strerror(res);
      }
      else
      {
        struct curl_slist *cookies;
        curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
        if (cookies)
        {
          struct curl_slist *each = cookies;
          while (each)
          {
            response_cookies.push_back(each->data);
            // printf("%s\n", each->data);
            each = each->next;
          }
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        // 返回数据
        response_status = response_code;
        response_body = _write_data.response;
        free(_write_data.response);
        return response_code;
      }
    }
    curl_global_cleanup();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
  return 0;
}

inline size_t curlget::_write(void *data, size_t size, size_t nmemb, void *dd)
{
  size_t realsize = size * nmemb;
  struct curlgetstr *mem = (struct curlgetstr *)dd;
  char *ptr = (char *)realloc(mem->response, mem->size + realsize + 1);
  if (!ptr)
    return 0; /* out of memory! */
  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
  return realsize;
}


#endif // curlget_h