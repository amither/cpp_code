/**
  * FileName: fund_update_trans_date.h
  * Author: dianaliu
  * Version :1.0
  * Date: 2015-06-19
  * Description: �����׷��� ���½�������Ϣ
  */


#ifndef _FUND_UPDATE_TRANS_DATE_H_
#define _FUND_UPDATE_TRANS_DATE_H_

class FundUpdateTransDate
{
public:
    FundUpdateTransDate(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
    void CheckToken() throw (CException);
    string GenFundToken();
    void CheckParams() throw (CException);

private:
    void UpdateTransDate();
    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��
};

#endif

