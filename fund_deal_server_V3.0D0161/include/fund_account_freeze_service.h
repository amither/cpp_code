/**
  * FileName: fund_spaccount_freeze_service.h
  * Author: sivenli	
  * Version :1.0
  * Date: 2015-3-31
  * Description: �����׷��� ���ͨ�˻�����ⶳ�ӿ�
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

    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    ST_FUND_BIND m_fund_bind;		// �����û��󶨻���˾�����˻���Ϣ

    bool m_fund_bind_exist;				// �󶨻�����ֵ�ʺ��Ƿ����
    int  m_optype;                   // ��������
    
};

#endif /* _FUND_ACCOUNT_FREEZE_H_*/

