/**
  * FileName: fund_nopass_reset_paycard_service.h
  * Author: jiggersong	
  * Version :1.0
  * Date: 2014-01-27
  * Description: 基金交易服务 重置用户的安全卡
  */


#ifndef _FUND_NOPASS_RESET_PAYCARD_H_
#define _FUND_NOPASS_RESET_PAYCARD_H_

/**
 * 操作类型，必须要兼容现在的客服调用
 */
enum RST_PAYCARD_OPTYPE {
	RST_TYPE_KF = 0,//客服重置
	RST_TYPE_USR_SELF_CHK = 1,//用户重置安全卡检查
	RST_TYPE_USR_SELF = 2//用户重置安全卡
};

class FundNopassResetPayCard
{
public:
    FundNopassResetPayCard(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void UpdatePayCard() ;

	void checkFundBind() throw (CException);
	void checkNewCardInfo() throw (CException);
	void checkCanRstPayCard() throw (CException);

	LONG getUsrBalance() throw (CException);
	LONG getChargingFee()throw (CException);
	LONG getFundFetchFee()throw (CException);
	LONG getRedemFee() throw (CException);
	LONG getBuyFee()throw (CException);

	void restPayCard() throw (CException);
	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

	ST_FUND_BIND m_fund_bind; 
	bool m_can_usr_rst_paycard;//用户是否要吧更换安全卡
};

#endif /* _FUND_NOPASS_RESET_PAYCARD_H_*/

