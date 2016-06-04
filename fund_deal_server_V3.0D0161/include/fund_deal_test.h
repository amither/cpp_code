
/**
  * FileName: fund_deal_test.h
  * Author: jessiegao
  * Description: ��Ԫ������
  */

#ifndef _FUND_DEAL_TEST_H_
#define _FUND_DEAL_TEST_H_

/**
  *  ��Ԫ���Խӿ�
  */
class FundDealTest
{	
public:
	FundDealTest(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
	void CheckToken(string testcase,string token) throw (CException);
		
private:
	char* m_szMsg;				 // �������͸��
	CMySQL* m_pFundCon; 			   // �������ݿ����Ӿ��
};

#endif

