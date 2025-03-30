/*******************************************************************************
 * Project:  neb
 * @file     JsonObj.cpp
 * @brief 
 * @author   bwarliao
 * @date:    2014-7-16
 * @note
 * Modify history:
 ******************************************************************************/

#include "CJsonObject.h"


JsonObj::JsonObj()
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL)
{
    // m_pJsonData = cJSON_CreateObject();  
}

JsonObj::JsonObj(const std::string& strJson)
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL)
{
    Parse(strJson);
}

JsonObj::JsonObj(const JsonObj* pJsonObject)
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL)
{
    if (pJsonObject)
    {
        Parse(pJsonObject->ToString());
    }
}

JsonObj::JsonObj(const JsonObj& oJsonObject)
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL)
{
    Parse(oJsonObject.ToString());
}

JsonObj::~JsonObj()
{
    Clear();
}

JsonObj& JsonObj::operator=(const JsonObj& oJsonObject)
{
    Parse(oJsonObject.ToString().c_str());
    return(*this);
}

bool JsonObj::operator==(const JsonObj& oJsonObject) const
{
    return(this->ToString() == oJsonObject.ToString());
}

bool JsonObj::AddEmptySubObject(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateObject();
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = std::string("create sub empty object error!");
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    m_listKeys.clear();
    return(true);
}

bool JsonObj::AddEmptySubArray(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateArray();
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = std::string("create sub empty array error!");
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    m_listKeys.clear();
    return(true);
}

bool JsonObj::GetKey(std::string& strKey)
{
    if (IsArray())
    {
        return(false);
    }
    if (m_listKeys.size() == 0)
    {
        cJSON* pFocusData = NULL;
        if (m_pJsonData != NULL)
        {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL)
        {
            pFocusData = m_pExternJsonDataRef;
        }
        else
        {
            return(false);
        }

        cJSON *c = pFocusData->child;
        while (c)
        {
            m_listKeys.push_back(c->string);
            c = c->next;
        }
        m_itKey = m_listKeys.begin();
    }

    if (m_itKey == m_listKeys.end())
    {
        strKey = "";
        m_itKey = m_listKeys.begin();
        return(false);
    }
    else
    {
        strKey = *m_itKey;
        ++m_itKey;
        return(true);
    }
}

