#ifndef CURLREQUEST_H
#define CURLREQUEST_H


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



struct curlrequestmemory {
    char *response;
    size_t size;
};

class curlrequest
{
public:
    curlrequest();

    /**
     * POST请求，发送JSON数据
     *
     * @brief PostJson
     * @param urlstr
     * @param jsonString
     * @param headers
     * @param response_status
     * @param response_body
     */
    void PostJson(std::string urlstr, std::string jsonString, std::vector<std::string> &headers, int &response_status, std::string &response_body);
    struct curlrequestmemory https_post_json_write_data = {};
    static size_t https_post_json_write_function(void *data, size_t size, size_t nmemb, void *clientp);


    /**
     * GET请求
     * @brief https_get
     */
    void https_get(std::string urlstr, int &response_status, std::string &response_body);
    struct curlrequestmemory https_get_write_data = {};
    static size_t https_get_write_function(void *data, size_t size, size_t nmemb, void *clientp);


    /**
     * 下载文件
     * @brief https_get_download
     */
    int https_get_download(std::string urlstr, std::string filepath, int *cancel, const std::function<void (double, double)> &newProcessFunction);
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

#endif // CURLREQUEST_H
