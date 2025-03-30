/*******************************************************************************
 * Project:  neb
 * @file     JsonObj.hpp
 * @brief    Json
 * @author   bwarliao
 * @date:    2014-7-16
 * @note
 * Modify history:
 ******************************************************************************/

#ifndef JsonObj_HPP_
#define JsonObj_HPP_

#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#ifdef _WIN32
    #define _UNISTD_H
    #include <io.h>
    #include <process.h>
    #include <malloc.h>
#else
    #include <unistd.h>
    #include <sys/malloc.h>
#endif


#include <limits.h>
#include <math.h>
#include <float.h>
#include <string>
#include <map>
#include <list>
#ifdef __cplusplus
extern "C" {
#endif
#include "cJSON.h"
#ifdef __cplusplus
}
#endif


class JsonObj
{
public:     // method of ordinary json object or json array
    JsonObj();
    JsonObj(const std::string& strJson);
    JsonObj(const JsonObj* pJsonObject);
    JsonObj(const JsonObj& oJsonObject);
    virtual ~JsonObj();

    JsonObj& operator=(const JsonObj& oJsonObject);
    bool operator==(const JsonObj& oJsonObject) const;
    bool Parse(const std::string& strJson);
    void Clear();
    bool IsEmpty() const;
    bool IsArray() const;
    bool IsObject() const;
    std::string ToString() const;
    std::string ToFormattedString() const;
    const std::string& GetErrMsg() const
    {
        return(m_strErrMsg);
    }

public:     // method of ordinary json object
    bool AddEmptySubObject(const std::string& strKey);
    bool AddEmptySubArray(const std::string& strKey);
    bool GetKey(std::string& strKey);
    JsonObj& operator[](const std::string& strKey);
    std::string operator()(const std::string& strKey) const;
    bool Get(const std::string& strKey, JsonObj& oJsonObject) const;
    bool Get(const std::string& strKey, std::string& strValue) const;
    bool Get(const std::string& strKey, int32& iValue) const;
    bool Get(const std::string& strKey, uint32& uiValue) const;
    bool Get(const std::string& strKey, int64& llValue) const;
    bool Get(const std::string& strKey, uint64& ullValue) const;
    bool Get(const std::string& strKey, bool& bValue) const;
    bool Get(const std::string& strKey, float& fValue) const;
    bool Get(const std::string& strKey, double& dValue) const;
    bool Add(const std::string& strKey, const JsonObj& oJsonObject);
    bool Add(const std::string& strKey, const std::string& strValue);
    bool Add(const std::string& strKey, int32 iValue);
    bool Add(const std::string& strKey, uint32 uiValue);
    bool Add(const std::string& strKey, int64 llValue);
    bool Add(const std::string& strKey, uint64 ullValue);
    bool Add(const std::string& strKey, bool bValue, bool bValueAgain);
    bool Add(const std::string& strKey, float fValue);
    bool Add(const std::string& strKey, double dValue);
    bool Delete(const std::string& strKey);
    bool Replace(const std::string& strKey, const JsonObj& oJsonObject);
    bool Replace(const std::string& strKey, const std::string& strValue);
    bool Replace(const std::string& strKey, int32 iValue);
    bool Replace(const std::string& strKey, uint32 uiValue);
    bool Replace(const std::string& strKey, int64 llValue);
    bool Replace(const std::string& strKey, uint64 ullValue);
    bool Replace(const std::string& strKey, bool bValue, bool bValueAgain);
    bool Replace(const std::string& strKey, float fValue);
    bool Replace(const std::string& strKey, double dValue);

public:     // method of json array
    int GetArraySize();
    JsonObj& operator[](unsigned int uiWhich);
    std::string operator()(unsigned int uiWhich) const;
    bool Get(int iWhich, JsonObj& oJsonObject) const;
    bool Get(int iWhich, std::string& strValue) const;
    bool Get(int iWhich, int32& iValue) const;
    bool Get(int iWhich, uint32& uiValue) const;
    bool Get(int iWhich, int64& llValue) const;
    bool Get(int iWhich, uint64& ullValue) const;
    bool Get(int iWhich, bool& bValue) const;
    bool Get(int iWhich, float& fValue) const;
    bool Get(int iWhich, double& dValue) const;
    bool Add(const JsonObj& oJsonObject);
    bool Add(const std::string& strValue);
    bool Add(int32 iValue);
    bool Add(uint32 uiValue);
    bool Add(int64 llValue);
    bool Add(uint64 ullValue);
    bool Add(int iAnywhere, bool bValue);
    bool Add(float fValue);
    bool Add(double dValue);
    bool AddAsFirst(const JsonObj& oJsonObject);
    bool AddAsFirst(const std::string& strValue);
    bool AddAsFirst(int32 iValue);
    bool AddAsFirst(uint32 uiValue);
    bool AddAsFirst(int64 llValue);
    bool AddAsFirst(uint64 ullValue);
    bool AddAsFirst(int iAnywhere, bool bValue);
    bool AddAsFirst(float fValue);
    bool AddAsFirst(double dValue);
    bool Delete(int iWhich);
    bool Replace(int iWhich, const JsonObj& oJsonObject);
    bool Replace(int iWhich, const std::string& strValue);
    bool Replace(int iWhich, int32 iValue);
    bool Replace(int iWhich, uint32 uiValue);
    bool Replace(int iWhich, int64 llValue);
    bool Replace(int iWhich, uint64 ullValue);
    bool Replace(int iWhich, bool bValue, bool bValueAgain);
    bool Replace(int iWhich, float fValue);
    bool Replace(int iWhich, double dValue);

private:
    JsonObj(cJSON* pJsonData);

private:
    cJSON* m_pJsonData;
    cJSON* m_pExternJsonDataRef;
    std::string m_strErrMsg;
    std::map<unsigned int, JsonObj*> m_mapJsonArrayRef;
    std::map<std::string, JsonObj*> m_mapJsonObjectRef;
    std::list<std::string> m_listKeys;
    std::list<std::string>::const_iterator m_itKey;
};


#endif /* CJSONHELPER_HPP_ */
