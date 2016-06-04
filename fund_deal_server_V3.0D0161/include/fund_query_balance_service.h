/**
  * FileName: fund_query_balance_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-12-31
  * Description: �����׷��� ��ѯ�����ֵ�˻����
  */

#ifndef _FUND_QUERY_BALANCE_SERVICE_H_
#define _FUND_QUERY_BALANCE_SERVICE_H_
class FundQueryBalance
{
public:
    FundQueryBalance(CMySQL* mysql,int type=NO_CHECK_LOGIN);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

	void CheckParams() throw (CException);
	void CheckFundBind() throw (CException);
	void queryBalance();

private:

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_fund_conn;                // �������ݿ����Ӿ��
    int m_servicetype;
    vector<SubaccUser> m_subaccListUser;
    map<int,string> m_cutypeToSpid;
    vector<int> m_subacc_curtype_list;

};

#endif 

