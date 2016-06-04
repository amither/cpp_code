
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <iostream>

using std::map;
using std::string;
using std::vector;

#include "dbconfig.h"
#include "error.h"
#include "UrlAnalyze.h"
#include "trpc_client.h"
#include "trpc_service.h"
#include "common.h"
#include "sqlapi.h"
#include "passconf_api.h"

using CFT::CUrlAnalyze;

#ifndef     SERVER_VERSION_SIGN
#error      "undefined SERVER_VERSION_SIGN"
#endif

/**
 * 构造函数
 */
CDbConfig::CDbConfig(const string &serverName, const string &confSign) 
    : m_sServername(serverName),m_sConfSign(confSign), m_bExecuteDbQuery(false)
{
}

/**
 * 析构函数
 */
CDbConfig::~CDbConfig()
{
}

/**
 * 添加数据库名称
 */
void CDbConfig::addDbName(const string &confKey, const string &confRole) throw(CException)
{
    DbName dbName;
    
    dbName.confKey = confKey;
    dbName.confRole = confRole;

    m_vecDbName.push_back(dbName);
}

/**
 * 获取数据库配置
 */
DbHostCfg CDbConfig::getDbConfigure(const string &confKey, const string &confRole) throw(CException)
{
    //检查是否执行配置查询
    if (!m_bExecuteDbQuery)
    {
        executeBatchDbQuery();
    }

    string key = confKey + "|" + confRole;

    //trpc_debug_log("getDbConfigure %s\n", key.c_str());
    
    if (m_mapDbConf.count(key) > 0)
    {
        return m_mapDbConf[key];
    }
    else
    {
        throw CException(ERR_DB_INITIAL, "no find db key " + key);
    }
}

/**
 * 执行数据库配置查询
 */
void CDbConfig::executeBatchDbQuery() throw(CException)
{
    for (unsigned int index = 0; index < m_vecDbName.size(); index += MAX_GET_DB_CONF_ITEM)
    {
        int end = index + MAX_GET_DB_CONF_ITEM > m_vecDbName.size() ? m_vecDbName.size() : index + MAX_GET_DB_CONF_ITEM;
        
        executeDbQuery(index, end);
    }
    
    m_bExecuteDbQuery = true;
}


/**
 * 执行数据库配置查询
 */
void CDbConfig::executeDbQuery(int start, int end) throw(CException)
{
    // 设置消息参数
    char szIdata[10 * 1024] = {0};
    char szOdata[10 * 1024] = {0};
    
    size_t oLen = sizeof(szOdata);

    // 设置参数
    CUrlAnalyze::setParam(szIdata, "server_name", m_sServername.c_str(), true); 
    CUrlAnalyze::setParam(szIdata, "conf_sign", m_sConfSign.c_str()); 
    CUrlAnalyze::setParam(szIdata, "conf_num", end - start); 

    // 设置明细信息
    for (int index = start; index < end; index++)
    {
        CUrlAnalyze::setParam(szIdata, add_suffix("conf_key", index - start).c_str(), m_vecDbName[index].confKey.c_str());
        CUrlAnalyze::setParam(szIdata, add_suffix("conf_role", index - start).c_str(), m_vecDbName[index].confRole.c_str());
    }
    
    // 获取数据库配置  
    trpc_debug_log("getPassConf idata %s\n", szIdata);
    
    if (getPassConf(szIdata, strlen(szIdata), szOdata, &oLen) != 0)
    {
        throw CException(ERR_DB_INITIAL, szOdata);
    }

    //trpc_debug_log("getPassConf odata %s\n", szOdata);
    
    // 解析数据库配置
    int conf_num = 0;
    CUrlAnalyze::getParam(szOdata, "conf_num", &conf_num);
    
    for (int index = 0; index < conf_num; index++)
    {
        DbHostCfg dbHost;
        
        memset(&dbHost, 0, sizeof(dbHost));
        
        CUrlAnalyze::getParam(szOdata, add_suffix("ip", index).c_str(), dbHost.szHost, sizeof(dbHost.szHost) - 1);
        CUrlAnalyze::getParam(szOdata, add_suffix("user", index).c_str(), dbHost.szUser, sizeof(dbHost.szUser) - 1);
        CUrlAnalyze::getParam(szOdata, add_suffix("passwd", index).c_str(), dbHost.szPswd, sizeof(dbHost.szPswd) - 1);
        CUrlAnalyze::getParam(szOdata, add_suffix("overtime", index).c_str(), &dbHost.iOvertime);
        CUrlAnalyze::getParam(szOdata, add_suffix("port", index).c_str(), &dbHost.iPort);
        CUrlAnalyze::getParam(szOdata, add_suffix("charset", index).c_str(), dbHost.szCharset, sizeof(dbHost.szCharset) - 1);
        CUrlAnalyze::getParam(szOdata, add_suffix("conf_key", index).c_str(), dbHost.szConfKey, sizeof(dbHost.szConfKey) - 1);
        CUrlAnalyze::getParam(szOdata, add_suffix("conf_role", index).c_str(), dbHost.szConfRole, sizeof(dbHost.szConfRole) - 1);
                
        //设置数据库配置默认值
        if (0 == dbHost.iPort)
        {
            dbHost.iPort = 3306;
        }

        if (0 == dbHost.iOvertime)
        {
            dbHost.iOvertime = 5;
        }

        if (0 == strlen(dbHost.szCharset))
        {
            strncpy(dbHost.szCharset, "latin1", sizeof(dbHost.szCharset) - 1);
        }
        
        string key = (string)dbHost.szConfKey + "|" + dbHost.szConfRole;
        
        trpc_debug_log("getPassConf add [%s][%s]\n", key.c_str(), dbHost.szHost);
        
        m_mapDbConf[key] = dbHost;
    }

    m_bExecuteDbQuery = true;
}


