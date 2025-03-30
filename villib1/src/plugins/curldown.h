#ifndef curldown_h
#define curldown_h


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


struct curldownmemory {
    char *response;
    size_t size;
};

class curldown
{
public:
    curldown();

    /**
     * 下载文件
     * @brief https_get_download
     */
    int down(std::string urlstr, std::string *data, const std::function<void (double, double)> &newProcessFunction);
    FILE *https_get_download_fp = nullptr;
    std::string *buffer=nullptr;
    int *https_get_download_cancel;
    static size_t https_get_download_write_function(void *data, size_t size, size_t nmemb, void *cr);
    char *https_get_download_progress_data;

    /**
     * @brief https_get_download_progress_function
     * @param clientp
     * @param dltotal       download total
     * @param dlnow         download now
     * @param ultotal       upload total
     * @param ulnow         upload now
     * @return
     */
    static size_t https_get_download_progress_function(void *cr, double dltotal, double dlnow, double ultotal, double ulnow);

public:
    std::function <void(double dltotal, double dlnow)> processFunction;
    long connect_timeout=3;
    long response_code;
};


inline curldown::curldown(){}


inline int curldown::down(std::string urlstr, std::string *data, const std::function<void(double, double)> &newProcessFunction)
{
  int code = 0;
  processFunction = newProcessFunction;
  try
  {

    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {

      // 设置curl的请求头
      struct curl_slist *header_list = NULL;
      header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36");
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

      // 不接收响应头数据0代表不接收 1代表接收
      curl_easy_setopt(curl, CURLOPT_HEADER, 0);

      // https_get_download_fp = fopen(filepath.data(), "wb");
      // if (https_get_download_fp == NULL)
      // {
      //   throw -1;
      // }
      this->buffer = data;

      curl_easy_setopt(curl, CURLOPT_URL, urlstr.c_str());
      curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);

      // 设置ssl验证
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, https_get_download_write_function);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, https_get_download_progress_function);
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);

      /* Perform the request, res will get the return code */
      res = curl_easy_perform(curl);

      /* Check for errors */
      if (res != CURLE_OK)
      {
        throw -2;
      }
      else
      {
        code = 1;
      }
      /* always cleanup */
      curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
  }
  catch (int err)
  {
    code = err;
  }

  // if (https_get_download_fp != nullptr)
  // {
  //   fwrite(this->buffer->data(), this->buffer->size(), 1, https_get_download_fp);
  //   this->buffer->clear();
  //   fclose(https_get_download_fp);
  // }

  return code;
}

inline size_t curldown::https_get_download_write_function(void *data, size_t size, size_t nmemb, void *cr)
{
  curldown *obj = (curldown *)cr;
  size_t realsize = size * nmemb;
  obj->buffer->append((char *)data, realsize);
  return realsize;
}

inline size_t curldown::https_get_download_progress_function(void *cr, double dltotal, double dlnow, double ultotal, double ulnow)
{
  curldown *obj = (curldown *)cr;
  // printf("已下载：%.2f MB/ %.2f MB\n", dlnow / (1024 * 1024), dltotal / (1024 * 1024));
  if (obj->processFunction != nullptr)
    obj->processFunction(dltotal, dlnow);
  return 0;
}




#endif