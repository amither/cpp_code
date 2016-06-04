#ifndef _DBCONFIGURE_H_
#define _DBCONFIGURE_H_

#include "exception.h"

const int MAX_GET_DB_CONF_ITEM = 30;

struct DbName
{
    string  confKey; // IP地址
    string  confRole; //用户名
};

struct DbHostCfg
{
    char    szHost[15 + 1]; // IP地址
    char    szUser[64 + 1]; //用户名
    char    szPswd[64 + 1]; // 密码
    int     iPort; // 端口
    int     iOvertime; // 超时时长
    char    szCharset[32 + 1];  //数据库字符集
    char    szConfKey[64 + 1];
    char    szConfRole[64 + 1];
};

class CDbConfig
{
public:
    
    CDbConfig(const string &serverName, const string &confSign);

    ~CDbConfig();

    void addDbName(const string &confKey, const string &confRole) throw(CException);

    DbHostCfg getDbConfigure(const string &confKey, const string &confRole) throw(CException);

    void executeBatchDbQuery() throw(CException);

protected:
    
    void executeDbQuery(int start, int end) throw(CException);

protected:
    /**
     * server_name
     */
    string m_sServername;

    /**
     * conf_sign
     */
    string m_sConfSign;

    /**
     * 数据库名称和role
     */
    vector<DbName> m_vecDbName;
    
    /**
     * 是否执行db配置查询
     */
    bool m_bExecuteDbQuery;
    
    /**
     * 数据库配置结果
     */
    map<string,DbHostCfg> m_mapDbConf;

};


#endif // _DBCONFIGURE_H_

