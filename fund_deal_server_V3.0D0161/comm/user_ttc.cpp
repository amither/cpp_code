#include "user_ttc.h"

CUserTTC::CUserTTC(char* szAddr, int iPort, int iTimeout)
{
        char szTmp[16];
        
	m_UserTableTTC.IntKey();				        //设置key的类型
	m_UserTableTTC.SetTableName("t_user_");	        //设置表名，只写前缀
	
	snprintf(szTmp, sizeof(szTmp) -1, "%d", iPort);
	m_UserTableTTC.SetAddress(szAddr, szTmp);	//设置ip,port
	m_UserTableTTC.SetTimeout(iTimeout);			//io超时时间
}

CUserTTC::~CUserTTC()
{

}

int CUserTTC::GetUser(int iUid, int iCurType, Result* pResult)
{
	int iRet = 0;
	GetRequest getResult(&m_UserTableTTC);
    
	iRet = getResult.SetKey((uint32_t)iUid);
        iRet = getResult.EQ("Fcurtype", iCurType);

        iRet = getResult.Need("Fqqid");
	iRet = getResult.Need("Fcurtype");
        iRet = getResult.Need("Ftruename");
        iRet = getResult.Need("Fcompany_name");
        iRet = getResult.Need("Fbalance");
        iRet = getResult.Need("Fcon");
        iRet = getResult.Need("Fyday_balance");
        iRet = getResult.Need("Fmin_balance");
        iRet = getResult.Need("Fquota");
        iRet = getResult.Need("Fapay");
        iRet = getResult.Need("Fquota_pay");
        iRet = getResult.Need("Fsave_time");
        iRet = getResult.Need("Ffetch_time");
        iRet = getResult.Need("Flogin_ip");
        iRet = getResult.Need("Fmodify_time_c2c");
        iRet = getResult.Need("Fstate");
        //iRet = getResult.Need("Fmemo");
        iRet = getResult.Need("Fmodify_time");
        iRet = getResult.Need("Fuser_type");
        iRet = getResult.Need("Fbpay_state");
        iRet = getResult.Need("Ffetch");
        iRet = getResult.Need("Fsave");
        iRet = getResult.Need("Fcreate_time");
        iRet = getResult.Need("Fcredit_num");
        iRet = getResult.Need("Fatt_id");
        //iRet = getResult.Need("Fbalance_time");

	iRet = getResult.Execute(*pResult);
	
	return pResult->ResultCode();
}

int CUserTTC::PurgeUser(int iUid, int iCurType, Result* pResult)
{
	int iRet = 0;
	PurgeRequest purgeResult(&m_UserTableTTC);
    
	iRet = purgeResult.SetKey((uint32_t)iUid);
        iRet = purgeResult.EQ("Fcurtype", iCurType);
    
	iRet = purgeResult.Execute(*pResult);

	return pResult->ResultCode();
}

