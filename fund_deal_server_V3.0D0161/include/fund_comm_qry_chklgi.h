/**
  * FileName: fund_comm_qry_chklgi.h
  * Author: louisjiang	
  * Version :1.0
  * Date: 2014-05-15
  * Description: 基金交易服务 查询serverice 校验登录态
  */


#ifndef _FUND__COMM_QUERY_CHECKLOGIN_SERVICE_H_
#define _FUND__COMM_QUERY_CHECKLOGIN_SERVICE_H_
class FundComQryChkLgi
{
public:
    FundComQryChkLgi(CMySQL* mysql,int type=NO_CHECK_LOGIN);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);
    string buildUserTransQuerySql() throw (CException);
    string buildTransQuerySql()  throw (CException);
    string buildFetchListQuerySql()  throw (CException);
    string buildProfitListQuerySql()  throw (CException);
    string  buildWeekListQuerySql()  throw (CException);
    string  buildBalanceListQuerySql()  throw (CException);
    string  buildCloseListQuerySql()  throw (CException);
    void packNVReturnMsg(TRPC_SVCINFO* rqst);
    void packXMLReturnMsg(TRPC_SVCINFO* rqst);
    
    void doQeury() throw (CException);
    void buildSql()  throw (CException);
    void checkUid()throw (CException);
private:

    CParams m_params;                   // 消息参数
    CStr2Map m_fieldsMap;
    CMySQL* m_fund_conn;                // 基金数据库连接句柄

    CStr2sVec m_result; //查询结果
    int m_servicetype;
    string m_querySql;
    ST_FUND_BIND m_fund_bind;
    string m_fields_nv_value;

};

#endif 


