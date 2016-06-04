
/**
  * FileName: fund_deal_testcase.h
  * Author: jessiegao
  * Description: 单元测试类
  */

#ifndef _FUND_DEAL_TESTCASE_H_
#define _FUND_DEAL_TESTCASE_H_

/**
 * 单元测试案例基类
 */
class FundTestCaseObj
{
public:	
	static map<string,FundTestCaseObj*> s_map; 
	
	static void call(const char * testCaseName, char* szMsg) throw (CException)
	{
		TRACE_DEBUG("calling test case:[%s]",testCaseName);
		map<string,FundTestCaseObj*>::iterator iter= s_map.find(testCaseName);
		if(iter==s_map.end())
		{
			TRACE_ERROR("not declare test case:[%s]",testCaseName);
			return;
		}
		FundTestCaseObj* obj = iter->second;
		obj->test(szMsg);
	}
	
public:	
	
	FundTestCaseObj(const char * Name)
	{
		m_name=Name;
		// TRACE_DEBUG("init FundTestCaseObj[%s]", Name);
		FundTestCaseObj::s_map[Name]=this;
		// TRACE_DEBUG("init FundTestCaseObj[%s] finish", Name);
	}
	~FundTestCaseObj()
	{
		FundTestCaseObj::s_map.erase(m_name);
	}
	virtual void test(char* szMsg) throw (CException)=0;
	
	string m_name;
	
};

/**
 * 定义单元测试类
 */
class QueryUserTotalAssetTest : public FundTestCaseObj
{
public:
	QueryUserTotalAssetTest(const char* name):FundTestCaseObj(name){};
	void test(char* szMsg) throw (CException);
};



#endif

