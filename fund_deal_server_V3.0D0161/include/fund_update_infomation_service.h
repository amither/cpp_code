/**
  * FileName: fund_update_infomation_service.h
  * Author: sivenli	
  * Version :1.0
  * Date: 2015-04-23
  * Description: 基金交易服务 更新资讯状态
  */


#ifndef _FUND_UPDATE_INFOMATION_STATE_H_
#define _FUND_UPDATE_INFOMATION_STATE_H_

class FundUpdateInfomation
{
public:
    FundUpdateInfomation(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
	void UpdateInfomation() ;
	
private:

    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

};

#endif /* _FUND_FUND_UPDATE_INFOMATION_H_*/

