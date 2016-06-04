/**
  * FileName: fund_spaccount_freeze_service.h
  * Author: sivenli	
  * Version :1.0
  * Date: 2015-3-31
  * Description: 基金交易服务 理财通账户冻结解冻接口
  */


#ifndef _FUND_ACCOUNT_FREEZE_H_
#define _FUND_ACCOUNT_FREEZE_H_

class FundAccountFreeze
{
public:
    FundAccountFreeze(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void CheckFundBind() throw (CException);
	void UpdateFundAccFreeze();
	void updateFundAccKvCache();
    void do_freeze();
    void undo_freeze();
    void do_query();
	
private:

    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;		// 基金用户绑定基金公司交易账户信息

    bool m_fund_bind_exist;				// 绑定基金增值帐号是否存在
    int  m_optype;                   // 操作类型
    
};

#endif /* _FUND_ACCOUNT_FREEZE_H_*/

