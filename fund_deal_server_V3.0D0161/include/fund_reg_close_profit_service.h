/**
  * FileName: fund_reg_close_profit_service.h
  * Author: wenlonwang	
  * Version :1.0
  */


#ifndef _FUND_REG_CLOSE_PROFIT_H_
#define _FUND_REG_CLOSE_PROFIT_H_

class FundRegCloseProfit
{
public:
    FundRegCloseProfit(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
	void parseCloseFundDetail() throw (CException);

    void CheckFundBind() throw (CException);
	void AddProfit() throw (CException);

	void QueryFundProfit() throw (CException);
	void CheckBalance()  throw (CException);

	void RecordFundProfit();
	void UpdateProfitInfo();
	void updateCache(FundProfit& fund_profit);
	void CheckProfitRecord() throw (CException);
	void addEndProfitToTrans(const char* tailRedemId, LONG tailProfit);
	void updateFundCloseProfit();
	bool payNotifyOvertime(string pay_suc_time, int inteval);
	LONG calBalanceFeeLastProfit(const FundCloseTrans &closeTrans);
	LONG calBalanceFeeInBuy(const FundCloseTrans &closeTrans);

	void doSave() throw(CException);
	
	void saveCloseProfitRecord() throw(CException);

private:
	TRPC_SVCINFO* m_request;			// ��������
    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
    FundProfit m_fund_profit;			// �û�������Ϣ
    FundUserTotalAcc m_fundUserTotalAcc; //�������˻���Ϣ
    vector<FundCloseTrans> m_fundCloseTransVec; //���ڽ��׼�¼
    map<string, FundCloseTransProfit> m_fundCloseTransProfitMap; //�����������ϸ, keyΪ��ս���ʱ��
    map<string, FundCloseProfitRecord> m_fundCloseProfitMap; //�����������ϸ, keyΪ��ս���ʱ��

    LONG m_tail_profit; //ɨβ���棬ɨβ�������
	int m_curtype;
	bool m_fund_profit_exist;
	bool m_is_final_profit; // �ж��Ƿ��������ڴε����һ����ĩ����

};

#endif 