JsonObj& JsonObj::operator[](const std::string& strKey)
{
    std::map<std::string, JsonObj*>::iterator iter;
    iter = m_mapJsonObjectRef.find(strKey);
    if (iter == m_mapJsonObjectRef.end())
    {
        cJSON* pJsonStruct = NULL;
        if (m_pJsonData != NULL)
        {
            if (m_pJsonData->type == cJSON_Object)
            {
                pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL)
        {
            if (m_pExternJsonDataRef->type == cJSON_Object)
            {
                pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL)
        {
            JsonObj* pJsonObject = new JsonObj();
            m_mapJsonObjectRef.insert(std::pair<std::string, JsonObj*>(strKey, pJsonObject));
            return(*pJsonObject);
        }
        else
        {
            JsonObj* pJsonObject = new JsonObj(pJsonStruct);
            m_mapJsonObjectRef.insert(std::pair<std::string, JsonObj*>(strKey, pJsonObject));
            return(*pJsonObject);
        }
    }
    else
    {
        return(*(iter->second));
    }
}

JsonObj& JsonObj::operator[](unsigned int uiWhich)
{
    std::map<unsigned int, JsonObj*>::iterator iter;
    iter = m_mapJsonArrayRef.find(uiWhich);
    if (iter == m_mapJsonArrayRef.end())
    {
        cJSON* pJsonStruct = NULL;
        if (m_pJsonData != NULL)
        {
            if (m_pJsonData->type == cJSON_Array)
            {
                pJsonStruct = cJSON_GetArrayItem(m_pJsonData, uiWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL)
        {
            if (m_pExternJsonDataRef->type == cJSON_Array)
            {
                pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, uiWhich);
            }
        }
        if (pJsonStruct == NULL)
        {
            JsonObj* pJsonObject = new JsonObj();
            m_mapJsonArrayRef.insert(std::pair<unsigned int, JsonObj*>(uiWhich, pJsonObject));
            return(*pJsonObject);
        }
        else
        {
            JsonObj* pJsonObject = new JsonObj(pJsonStruct);
            m_mapJsonArrayRef.insert(std::pair<unsigned int, JsonObj*>(uiWhich, pJsonObject));
            return(*pJsonObject);
        }
    }
    else
    {
        return(*(iter->second));
    }
}

std::string JsonObj::operator()(const std::string& strKey) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(std::string(""));
    }
    if (pJsonStruct->type == cJSON_String)
    {
        return(pJsonStruct->valuestring);
    }
    else if (pJsonStruct->type == cJSON_Int)
    {
        char szNumber[128] = {0};
        if (pJsonStruct->sign == -1)
        {
            if ((int64)pJsonStruct->valueint <= (int64)INT_MAX && (int64)pJsonStruct->valueint >= (int64)INT_MIN)
            {
                snprintf(szNumber, sizeof(szNumber), "%d", (int32)pJsonStruct->valueint);
            }
            else
            {
                snprintf(szNumber, sizeof(szNumber), "%lld", (int64)pJsonStruct->valueint);
            }
        }
        else
        {
            if (pJsonStruct->valueint <= (uint64)UINT_MAX)
            {
                snprintf(szNumber, sizeof(szNumber), "%u", (uint32)pJsonStruct->valueint);
            }
            else
            {
                snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
            }
        }
        return(std::string(szNumber));
    }
    else if (pJsonStruct->type == cJSON_Double)
    {
        char szNumber[128] = {0};
        if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9)
        {
            snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
        }
        else
        {
            snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
        }
    }
    else if (pJsonStruct->type == cJSON_False)
    {
        return(std::string("false"));
    }
    else if (pJsonStruct->type == cJSON_True)
    {
        return(std::string("true"));
    }
    return(std::string(""));
}

std::string JsonObj::operator()(unsigned int uiWhich) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, uiWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, uiWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(std::string(""));
    }
    if (pJsonStruct->type == cJSON_String)
    {
        return(pJsonStruct->valuestring);
    }
    else if (pJsonStruct->type == cJSON_Int)
    {
        char szNumber[128] = {0};
        if (pJsonStruct->sign == -1)
        {
            if ((int64)pJsonStruct->valueint <= (int64)INT_MAX && (int64)pJsonStruct->valueint >= (int64)INT_MIN)
            {
                snprintf(szNumber, sizeof(szNumber), "%d", (int32)pJsonStruct->valueint);
            }
            else
            {
                snprintf(szNumber, sizeof(szNumber), "%lld", (int64)pJsonStruct->valueint);
            }
        }
        else
        {
            if (pJsonStruct->valueint <= (uint64)UINT_MAX)
            {
                snprintf(szNumber, sizeof(szNumber), "%u", (uint32)pJsonStruct->valueint);
            }
            else
            {
                snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
            }
        }
        return(std::string(szNumber));
    }
    else if (pJsonStruct->type == cJSON_Double)
    {
        char szNumber[128] = {0};
        if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9)
        {
            snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
        }
        else
        {
            snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
        }
    }
    else if (pJsonStruct->type == cJSON_False)
    {
        return(std::string("false"));
    }
    else if (pJsonStruct->type == cJSON_True)
    {
        return(std::string("true"));
    }
    return(std::string(""));
}

