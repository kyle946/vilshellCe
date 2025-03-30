#include "curlrequest.h"

curlrequest::curlrequest()
{
}

///======================================================================
///
///

void curlrequest::PostJson(std::string urlstr, std::string jsonString, std::vector<std::string> &headers, int &response_status, std::string &response_body)
{
  //    std::string urlstr="https://vildoc.com/vildoc/";

  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, urlstr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());

#ifdef SKIP_PEER_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    /* set custom headers */
    struct curl_slist *slist1 = NULL;
    slist1 = curl_slist_append(slist1, "Content-Type: application/json");
    slist1 = curl_slist_append(slist1, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, https_post_json_write_function);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&https_post_json_write_data);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      //            vdlog::getInstance()->info("http code: %d", response_code);
    }

    //
    response_status = response_code;
    response_body = https_post_json_write_data.response;
    //        vdlog::getInstance()->info("http body: %s", https_post_json_write_data.response);
    free(https_post_json_write_data.response);
  }
  curl_global_cleanup();
}

size_t curlrequest::https_post_json_write_function(void *data, size_t size, size_t nmemb, void *clientp)
{

  size_t realsize = size * nmemb;
  struct curlrequestmemory *mem = (struct curlrequestmemory *)clientp;
  char *ptr = (char *)realloc(mem->response, mem->size + realsize + 1);
  if (!ptr)
    return 0; /* out of memory! */
  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
  return realsize;
}

///======================================================================
///
///

void curlrequest::https_get(std::string urlstr, int &response_status, std::string &response_body)
{
  CURL *curl;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, urlstr.c_str());

#ifdef SKIP_PEER_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, https_get_write_function);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&https_get_write_data);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }
    //
    response_status = response_code;
    response_body = https_get_write_data.response;
    free(https_get_write_data.response);
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
}

size_t curlrequest::https_get_write_function(void *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = size * nmemb;
  struct curlrequestmemory *mem = (struct curlrequestmemory *)clientp;
  char *ptr = (char *)realloc(mem->response, mem->size + realsize + 1);
  if (!ptr)
    return 0; /* out of memory! */
  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
  return realsize;
}

///======================================================================
///
///

int curlrequest::https_get_download(std::string urlstr, std::string filepath, int *cancel, const std::function<void(double, double)> &newProcessFunction)
{
  int code = 0;
  processFunction = newProcessFunction;
  https_get_download_cancel = cancel;

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

      https_get_download_fp = fopen(filepath.data(), "wb");
      if (https_get_download_fp == NULL)
      {
        throw -1;
      }
      this->buffer = new std::string;

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

  if (https_get_download_fp != nullptr)
  {
    fwrite(this->buffer->data(), this->buffer->size(), 1, https_get_download_fp);
    this->buffer->clear();
    fclose(https_get_download_fp);
  }

  return code;
}

size_t curlrequest::https_get_download_write_function(void *data, size_t size, size_t nmemb, void *cr)
{
  curlrequest *obj = (curlrequest *)cr;
  if ((*obj->https_get_download_cancel) == 1)
  { // 中途取消下载
    return 0;
  }
  else
  {
    size_t realsize = size * nmemb;
    obj->buffer->append((char *)data, realsize);
    const int bufsize = 1024 * 1024 * 2;
    if (obj->buffer->size() >= bufsize)
    {
      fwrite(obj->buffer->data(), obj->buffer->size(), 1, (FILE *)obj->https_get_download_fp);
      obj->buffer->clear();
    }
    return realsize;
  }
}

size_t curlrequest::https_get_download_progress_function(void *cr, double dltotal, double dlnow, double ultotal, double ulnow)
{
  curlrequest *obj = (curlrequest *)cr;
  // printf("已下载：%.2f MB/ %.2f MB\n", dlnow / (1024 * 1024), dltotal / (1024 * 1024));
  if (obj->processFunction != nullptr)
    obj->processFunction(dltotal, dlnow);
  return 0;
}
