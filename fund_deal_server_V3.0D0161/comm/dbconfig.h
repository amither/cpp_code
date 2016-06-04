#ifndef _DBCONFIGURE_H_
#define _DBCONFIGURE_H_

#include "exception.h"

const int MAX_GET_DB_CONF_ITEM = 30;

struct DbName
{
    string  confKey; // IP��ַ
    string  confRole; //�û���
};

struct DbHostCfg
{
    char    szHost[15 + 1]; // IP��ַ
    char    szUser[64 + 1]; //�û���
    char    szPswd[64 + 1]; // ����
    int     iPort; // �˿�
    int     iOvertime; // ��ʱʱ��
    char    szCharset[32 + 1];  //���ݿ��ַ���
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
     * ���ݿ����ƺ�role
     */
    vector<DbName> m_vecDbName;
    
    /**
     * �Ƿ�ִ��db���ò�ѯ
     */
    bool m_bExecuteDbQuery;
    
    /**
     * ���ݿ����ý��
     */
    map<string,DbHostCfg> m_mapDbConf;

};


#endif // _DBCONFIGURE_H_

