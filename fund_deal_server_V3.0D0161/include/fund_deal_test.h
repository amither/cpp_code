
/**
  * FileName: fund_deal_test.h
  * Author: jessiegao
  * Description: 单元测试类
  */

#ifndef _FUND_DEAL_TEST_H_
#define _FUND_DEAL_TEST_H_

/**
  *  单元测试接口
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
	char* m_szMsg;				 // 输入参数透传
	CMySQL* m_pFundCon; 			   // 基金数据库连接句柄
};

#endif

