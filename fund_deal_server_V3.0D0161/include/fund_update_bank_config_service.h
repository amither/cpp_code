/**
  * FileName: fund_update_bank_config_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2014-04-16
  * Description: 基金交易服务 更新银行配置信息
  */


#ifndef _FUND_UPDATE_BANK_CONFIG_H_
#define _FUND_UPDATE_BANK_CONFIG_H_

class FundUpdateBankConfig
{
public:
    FundUpdateBankConfig(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void UpdateBankConfig() ;
	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    
};

#endif /* _FUND_UPDATE_BANK_CONFIG_H_*/

