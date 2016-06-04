/**
  * FileName: FundUpdateEndType.h
  * Author: wenlonwang	
  * Version :1.0
  */


#ifndef _FUND_UPDATE_END_TYPE_H_
#define _FUND_UPDATE_END_TYPE_H_

class FundUpdateEndType
{
public:
    FundUpdateEndType(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
	
	void checkFundCloseTrans() throw (CException);
	void updateFundCloseTransEndType();


private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

	FundCloseTrans m_fundCloseTrans; //定期产品交易记录


};

#endif 