bool JsonObj::Parse(const std::string& strJson)
{
    Clear();
    m_pJsonData = cJSON_Parse(strJson.c_str());
    if (m_pJsonData == NULL)
    {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    return(true);
}

void JsonObj::Clear()
{
    m_pExternJsonDataRef = NULL;
    if (m_pJsonData != NULL)
    {
        cJSON_Delete(m_pJsonData);
        m_pJsonData = NULL;
    }
    for (std::map<unsigned int, JsonObj*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); ++iter)
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
    }
    m_mapJsonArrayRef.clear();
    for (std::map<std::string, JsonObj*>::iterator iter = m_mapJsonObjectRef.begin();
                    iter != m_mapJsonObjectRef.end(); ++iter)
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
    }
    m_mapJsonObjectRef.clear();
    m_listKeys.clear();
}

bool JsonObj::IsEmpty() const
{
    if (m_pJsonData != NULL)
    {
        return(false);
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::IsObject() const
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }

    if (pFocusData == NULL)
    {
        return(false);
    }

    if (pFocusData->type == cJSON_Object)
    {
        return(true);
    }
    else
    {
        return(false);
    }

}

bool JsonObj::IsArray() const
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }

    if (pFocusData == NULL)
    {
        return(false);
    }

    if (pFocusData->type == cJSON_Array)
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

std::string JsonObj::ToString() const
{
    char* pJsonString = NULL;
    std::string strJsonData = "";
    if (m_pJsonData != NULL)
    {
        pJsonString = cJSON_PrintUnformatted(m_pJsonData);
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pJsonString = cJSON_PrintUnformatted(m_pExternJsonDataRef);
    }
    if (pJsonString != NULL)
    {
        strJsonData = pJsonString;
        free(pJsonString);
    }
    return(strJsonData);
}

std::string JsonObj::ToFormattedString() const
{
    char* pJsonString = NULL;
    std::string strJsonData = "";
    if (m_pJsonData != NULL)
    {
        pJsonString = cJSON_Print(m_pJsonData);
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pJsonString = cJSON_Print(m_pExternJsonDataRef);
    }
    if (pJsonString != NULL)
    {
        strJsonData = pJsonString;
        free(pJsonString);
    }
    return(strJsonData);
}


bool JsonObj::Get(const std::string& strKey, JsonObj& oJsonObject) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    char* pJsonString = cJSON_Print(pJsonStruct);
    std::string strJsonData = pJsonString;
    free(pJsonString);
    if (oJsonObject.Parse(strJsonData))
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

bool JsonObj::Get(const std::string& strKey, std::string& strValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_String)
    {
        return(false);
    }
    strValue = pJsonStruct->valuestring;
    return(true);
}

bool JsonObj::Get(const std::string& strKey, int32& iValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    iValue = (int32)(pJsonStruct->valueint);
    return(true);
}

bool JsonObj::Get(const std::string& strKey, uint32& uiValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    uiValue = (uint32)(pJsonStruct->valueint);
    return(true);
}

bool JsonObj::Get(const std::string& strKey, int64& llValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    llValue = (int64)pJsonStruct->valueint;
    return(true);
}

bool JsonObj::Get(const std::string& strKey, uint64& ullValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    ullValue = (uint64)pJsonStruct->valueint;
    return(true);
}

bool JsonObj::Get(const std::string& strKey, bool& bValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type > cJSON_True)
    {
        return(false);
    }
    bValue = pJsonStruct->type;
    return(true);
}

bool JsonObj::Get(const std::string& strKey, float& fValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Double)
    {
        return(false);
    }
    fValue = (float)(pJsonStruct->valuedouble);
    return(true);
}

bool JsonObj::Get(const std::string& strKey, double& dValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Object)
        {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Double)
    {
        return(false);
    }
    dValue = pJsonStruct->valuedouble;
    return(true);
}

bool JsonObj::Add(const std::string& strKey, const JsonObj& oJsonObject)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    std::map<std::string, JsonObj*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end())
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, int32 iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, uint32 uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, int64 llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, uint64 ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt(ullValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, bool bValue, bool bValueAgain)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Add(const std::string& strKey, double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateObject();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Delete(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON_DeleteItemFromObject(pFocusData, strKey.c_str());
    std::map<std::string, JsonObj*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end())
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    m_listKeys.clear();
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, const JsonObj& oJsonObject)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    std::map<std::string, JsonObj*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end())
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, int32 iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, uint32 uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, int64 llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, uint64 ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, bool bValue, bool bValueAgain)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(const std::string& strKey, double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    return(true);
}

