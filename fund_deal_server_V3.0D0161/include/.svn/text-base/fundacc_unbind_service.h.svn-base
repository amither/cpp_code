/**
  * FileName: fundacc_unbind_service.h
  * Author: louisjiang	
  * Version :1.0
  * Date: 2014-04-11
  * Description: 基金交易服务 基金销户头文件
  */

#ifndef _FUND_DEAL_UNBIND_ACC_H_
#define _FUND_DEAL_UNBIND_ACC_H_

class FundUnbindAcc
{
public:
    FundUnbindAcc(CMySQL* mysql,int para);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void checkWhiteList() throw (CException);
    void checkToken()throw (CException);
    void checkFundbind() throw (CException);
    void checkBalance() throw (CException);

    //校验对账之后无申购记录
    void checkBuyRecord() throw (CException);

    //校验无在途t+1赎回
    void checkTplusRedem() throw (CException);

    void checkFundFetch()throw (CException);

	//检查是否有未完成的充值单
	void checkChargingRecord() throw (CException);

    //记录销户信息表
    void addUnbindRecord() throw (CException);

    //更新绑定表状态为注销
    void updateFundBindRecord() throw (CException);

    //更新安全卡表状态为注销
    void updateFundPayCardRecord() throw (CException);

	// 检查未确认资产数据
	void checkUnconfirm() throw (CException);

    void checkUnbind()  throw (CException);
    void doUnbind()  throw (CException);
    void checkRedem() throw (CException);
    
    CParams m_params;               // 消息参数
    CMySQL* m_fund_conn;           // 基金数据库连接句柄
    ST_FUND_BIND m_fund_bind;
    int m_op_type;
};



#endif
