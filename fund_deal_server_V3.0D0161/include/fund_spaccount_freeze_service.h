/**
  * FileName: fund_spaccount_freeze_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-28
  * Description: �����׷��� ����˾�˻�����ⶳ�ӿ�
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

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    FundBindSp m_fund_bind_sp_acc;		// �����û��󶨻���˾�����˻���Ϣ

    bool m_bind_spacc_exist;				// �󶨻�����ֵ�ʺ��Ƿ����
    int  m_optype;                      // ��������
    
};

#endif /* _FUND_SPACCOUNT_FREEZE_H_*/

