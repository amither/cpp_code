#include "db_fund_bind_sp.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 





/**
*��ѯ�������ʺ�
*/
bool queryMasterSpAcc(CMySQL* pMysql, FundBindSp& data,  bool lock) //����
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Ftrade_id,Fspid,Fsp_user_id,Fsp_trans_id, "
                    " Facct_type,Fstate,Flstate,Frecon_state,Facc_time, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fexplain,Fstandby1 "
                    " FROM fund_db_%02d.t_fund_bind_sp_%d "
                    " WHERE Flstate <> %d AND Fstate = %d AND Facct_type = %d AND"
                    " Ftrade_id='%s' " 
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    LSTATE_INVALID,
                    BIND_SPACC_SUC,
                    BIND_SPACC_MASTER,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // ִ�в�ѯ
        pMysql->Query(szSql, iLen);
        // ȡ�����
        pRes = pMysql->FetchResult();
        // ��ȡ�����
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[2] ? row[2] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsp_user_id,row[3] ? row[3] : "", sizeof(data.Fsp_user_id) - 1);
            strncpy(data.Fsp_trans_id,row[4] ? row[4] : "", sizeof(data.Fsp_trans_id) - 1);
            data.Facct_type = row[5] ? atoi(row[5]) : 0;
            data.Fstate = row[6] ? atoi(row[6]) : 0;
            data.Flstate = row[7] ? atoi(row[7]) : 0;
            data.Frecon_state = row[8] ? atoi(row[8]) : 0;
            strncpy(data.Facc_time,row[9] ? row[9] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fcreate_time,row[10] ? row[10] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[11] ? row[11] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[12] ? row[12] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[13] ? row[13] : "", sizeof(data.Fexplain) - 1);
			data.Fstandby1= row[14] ? atoi(row[14]) : 0;

        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

/**
*��ѯ��Ӧ���̻��Ƿ�Ϊ��Ч���������ʺ�

void queryValidMasterSpAcc(CMySQL* pMysql, FundBindSp& data, string spid,  bool lock)
{
	if(!queryMasterSpAcc(pMysql, data, lock))
	{
		TRACE_ERROR("the fund bind sp account record not exist.");
        throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not exist");
	}

	if(spid != data.Fspid)
	{
		//���������ʺ�
		TRACE_ERROR("the sp account not the master account.");
        throw EXCEPTION(ERR_SP_ACC_NOT_MASTER_ACC, "the sp account not the master account.");
	}

	if(LSTATE_FREEZE == data.Flstate)
	{
		//�˻������᲻�������
		TRACE_ERROR("the fund bind sp account record has been frozen.");
        throw EXCEPTION(ERR_SP_ACC_FREEZE, "the fund bind sp account record has been frozen.");
	}
}
*/


bool queryFundBindSp(CMySQL* pMysql, FundBindSp& data,  bool lock) //����
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Ftrade_id,Fspid,Fsp_user_id,Fsp_trans_id, "
                    " Facct_type,Fstate,Flstate,Frecon_state,Facc_time, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fexplain,Fstandby1 "
                    " FROM fund_db_%02d.t_fund_bind_sp_%d "
                    " WHERE Flstate <> %d AND"
                    " Ftrade_id='%s'  AND " 
                    " Fspid='%s' " 
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    LSTATE_INVALID,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // ִ�в�ѯ
        pMysql->Query(szSql, iLen);
        // ȡ�����
        pRes = pMysql->FetchResult();
        // ��ȡ�����
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[2] ? row[2] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsp_user_id,row[3] ? row[3] : "", sizeof(data.Fsp_user_id) - 1);
            strncpy(data.Fsp_trans_id,row[4] ? row[4] : "", sizeof(data.Fsp_trans_id) - 1);
            data.Facct_type = row[5] ? atoi(row[5]) : 0;
            data.Fstate = row[6] ? atoi(row[6]) : 0;
            data.Flstate = row[7] ? atoi(row[7]) : 0;
            data.Frecon_state = row[8] ? atoi(row[8]) : 0;
            strncpy(data.Facc_time,row[9] ? row[9] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fcreate_time,row[10] ? row[10] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[11] ? row[11] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[12] ? row[12] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[13] ? row[13] : "", sizeof(data.Fexplain) - 1);
			data.Fstandby1= row[14] ? atoi(row[14]) : 0;

        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

