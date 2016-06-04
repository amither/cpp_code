/**
  * FileName: fund_query_recommend_sp_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-21
  * Description: 基金交易服务 查询推荐绑定的基金公司
  */


#ifndef _FUND_QUERY_RECOMMEND_SP_H_
#define _FUND_QUERY_RECOMMEND_SP_H_

class FundQueryRecommendSp
{
public:
    FundQueryRecommendSp(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

	string getWxRecommendSp() throw (CException);
	string getCftQQRecommendSp() throw (CException);

	bool checkSpValid(string spid);
	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

};

#endif 

