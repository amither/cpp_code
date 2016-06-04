/**
  * FileName: fund_query_bind_sp_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-9-19
  * Description: �����׷��� ��ѯ�����ֵ�˻�����Ϣ
  */


#ifndef _FUND_QUERY_BIND_SP_H_
#define _FUND_QUERY_BIND_SP_H_

class FundQueryBindSp
{
public:
    FundQueryBindSp(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:

    void CheckParams() throw (CException);

    void CheckFundBind() throw (CException);
	void CheckFundBindSpAcc() throw (CException);


private:

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_fund_conn;                // �������ݿ����Ӿ��

    ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
    FundBindSp m_fund_bind_sp_acc;		// �����û��󶨻���˾�����˻���Ϣ

    bool m_bind_exist;                  // �˺���Ϣ�Ƿ����
    bool m_bind_spacc_exist;				// �󶨻�����ֵ�ʺ��Ƿ����

    int  relation_exist;                      // �󶨹�ϵ�Ƿ���� 0:������  1:����
};

#endif /* _FUND_QUERY_BIND_SP_H_ */