int JsonObj::GetArraySize()
{
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            return(cJSON_GetArraySize(m_pJsonData));
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            return(cJSON_GetArraySize(m_pExternJsonDataRef));
        }
    }
    return(0);
}

bool JsonObj::Get(int iWhich, JsonObj& oJsonObject) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    char* pJsonString = cJSON_Print(pJsonStruct);
    std::string strJsonData = pJsonString;
    free(pJsonString);
    if (oJsonObject.Parse(strJsonData))
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

bool JsonObj::Get(int iWhich, std::string& strValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_String)
    {
        return(false);
    }
    strValue = pJsonStruct->valuestring;
    return(true);
}

bool JsonObj::Get(int iWhich, int32& iValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    iValue = (int32)(pJsonStruct->valueint);
    return(true);
}

bool JsonObj::Get(int iWhich, uint32& uiValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    uiValue = (uint32)(pJsonStruct->valueint);
    return(true);
}

bool JsonObj::Get(int iWhich, int64& llValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    llValue = (int64)pJsonStruct->valueint;
    return(true);
}

bool JsonObj::Get(int iWhich, uint64& ullValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Int)
    {
        return(false);
    }
    ullValue = (uint64)pJsonStruct->valueint;
    return(true);
}

bool JsonObj::Get(int iWhich, bool& bValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type > cJSON_True)
    {
        return(false);
    }
    bValue = pJsonStruct->type;
    return(true);
}

bool JsonObj::Get(int iWhich, float& fValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Double)
    {
        return(false);
    }
    fValue = (float)(pJsonStruct->valuedouble);
    return(true);
}

bool JsonObj::Get(int iWhich, double& dValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL)
    {
        if (m_pJsonData->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        if(m_pExternJsonDataRef->type == cJSON_Array)
        {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    if (pJsonStruct->type != cJSON_Double)
    {
        return(false);
    }
    dValue = pJsonStruct->valuedouble;
    return(true);
}

bool JsonObj::Add(const JsonObj& oJsonObject)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    unsigned int uiLastIndex = (unsigned int)cJSON_GetArraySize(pFocusData) - 1;
    for (std::map<unsigned int, JsonObj*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); )
    {
        if (iter->first >= uiLastIndex)
        {
            if (iter->second != NULL)
            {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        }
        else
        {
            iter++;
        }
    }
    return(true);
}

bool JsonObj::Add(const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Add(int32 iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Add(uint32 uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Add(int64 llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Add(uint64 ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Add(int iAnywhere, bool bValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Add(float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Add(double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(const JsonObj& oJsonObject)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    for (std::map<unsigned int, JsonObj*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); )
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter++);
    }
    return(true);
}

bool JsonObj::AddAsFirst(const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(int32 iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(uint32 uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(int64 llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(uint64 ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(int iAnywhere, bool bValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::AddAsFirst(double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Delete(int iWhich)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON_DeleteItemFromArray(pFocusData, iWhich);
    for (std::map<unsigned int, JsonObj*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); )
    {
        if (iter->first >= (unsigned int)iWhich)
        {
            if (iter->second != NULL)
            {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        }
        else
        {
            iter++;
        }
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, const JsonObj& oJsonObject)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    std::map<unsigned int, JsonObj*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end())
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, int32 iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, uint32 uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, int64 llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)((uint64)llValue), -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, uint64 ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, 1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, bool bValue, bool bValueAgain)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

bool JsonObj::Replace(int iWhich, double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL)
    {
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    return(true);
}

JsonObj::JsonObj(cJSON* pJsonData)
    : m_pJsonData(NULL), m_pExternJsonDataRef(pJsonData)
{
}