/**
* ��ѯ�󶨻���˾�˻��ɹ��ļ�¼
*/
bool queryFundBindSpSuccess(CMySQL* pMysql, FundBindSp& data,  bool lock) 
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Ftrade_id,Fspid,Fsp_user_id,Fsp_trans_id, "
                    " Facct_type,Fstate,Flstate,Frecon_state,Facc_time, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fexplain,Fstandby1 "
                    " FROM fund_db_%02d.t_fund_bind_sp_%d "
                    " WHERE Flstate <> %d AND Fstate = %d AND"
                    " Ftrade_id='%s'  AND " 
                    " Fspid='%s' " 
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    LSTATE_INVALID,
                    BIND_SPACC_SUC,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // ִ�в�ѯ
        pMysql->Query(szSql, iLen);
        // ȡ�����
        pRes = pMysql->FetchResult();
        // ��ȡ�����
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[2] ? row[2] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsp_user_id,row[3] ? row[3] : "", sizeof(data.Fsp_user_id) - 1);
            strncpy(data.Fsp_trans_id,row[4] ? row[4] : "", sizeof(data.Fsp_trans_id) - 1);
            data.Facct_type = row[5] ? atoi(row[5]) : 0;
            data.Fstate = row[6] ? atoi(row[6]) : 0;
            data.Flstate = row[7] ? atoi(row[7]) : 0;
            data.Frecon_state = row[8] ? atoi(row[8]) : 0;
            strncpy(data.Facc_time,row[9] ? row[9] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fcreate_time,row[10] ? row[10] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[11] ? row[11] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[12] ? row[12] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[13] ? row[13] : "", sizeof(data.Fexplain) - 1);
			data.Fstandby1= row[14] ? atoi(row[14]) : 0;

        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}


bool queryFundBindAllSp(CMySQL* pMysql, vector<FundBindSp>& dataList, const string& trade_id, bool lock) //����
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Ftrade_id,Fspid,Fsp_user_id,Fsp_trans_id, "
                    " Facct_type,Fstate,Flstate,Frecon_state,Facc_time, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fexplain,Fstandby1,Fchannel_id "
                    " FROM fund_db_%02d.t_fund_bind_sp_%d "
                    " WHERE Flstate <> %d AND Fstate = %d AND"
                    " Ftrade_id='%s'  " 
                    " %s ",
                    Sdb2(trade_id.c_str()),
                    Stb2(trade_id.c_str()),
                    LSTATE_INVALID,
                    BIND_SPACC_SUC,
                    pMysql->EscapeStr(trade_id).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // ִ�в�ѯ
        pMysql->Query(szSql, iLen);
        // ȡ�����
        pRes = pMysql->FetchResult();
        // ��ȡ�����
        iRow = mysql_num_rows(pRes);
        if(iRow <0 )
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
			FundBindSp data;
            memset(&data, 0, sizeof(data));
            
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[2] ? row[2] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsp_user_id,row[3] ? row[3] : "", sizeof(data.Fsp_user_id) - 1);
            strncpy(data.Fsp_trans_id,row[4] ? row[4] : "", sizeof(data.Fsp_trans_id) - 1);
            data.Facct_type = row[5] ? atoi(row[5]) : 0;
            data.Fstate = row[6] ? atoi(row[6]) : 0;
            data.Flstate = row[7] ? atoi(row[7]) : 0;
            data.Frecon_state = row[8] ? atoi(row[8]) : 0;
            strncpy(data.Facc_time,row[9] ? row[9] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fcreate_time,row[10] ? row[10] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[11] ? row[11] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[12] ? row[12] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[13] ? row[13] : "", sizeof(data.Fexplain) - 1);
            data.Fstandby1= row[14] ? atoi(row[14]) : 0;
            strncpy(data.Fchannel_id,row[15] ? row[15] : "", sizeof(data.Fchannel_id) - 1);
			dataList.push_back(data);
        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow >= 1;
}



