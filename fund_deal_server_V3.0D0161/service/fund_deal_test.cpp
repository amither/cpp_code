/**
  * FileName: fund_deal_test.cpp
  * Author: jessiegao
  * Description: 单元测试代码
  */	

#include "fund_commfunc.h"
#include "fund_deal_test.h"
#include "fund_deal_testcase.h"

/**
  *  单元测试接口
  */
FundDealTest::FundDealTest(CMySQL* mysql)
{
	m_pFundCon=mysql;
}

/**
  * service step 1: 解析输入参数
  */
void FundDealTest::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_test] receives: %s", szMsg);
	m_szMsg=szMsg;
}

void FundDealTest::CheckToken(string testcase,string token) throw (CException)
{
	// 生成token
	stringstream ss;
	char buff[128] = {0};

	ss << testcase << "|" ;
	ss << "ea77db32c412d407030bbc91d2b2798f";

	getMd5(ss.str().c_str(), ss.str().size(), buff);

	if (StrUpper(token) != StrUpper(buff))
	{   
		TRACE_DEBUG("fund authen token check failed, testcase=%s,input=%s,real=%s", 
			testcase.c_str(),buff, token.c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
	}   
}

/**
  * 执行申购请求
  */
void FundDealTest::excute() throw (CException)
{
    CParams params;
    params.readStrParam(m_szMsg, "testcase", 0, 64);
    params.readStrParam(m_szMsg, "token", 0, 32);

	try{
    	CheckToken(params.getString("testcase"),params.getString("token"));
    	FundTestCaseObj::call(params.getString("testcase").c_str(),m_szMsg);
	}
	catch (CException& e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
		throw;
	}
}

/**
  * 打包输出参数
  */
void FundDealTest::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}
