/**
  * FileName: fund_query_total_profit_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-05
  * Description: �����׷��� ��ѯ�����漰��������
  */


#ifndef _FUND_QUERY_TOTAL_PROFIT_SERVICE_H_
#define _FUND_QUERY_TOTAL_PROFIT_SERVICE_H_
class FundQueryTotalProfit
{
public:
    FundQueryTotalProfit(CMySQL* mysql,int type=NO_CHECK_LOGIN);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

	void CheckParams() throw (CException);
	void CheckFundBind() throw (CException);

    void queryTotalProfit() throw (CException);

private:

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_fund_conn;                // �������ݿ����Ӿ��

    FundProfit m_fund_profit;           // �û���������Ϣ
    vector<FundProfit> m_fundProfitVec; //������������Ϣ

    int m_servicetype;


};

#endif 

