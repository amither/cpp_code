/**
  * FileName: fund_pre_record_user_acc_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2015-03-04
  * Description: ��¼Ǳ�������ͨ�û��˻�
  */

#ifndef _FUND_PRE_RECORD_USER_ACC_SERVICE_H_
#define _FUND_PRE_RECORD_USER_ACC_SERVICE_H_

class FundPreRecordUserAcc
{
public:
    FundPreRecordUserAcc(CMySQL* mysql);
    ~FundPreRecordUserAcc(){};
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
private:

    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    bool isOldUser() throw (CException);
    void RecordAccInfo()throw (CException);
    bool isAlreadyExist(ST_PREUSER_ACC& data)throw (CException);

    
    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

};



#endif

