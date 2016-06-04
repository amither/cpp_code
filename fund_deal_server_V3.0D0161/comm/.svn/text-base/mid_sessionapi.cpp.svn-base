
#include "mid_sessionapi.h"
#include "error.h"
#include "cftlog.h"
#include "parameter.h"
#include "appcomm.h"

CSessionApi::CSessionApi()
{
    GlobalConfig::SessionCfg &scfg = gPtrConfig->m_SessionSvrCfg;
    vector<SessionHostInfo>::iterator it = scfg.sess_hosts.begin();
    for(; it!=scfg.sess_hosts.end(); ++it)
    {
        struct_serveraddr svrinfo;
        svrinfo.strIP = it->strHost.c_str();
        svrinfo.iPort = it->iPort;
        m_SessionCfg.m_vSessions.push_back(svrinfo);

        TRACE_DEBUG("session node %s added",it->strHost.c_str());
    }

    m_SessionCfg.m_shmid = gPtrConfig->m_SessionSvrCfg.shm_id;
    m_SessionCfg.m_timeout = gPtrConfig->m_SessionSvrCfg.conn_timeout;
    m_SessionCfg.m_sockettimeout = gPtrConfig->m_SessionSvrCfg.rw_timeout;
    m_SessionCfg.m_nIsCheckShm = gPtrConfig->m_SessionSvrCfg.ischk_shm;
    m_SessionCfg.m_nCheckShmNum = gPtrConfig->m_SessionSvrCfg.chk_shm_num;

    enum_session_err ret = m_Session.Init(m_SessionCfg, "");
    if (SESSIONERR_SUCESS != ret)
    {
        string errinfo = cxx_printf("session init failed! errinfo=%s",
            m_Session.GetErrMsg().c_str());
        throw CException(ERR_SESSION_SERVER_ADD, errinfo.c_str(), __FILE__, __LINE__);        
    } else {
        TRACE_DEBUG("session init succeed!");
    }    
}

CSessionApi::~CSessionApi()
{
}

void CSessionApi::getSessionData(const string& strKey, map<string, string> &exKeys, map<string, string> &sessionValues) throw(CException)
{
    static string local_ip =getLocalHostIp();

    //TRACE_DEBUG("local_ip=%s", local_ip.c_str());
    
    enum_session_err ret;
    ret = m_Session.SetSessionKey(strKey);
    if (SESSIONERR_SUCESS != ret) {
        throw CException(ERR_SESSION_DATA_GET, "set session key failed", __FILE__, __LINE__);
    }

    AuditLog auditLog;
    auditLog.bid = gPtrConfig->m_SessionSvrCfg.bid;//if bid=-1,audit log will be disabled
    auditLog.fromID = exKeys["from"];
    auditLog.selfID = exKeys["self"];
    auditLog.toID = "session";
    auditLog.Ext_Map["qlskey"] = exKeys["qlskey"];
    auditLog.Ext_Map["msgno"] = exKeys["msgno"];
    auditLog.Ext_Map["ip"] = local_ip;
    auditLog.Ext_Map["uin"] = exKeys["qluin"];
    
    ret = m_Session.Get(sessionValues, auditLog);
    if (SESSIONERR_SUCESS != ret) {
        TRACE_ERROR("get session data faild.key=%s,err=%d,errmsg=%s",
            strKey.c_str(), ret, m_Session.GetErrMsg().c_str());
        throw CException(ERR_SESSION_DATA_GET, "get session data failed", __FILE__, __LINE__);
    }    
}

/*
 * 登录态校验
 */
void CSessionApi::checkLogin(const string& strKey, const string& strUin, map<string, string> &exKeys) throw(CException)
{
    if(strKey.empty() || strUin.empty())
    {
        throw CException(ERR_SESSION_DATA_EMPTY, "qlskey or qluin empty", __FILE__, __LINE__);
    }

    map<string, string> sessionValues;
    getSessionData(strKey, exKeys, sessionValues);

    //如果有if_login值认为是手Q登陆,否则认为是微信登陆
    map<string, string>::const_iterator cite = sessionValues.find("if_login");
    if (cite != sessionValues.end()) {
        checkMQQLogin(strKey, strUin, sessionValues);
    } else {
        checkWxLogin(strKey, strUin, sessionValues);
    }
}

/**
 * 检查微信登陆态,uin=xx&status=xx
 * @param strKey 
 * @param strUin 
 * @param sessionValues 
 */
void CSessionApi::checkWxLogin(const string& strKey, const string& strUin, const map<string, string> &sessionValues) throw(CException)
{
    //获取uin参数
    map<string, string>::const_iterator cite = sessionValues.find("uin");
    string strSessUin;
    if (cite != sessionValues.end())
        strSessUin = cite->second;
    
    if(strSessUin.empty())
    {
        TRACE_ERROR("session data uin is empty.key=%s", strKey.c_str());
        throw CException(ERR_SESSION_CHECK, "session data uin is empty", __FILE__, __LINE__);
    }

    //比较session中uin与参数中的是否一致
    if(strUin != strSessUin)
    {
        TRACE_ERROR("session data uin is inconsistent.key=%s", strKey.c_str());
        throw CException(ERR_SESSION_CHECK, "param uin not equal session data uin", __FILE__, __LINE__);
    }

    //检测状态是否为微信登陆或QQ登陆
    cite = sessionValues.find("status");
    if (cite == sessionValues.end()) {
        TRACE_ERROR("session data status is not exsit.key=%s", strKey.c_str());
        throw CException(ERR_SESSION_CHECK, "session data status is invalid", __FILE__, __LINE__);
    }

    int status = atoi((cite->second).c_str());
    if (0 == (WXLOGIN & status)) {
        TRACE_ERROR("session data status is not WXLOGIN. key=%s, status=%d",
            strKey.c_str(), status);
        throw CException(ERR_SESSION_DATA_GET, "session data status is invalid", __FILE__, __LINE__);
    } 
}

/**
 * 检查手Q登陆态,user_id=xx&if_login=xx
 * @param strKey 
 * @param strUin 
 * @param exKeys 
 */
void CSessionApi::checkMQQLogin(const string& strKey, const string& strUin, const map<string, string> &sessionValues) throw(CException)
{
    //获取uin参数
    map<string, string>::const_iterator cite = sessionValues.find("user_id");
    string strSessUin;
    if (cite != sessionValues.end())
        strSessUin = cite->second;
    
    if(strSessUin.empty())
    {
        TRACE_ERROR("session data uin is empty.key=%s", strKey.c_str());
        throw CException(ERR_SESSION_CHECK, "session data uin is empty", __FILE__, __LINE__);
    }

    //比较session中uin与参数中的是否一致
    if(strUin != strSessUin)
    {
        TRACE_ERROR("session data uin is inconsistent.key=%s", strKey.c_str());
        throw CException(ERR_SESSION_CHECK, "param uin not equal session data uin", __FILE__, __LINE__);
    }

    //检测状态是否为微信登陆或QQ登陆
    cite = sessionValues.find("if_login");
    if (cite == sessionValues.end()) {
        TRACE_ERROR("session data if_login is not exsit.key=%s", strKey.c_str());
        throw CException(ERR_SESSION_CHECK, "session data if_login is invalid", __FILE__, __LINE__);
    }

    //if_login为1表示手Q登陆
    int status = atoi((cite->second).c_str());
    if (1 != status) {
        TRACE_ERROR("session data status is not Mobile QQ LOGIN. key=%s, status=%d",
            strKey.c_str(), status);
        throw CException(ERR_SESSION_DATA_GET, "session data status is invalid", __FILE__, __LINE__);
    }
}


