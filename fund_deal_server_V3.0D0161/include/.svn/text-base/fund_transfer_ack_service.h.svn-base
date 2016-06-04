/**
  * FileName: fund_transfer_ack_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-16
  * Description: 份额转换确认接口
  */

#ifndef _FUND_DEAL_TRANSFER_ACK_H_
#define _FUND_DEAL_TRANSFER_ACK_H_

class FundTransferAck
{
public:
    FundTransferAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    void CheckFundBind() throw (CException);
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void checkTransferState() throw (CException);
    void CheckFundTransfer() throw (CException);
    void processTransferBuyReqSuc() throw (CException);
    void processTransferRedemSuc() throw (CException);
    void processTransferRedemFail() throw (CException);
    void processTransferBuySuc() throw (CException);
    void processTransferSubaccSaveRedo() throw (CException);
    void processTransferSpRedemRedoSuc() throw (CException);
    void dealTransfer() throw (CException);
    bool doSave(bool exp=false) throw(CException);
    void doDraw() throw (CException);
    void UpdateTransferState(int state,int sub_acc_state=0,const string&acc_time="")throw(CException);
    void RecordFundBuy() throw (CException);
    void RecordFundRedem() throw (CException);
    void checkBuyTrade() throw (CException);
    void checkRedemTrade() throw (CException);
    void updateCkvs();
    void checkUserBalance() throw (CException);
    void RecordTransferTimes() throw (CException);
    void checkResultSign()throw (CException);
    void UpdateFundRedem(int state)throw (CException);
    void UpdateFundBuy(int state, LONG close_listid = 0)throw (CException);
    void UpdateFundSpRedemInfo()throw (CException);
    void queryFundBindNewSpInfo()throw (CException);
	void queryNewFundSpAndFundcodeInfo();
	void checkUserPermissionBuyCloseFund();
	void recordCloseFund(string trade_date, LONG& close_listid);
	void checkCloseEndDate();
private:
    TRPC_SVCINFO*  m_request;
    CParams m_params;
    int m_optype;
    CMySQL *m_pFundCon;
    ST_FUND_BIND m_fund_bind;           // 用户开户信息
    ST_TRANSFER_FUND m_transferOrder; // 转换单
    ST_TRADE_FUND m_buyOrder;
    ST_TRADE_FUND m_redemOrder;
    FundBindSp m_fund_bind_orisp_acc;	
    FundBindSp m_fund_bind_newsp_acc;	
    FundSpConfig m_fund_orisp_config;
    FundSpConfig m_fund_newsp_config;
    ST_FUND_DYNAMIC m_dynamic_info;
	FundCloseTrans m_fundCloseTrans; //定期产品交易记录

	int m_close_fund_seqno; //定期产品可用序列
};

#endif

