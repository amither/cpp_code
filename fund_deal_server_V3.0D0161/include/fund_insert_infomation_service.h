/**
  * FileName: fund_insert_infomation_service.h
  * Author: sivenli	
  * Version :1.0
  * Date: 2015-04-23
  * Description: �����׷��� ������Ѷ��Ϣ
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

    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

};

#endif /* _FUND_INSERT_INFOMATION_H_*/