/**
*��ѯ��Ч�İ󶨻���˾�˻���Ϣ
*/
void queryValidFundBindSp(CMySQL* pMysql, FundBindSp& data, bool lock)
{
	if(!queryFundBindSpSuccess(pMysql, data, lock))
	{
		TRACE_ERROR("the fund bind sp account record not exist.spid:%s", data.Fspid);
        throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not exist");
	}

	if(LSTATE_FREEZE == data.Flstate)
	{
		//�˻������᲻�������
		TRACE_ERROR("the fund bind sp account record has been frozen.");
        throw EXCEPTION(ERR_SP_ACC_FREEZE, "the fund bind sp account record has been frozen.");
	}
	
}

void checkValidFundBindSp(vector<FundBindSp> & fundBindSpVec, string new_spid)
{
	for(vector<FundBindSp>::iterator iter = fundBindSpVec.begin();iter != fundBindSpVec.end(); ++iter)
	{
		FundBindSp& data = (*iter);
		if(new_spid == data.Fspid)
		{
			
			if(BIND_SPACC_SUC != data.Fstate)
			{
				TRACE_ERROR("the fund bind sp account record not success.spid:%s",data.Fspid);
		        throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not success");
			}

			if(LSTATE_FREEZE == data.Flstate)
			{
				//�˻������᲻�������
				TRACE_ERROR("the fund bind sp account record has been frozen.");
		        throw EXCEPTION(ERR_SP_ACC_FREEZE, "the fund bind sp account record has been frozen.");
			}

			return ;
		}

	}

	throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not exist");
}



