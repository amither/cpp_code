/**
  * FileName: fund_insert_infomation_service.h
  * Author: sivenli	
  * Version :1.0
  * Date: 2015-04-23
  * Description: 基金交易服务 插入资讯信息
  */


#ifndef _FUND_INSERT_INFOMATION_H_
#define _FUND_INSERT_INFOMATION_H_

class FundInsertInfomation
{
public:
    FundInsertInfomation(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
	void InsertInfomation() ;
	
private:

    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

};

#endif /* _FUND_INSERT_INFOMATION_H_*/

