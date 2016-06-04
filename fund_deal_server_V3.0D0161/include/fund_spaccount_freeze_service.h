/**
  * FileName: fund_spaccount_freeze_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-28
  * Description: 基金交易服务 基金公司账户冻结解冻接口
  */


#ifndef _FUND_SPACCOUNT_FREEZE_H_
#define _FUND_SPACCOUNT_FREEZE_H_

class FundSpAccFreeze
{
public:
    FundSpAccFreeze(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void CheckFundBindSpAcc() throw (CException);
	void UpdateFundBindSpAccFreeze();
	void updateBindSpKvCache();

	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    FundBindSp m_fund_bind_sp_acc;		// 基金用户绑定基金公司交易账户信息

    bool m_bind_spacc_exist;				// 绑定基金增值帐号是否存在
    int  m_optype;                      // 操作类型
    
};

#endif /* _FUND_SPACCOUNT_FREEZE_H_*/