void insertFundBindSp(CMySQL* pMysql, FundBindSp &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_bind_sp_%d("
                    " Ftrade_id,Fspid,Facct_type,Fchannel_id,Fstate,Flstate, "
                    " Frecon_state,Facc_time,Fcreate_time,Fmodify_time)"
                    " VALUES("
                    " '%s','%s',%d,'%s',%d,%d, "
                    " %d,'%s','%s','%s')",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    data.Facct_type,
                    pMysql->EscapeStr(data.Fchannel_id).c_str(),
                    data.Fstate,
                    data.Flstate,
                    data.Frecon_state,
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
}


  

/*
*update����
*updateӰ������Ϊ1ʱ��ȷ��Ϊ0��>1�����׳��쳣
*/  
void updateFundBindSp(CMySQL* pMysql, FundBindSp& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db_%02d.t_fund_bind_sp_%d SET "
                    " Fsp_user_id='%s',"
                    " Fsp_trans_id='%s',"
                    " Fstate=%d,"
                    " Facct_type=%d,"
                    " Fstandby1=%d,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fimt_id=%zd", 
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
					pMysql->EscapeStr(data.Fsp_user_id).c_str(),
                    pMysql->EscapeStr(data.Fsp_trans_id).c_str(),
                    data.Fstate,
                    data.Facct_type,
                    data.Fstandby1,
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where����--------
                    data.Fimt_id
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
    // �ж�Ӱ�������Ƿ�Ψһ
    if (pMysql->AffectedRows() != 1)
    {
		TRACE_ERROR("affected row[%d] not equal 1. Fimt_id=[%lld], ", pMysql->AffectedRows(), data.Fimt_id);
        throw CException(ERR_DB_AFFECTED, "affected row not equal 1!", __FILE__, __LINE__);
    }
}

/*
*���°󶨼�¼����״̬
*updateӰ������Ϊ1ʱ��ȷ��Ϊ0��>1�����׳��쳣
*/  
void updateFundBindSpFreeze(CMySQL* pMysql, FundBindSp& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db_%02d.t_fund_bind_sp_%d SET "
                    " Flstate=%d,"
                    " Fmemo='%s',"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fimt_id=%zd", 
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Flstate,
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where����--------
                    data.Fimt_id
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
    // �ж�Ӱ�������Ƿ�Ψһ
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


/*
*update����,�������˻�����
*updateӰ������Ϊ1ʱ��ȷ��Ϊ0��>1�����׳��쳣
*/  
void updateFundBindSpAcctType(CMySQL* pMysql, FundBindSp& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db_%02d.t_fund_bind_sp_%d SET "
                    " Facct_type=%d,"
                    " Fstandby1=%d,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fimt_id=%zd", 
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Facct_type,
                    data.Fstandby1,
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where����--------
                    data.Fimt_id
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
    // �ж�Ӱ�������Ƿ�Ψһ
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

/*
*update����,����acc_time
*updateӰ������Ϊ1ʱ��ȷ��Ϊ0��>1�����׳��쳣
*/  
void updateBindSpStateAcctime(CMySQL* pMysql, FundBindSp& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db_%02d.t_fund_bind_sp_%d SET "
                    " Facc_time='%s',"
                    " Fstate='%d',"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fimt_id=%zd", 
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    data.Fstate,
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where����--------
                    data.Fimt_id
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
    // �ж�Ӱ�������Ƿ�Ψһ
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


/**
*ɾ��cache
*/
/*
void deleteFundBindAllSpFromKV(const string& trade_id)
{
	int appid = gPtrConfig->m_KvCfg.appid;
   	//string key = gPtrConfig->m_KvCfg.kvAddress + trade_id;
   	string key = trade_id;


    gCkvSvrOperator->del(appid, key);	
}
*/

/**
* Ĭ�ϻ���ֻ��¼ckv �����ݿ��еļ�¼�Ѿ���Ч
* changeDefaultSpΪtrue�ǲŽ�ckv�е�Ĭ�ϻ����޸�Ϊ�����defaultSp,�����������ckv��Ĭ�ϻ��𲻱�

bool setFundBindAllSpFromKV(CMySQL* pMysql,const string& trade_id, bool changeDefaultSp, string defaultSp, bool set_type, string modify_time)
{
	if(trade_id.empty() || (changeDefaultSp && (defaultSp.empty() || modify_time.empty())))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "trade_id or defaultSp or modify_time empty.");    
	}
	
	string key = trade_id;

	vector<FundBindSp> fundBindSpVec;

	//ֻΪ�޸�Ĭ�ϻ���ģ�����ѯ���ݿ⣬��������ݿ����ѹ��
	if(changeDefaultSp)
	{
		if(!getFundBindAllSpFromKV(pMysql, trade_id, fundBindSpVec))
		{
			return false;
		}
	}
	else{
		//��ѯĬ�ϻ���
		FundBindSp data;
    	memset(&data, 0, sizeof(FundBindSp));
    	strncpy(data.Ftrade_id, trade_id.c_str(), sizeof(data.Ftrade_id) - 1);
		if(getDefaultSpFromKV(pMysql,data))
		{
			defaultSp = data.Fspid;
			set_type = data.Fstandby1;
			modify_time = data.Fmodify_time;
			
		}
		else
		{
			//��Ĭ�ϻ���ģ������£��״��û���д��ckv�쳣�����ܳ����������
			defaultSp = "";	
		}
		
		if( !queryFundBindAllSp(pMysql, fundBindSpVec, trade_id,false))
		{
			TRACE_DEBUG("no fund bind sp record");
			return true;
		}
	}
}
*/

bool setFundBindAllSpToKVFromDB(CMySQL* pMysql,const string& trade_id)
{
	vector<FundBindSp> fundBindSpVec;

	if( !queryFundBindAllSp(pMysql, fundBindSpVec, trade_id,false))
	{
		TRACE_DEBUG("no fund bind sp record");
                
		return true;
	}

	return setFundBindAllSpToKV(trade_id, fundBindSpVec);
}

/**
 * �����û���ͨ����˾�˻��б�ckv����
 * @param list �󶨵Ļ���˾�б�
 * @param value ckv�д������ִ�ֵ
 * @return 0-�ɹ�������-ʧ��
 */
int packUsrBindSpCkvValue(const vector<FundBindSp>& list, string &value)
{
    CParams kvReqSet;
	char szParaName[64] = {0};
    
    //����Ҫ�޸ĵ�����szValue
    for(vector<FundBindSp>::size_type i= 0; i != list.size(); ++i)
    {
		const FundBindSp &fundBindSp = list[i];

		snprintf(szParaName, sizeof(szParaName), "Fimt_id_%zd", i);
		kvReqSet.setParam(szParaName, fundBindSp.Fimt_id);
		
		snprintf(szParaName, sizeof(szParaName), "Ftrade_id_%zd", i);
		kvReqSet.setParam(szParaName, fundBindSp.Ftrade_id);
		
		snprintf(szParaName, sizeof(szParaName), "Fspid_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fspid);
		
		snprintf(szParaName, sizeof(szParaName), "Fsp_user_id_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fsp_user_id);
		
		snprintf(szParaName, sizeof(szParaName), "Fsp_trans_id_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fsp_trans_id);

		snprintf(szParaName, sizeof(szParaName), "Facct_type_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Facct_type);

		snprintf(szParaName, sizeof(szParaName), "Fstandby1_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fstandby1);

		snprintf(szParaName, sizeof(szParaName), "Fmodify_time_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fmodify_time);
	
		snprintf(szParaName, sizeof(szParaName), "Fstate_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fstate);

		snprintf(szParaName, sizeof(szParaName), "Flstate_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Flstate);

		snprintf(szParaName, sizeof(szParaName), "Facc_time_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Facc_time);
		
		snprintf(szParaName, sizeof(szParaName), "Fcreate_time_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fcreate_time);
		
		snprintf(szParaName, sizeof(szParaName), "Fmemo_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fmemo);	

        snprintf(szParaName, sizeof(szParaName), "Fchannel_id_%zd", i);
		kvReqSet.setParam(szParaName,fundBindSp.Fchannel_id);
    }

	kvReqSet.setParam("total_num",(int)(list.size()));
    value = kvReqSet.pack();

    return 0;
}

bool setFundBindAllSpToKV(const string& trade_id, vector<FundBindSp>& fundBindSpVec)
{    
    //��״ckv��value
    string szValue;
	packUsrBindSpCkvValue(fundBindSpVec, szValue);

  	string key = trade_id;
    //��szValueд��ckv
	if(gCkvSvrOperator->set(CKV_KEY_TRADE_ID, key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool getDefaultSpFromKV(const string& trade_id, FundBindSp& data, vector<FundBindSp> & ckv_bindsp_list)
{
    memset(&data, 0, sizeof(data));	
    
    string key = trade_id;
    string ckv_value;
    int ret = gCkvSvrOperator->get(key, ckv_value);
    if (0 != ret)
        return false;

    //��ckv��ѯ�û��󶨵����л���˾�б�
	if(!parseUsrBindSpCkvValue(ckv_value, ckv_bindsp_list))
	{
		return false;
	}

    return getDefaultSp(data, ckv_bindsp_list);
}

bool getFundBindAllSpFromKV(const string& trade_id, vector<FundBindSp>& fundBindSpVec)
{   
    string key = trade_id;
    string ckv_value;
    int ret = gCkvSvrOperator->get(key, ckv_value);
    if (0 != ret)
        return false;

    //��ckv��ѯ�û��󶨵����л���˾�б�
	return parseUsrBindSpCkvValue(ckv_value, fundBindSpVec);
}

bool getDefaultSp(FundBindSp& def_sp, const vector<FundBindSp> & bindsp_list)
{
	for(vector<FundBindSp>::const_iterator iter = bindsp_list.begin(); iter != bindsp_list.end(); ++iter)
	{
		if(BIND_SPACC_SUC == (*iter).Fstate && LSTATE_INVALID != (*iter).Flstate  && BIND_SPACC_MASTER == (*iter).Facct_type)
		{
			def_sp = (*iter);
			return true;
		}

	}

	return false;
}

bool parseUsrBindSpCkvValue(const string& value, vector<FundBindSp>& ckv_bindsp_list)
{
	CParams kvRspGet;
    kvRspGet.parse(value);

	if( !kvRspGet.isExists("total_num"))
	{
		return false;
	}

	int total_num = atoi(kvRspGet.getString("total_num").c_str());

	for(int i = 0; i < total_num; i++)
	{
		FundBindSp fundBindSp;
        memset(&fundBindSp, 0 , sizeof(fundBindSp));

		fundBindSp.Fimt_id = kvRspGet.getLong(("Fimt_id_"+toString(i)).c_str());
		strncpy(fundBindSp.Ftrade_id, kvRspGet["Ftrade_id_"+toString(i)], sizeof(fundBindSp.Ftrade_id) - 1);
		strncpy(fundBindSp.Fspid, kvRspGet["Fspid_"+toString(i)], sizeof(fundBindSp.Fspid) - 1);
		strncpy(fundBindSp.Fsp_user_id, kvRspGet["Fsp_user_id_"+toString(i)], sizeof(fundBindSp.Fsp_user_id) - 1);
		strncpy(fundBindSp.Fsp_trans_id, kvRspGet["Fsp_trans_id_"+toString(i)], sizeof(fundBindSp.Fsp_trans_id) - 1);
		fundBindSp.Facct_type = kvRspGet.getInt(("Facct_type_"+toString(i)).c_str());
		fundBindSp.Fstate= kvRspGet.getInt(("Fstate_"+toString(i)).c_str());
		fundBindSp.Flstate = kvRspGet.getInt(("Flstate_"+toString(i)).c_str());
		strncpy(fundBindSp.Facc_time, kvRspGet["Facc_time_"+toString(i)], sizeof(fundBindSp.Facc_time) - 1);
		strncpy(fundBindSp.Fcreate_time, kvRspGet["Fcreate_time_"+toString(i)], sizeof(fundBindSp.Fcreate_time) - 1);
		strncpy(fundBindSp.Fmodify_time, kvRspGet["Fmodify_time_"+toString(i)], sizeof(fundBindSp.Fmodify_time) - 1);
		strncpy(fundBindSp.Fmemo, kvRspGet["Fmemo_"+toString(i)], sizeof(fundBindSp.Fmemo) - 1);
		fundBindSp.Fstandby1 = kvRspGet.getInt(("Fstandby1_"+toString(i)).c_str());
        strncpy(fundBindSp.Fchannel_id, kvRspGet["Fchannel_id_"+toString(i)], sizeof(fundBindSp.Fchannel_id) - 1);
		
		ckv_bindsp_list.push_back(fundBindSp);
	}

	return true;	
}

/**
* input set_type �������� 1:�û���������
*/
/*
void changeDefaultTradeAcc(CMySQL* pMysql,string trade_id, string new_spid, string old_spid, string systime, int set_type)
{
	if(trade_id.empty() || new_spid.empty() || (DEFAULT_SP_SET_TYPE == set_type && old_spid.empty()))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "trade_id or new_spid or old_spid empty.");    
	}
	
    //��ѯ��ǰĬ�ϻ���
    FundBindSp master_fund_bind_sp_acc;
	vector<FundBindSp> fundBindSpVec;

    if(getDefaultSpFromKV(trade_id, master_fund_bind_sp_acc,fundBindSpVec)) 
    {		
		if(!old_spid.empty() && old_spid != master_fund_bind_sp_acc.Fspid)
		{
			//���������ʺ�
			TRACE_ERROR("the sp account not the master account.orig spid=[%s]", old_spid.c_str());
			throw EXCEPTION(ERR_SP_ACC_NOT_MASTER_ACC, "the sp account not the master account.");
		}

		if(master_fund_bind_sp_acc.Fstandby1 == DEFAULT_SP_SET_TYPE && set_type != DEFAULT_SP_SET_TYPE)
		{
			return; //�����û��������õ�Ĭ�ϻ��𣬱������û������ſ�������
		}

		if(new_spid == master_fund_bind_sp_acc.Fspid && (set_type == 0 || (set_type == DEFAULT_SP_SET_TYPE && master_fund_bind_sp_acc.Fstandby1 == 1)))
		{
			return ; //�Ѿ���Ĭ�ϻ�����ֱ�ӷ���
		}

    }   //û�鵽Ҳ���������ݰɷ�ֹ����

    //���
    checkValidFundBindSp(fundBindSpVec,new_spid);
    
    //����ckv����
    if(!setFundBindAllSpToKV(trade_id, fundBindSpVec, new_spid, set_type, systime))
    {
		throw EXCEPTION(ERR_CHANGE_DEFAULT_SP_FAIL, "failed to set default sp.");
    }
	
	TRACE_DEBUG("UpdateDefaultTradeAcc success,tradeid=%s,new_spid=%s",trade_id.c_str(), new_spid.c_str());
 
    return;
}
*/

