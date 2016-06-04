/**
  * FileName: fund_deal_testcase.cpp
  * Author: jessiegao
  * Description: 单元测试代码
  */	

#include "fund_commfunc.h"
#include "fund_deal_testcase.h"

#define DECLARE_FUNDDEAL_TESTCASE(ClassType)\
ClassType g_tmp_jessiegao_##ClassType(#ClassType); 

/**
 * 初始化单元测试全局链接
 */ 
map<string,FundTestCaseObj*> FundTestCaseObj::s_map = map<string,FundTestCaseObj*>();

/**
 * 定义单元测试类
 */
DECLARE_FUNDDEAL_TESTCASE(QueryUserTotalAssetTest);
//QueryUserTotalAssetTest g_tmp_jessiegao_QueryUserTotalAssetTest("QueryUserTotalAssetTest");

void QueryUserTotalAssetTest::test(char* szMsg) throw (CException)
{
    CParams params;
    params.readStrParam(szMsg, "tradeId", 0, 64);
    params.readIntParam(szMsg, "uid", 0, MAX_INTEGER);
    TRACE_DEBUG("do testcase[QueryUserTotalAssetTest]");
	LONG totalAssert = queryUserTotalAsset(params.getInt("uid"),params.getString("tradeId"));
	LONG totalAssertFalseTrue = queryUserTotalAsset(params.getInt("uid"),params.getString("tradeId"),false,true);
	LONG totalAssertTrueFalse = queryUserTotalAsset(params.getInt("uid"),params.getString("tradeId"),true,false);
	LONG totalAssertFalseFalse = queryUserTotalAsset(params.getInt("uid"),params.getString("tradeId"),false,false);
	TRACE_DEBUG("QueryUserTotalAssetTest:uid=[%d];tradeId=[%s];totalAssert=[%ld];totalAssertFalseTrue=[%ld];totalAssertTrueFalse=[%ld];totalAssertFalseFalse=[%ld]",
		params.getInt("uid"),params.getString("tradeId").c_str(),totalAssert,totalAssertFalseTrue,totalAssertTrueFalse,totalAssertFalseFalse);
}


