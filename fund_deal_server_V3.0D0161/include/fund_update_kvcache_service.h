/**
  * FileName: fund_update_kvcache_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-10
  * Description: 基金交易服务 更新kv-cache接口
  */


#ifndef _FUND_UPDATE_KVCACHE_H_
#define _FUND_UPDATE_KVCACHE_H_

class FundUpdateKvcache
{
public:
    FundUpdateKvcache(CMySQL* mysql, int type=NO_CHECK_LOGIN);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
	void CheckParams() throw (CException);

	bool setProfitRateToKV();

	bool setKeyValue();

	bool setV2KeyValue();

	bool delKeyValue();

	bool setPayCardInfoToKV();

	bool setFundTotalaccInfoToKV();

    /**
     * 更新用户收益流水到ckv记录
     * @return 
     */
    bool setUserProfitRecordsToKV();
	bool setUserSubAccToKV();
	bool setUserCloseTransToKV();

    /**
     * 更新用户总收益到ckv
     * @return
     */
    bool setUserTotalProfitToKV();

	bool setUserFundTradeToKV();
    bool updateUserWhiteList();
	bool setFundUnconfirmCKV();
	bool setTDayToKV();
	bool setUnfinishTransCKV();
	bool setFundCashInTransitCKV();
private:


	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    int m_servicetype;


};

#endif 

