/**
  * FileName: fund_insert_trans_date.h
  * Author: dianaliu
  * Version :1.0
  * Date: 2015-06-19
  * Description: �����׷��� ���뽻������Ϣ
  */


#ifndef _FUND_INSERT_TRANS_DATE_H_
#define _FUND_INSERT_TRANS_DATE_H_

class FundInsertTransDate
{
public:
    FundInsertTransDate(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
    void CheckToken() throw (CException);
    void CheckParams() throw (CException);
    string GenFundToken();

private:
    void InsertTransDate();
    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��
    int isExists;

};

#endif

