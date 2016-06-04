#include "db_fund_sp_config.h"
#include "fund_commfunc.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 
extern CMySQL* gPtrFundDB;



/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundSpRedomTotal(CMySQL* pMysql, FundSpConfig& data, LONG& total_fee ,int userLoading,const string& acc_time)
{
	updateFundSpRedom(pMysql,data,total_fee,total_fee,userLoading,acc_time);
}

/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundSpRedom(CMySQL* pMysql, FundSpConfig& data, LONG& total_fee, LONG& redemScope ,int userLoading,const string& acc_time)
{

    if (acc_time.length() != 19)
    {
         throw CException(ERR_BAD_PARAM, string("updateFundSpRedomTotal  invalid acc_time=")+acc_time, __FILE__, __LINE__);
    }

    bool needUpdateCkv = false;
	// 计算当前D日:0点到24点的自然日
    string dDay = acc_time.substr(0,4)+acc_time.substr(5,2)+acc_time.substr(8,2);
    string redemDay = dDay;
    LONG redemFee = 0;
    if (redemDay <data.Fredem_day) //异常情况
    {
        redemDay = data.Fredem_day;
        redemFee = data.Fredem_total_day;
    }
    else if (redemDay == data.Fredem_day) //累加日赎回金额
    {
        redemFee = data.Fredem_total_day+total_fee;
    }
    else   //昨日的清0
    {
        redemFee = total_fee;
    }

    //LONG spTotalBalance = QuerySpTotalBalance(pMysql,addDays(redemDay,-1), data.Fspid); 
    LONG spTotalBalance = QuerySpTotalBalance(pMysql, data.Fspid); 
    
    // 构造SQL
    string querySql = "UPDATE fund_db.t_fund_sp_config SET ";
	// 统计垫支赎回金额
    if (userLoading == DRAW_USE_LOADING)
    {
        querySql += string(" Fredem_total = Fredem_total+")+toString(total_fee)+" ,"; //TODO 大金额验证
    }
	//赎回限制
    int rate = (gPtrConfig->m_spTransferRate[data.Fspid]>0?gPtrConfig->m_spTransferRate[data.Fspid]:gPtrConfig->m_AppCfg.sp_redem_rate);
    if (spTotalBalance<=(redemFee)*rate+gPtrConfig->m_AppCfg.sp_redem_tranfer_limit_offset) 
    {
        data.Fredem_valid=(data.Fredem_valid|SP_STOP_TRANSFER);
        querySql += string(" Fredem_valid = ")+toString(data.Fredem_valid)+" ,"; 
        needUpdateCkv = true;
    }
    else if (data.Fredem_valid&SP_STOP_TRANSFER)
    {
        data.Fredem_valid=(data.Fredem_valid&(~SP_STOP_TRANSFER));
        querySql += string(" Fredem_valid = ")+toString(data.Fredem_valid)+" ,"; 
        needUpdateCkv = true;
    }

    //统计scope
    if(data.Fscope_upper_limit != 0)
    {
        querySql += string(" Fscope = Fscope -")+toString(redemScope)+" ,"; 
    }
    // 统计T日赎回总额, 用于计算T日净申购
    if(data.Fbuyfee_tday_limit!=0&&(data.Fstat_flag&SPCONFIG_STAT_NET)){
        string statDay=( SPCONFIG_STAT_DDAY & data.Fstat_flag )? dDay:getCacheTradeDate(gPtrFundSlaveDB,acc_time);
	    if(data.Fstat_redeem_tdate< statDay)
	    {
	        strncpy(data.Fstat_redeem_tdate,statDay.c_str(),sizeof(data.Fstat_redeem_tdate)-1);
	        data.Ftotal_redeem_tday = total_fee;			
    		querySql += string(" Fstat_redeem_tdate = ")+statDay+" ,";
    		querySql += string(" Ftotal_redeem_tday = ")+toString(data.Ftotal_redeem_tday)+" ,";
	    }else if(data.Fstat_redeem_tdate==statDay){
	        data.Ftotal_redeem_tday = data.Ftotal_redeem_tday+total_fee;
    		querySql += string(" Ftotal_redeem_tday = ")+toString(data.Ftotal_redeem_tday)+" ,";
	    }
    }
    querySql += string(" Fredem_total_day = ")+toString(redemFee)+" ,"; 
    querySql += string(" Fredem_day = '")+redemDay+"' ,"; 
    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   

    querySql += string(" WHERE Fspid='")+pMysql->EscapeStr(data.Fspid)+"' ";   
    querySql += string(" AND Ffund_code='")+pMysql->EscapeStr(data.Ffund_code)+"' ";
    

    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str() );
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }

    if (needUpdateCkv)
    {
        setAllSupportSpConfig(pMysql);
    }
}



/*
*query函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
bool sumRedomTotal(CMySQL* pMysql, LONG& allRedomTotal)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Fredem_total) "
                    " FROM fund_db.t_fund_sp_config "
                    " WHERE "
                    " Flstate <> %d " ,
                    LSTATE_INVALID 
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
			allRedomTotal = row[0] ? atoll(row[0]) : 0;
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


bool queryFundSpAndFundcodeConfig(CMySQL* pMysql, FundSpConfig& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fspid,Fsp_name,Ffund_code,Ffund_name,Flstate, "
                    " Fpurpose,Fbind_valid,Fbuy_valid,Fredem_valid,Fredem_total, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fstandby1,Fstandby2, "
                    " Fstandby3,Fstandby4,Fstandby5,Fstandby6,Fdebt_charge_bankid, "
                    " Fcurtype,Fsp_chargenum,Fcft_chargenum,Fcharge_bankid,Ftplus_redem_spid, "
                    " Frealtime_redem_spid,Ftotal_charge_spid,Fredem_day,Fredem_total_day,Fredem_exflag, "
                    " Fchange_charge_spid,Ftype,Fclose_flag,Ftransfer_flag,Ffirst_settlement_date, "
                    " Fnormal_settlement_date,Fend_type,Fscope_upper_limit,Fscope_lower_limit,Fscope, "
                    " Fstart_date,Fend_date,Fduration_type,Fduration,Frestrict_mode, "
                    " Frestrict_num,Fstate,Fbuy_first_lower_limit,Fbuy_lower_limit,Fbuy_add_limit, "
                    " Fstat_buy_tdate,Ftotal_buyfee_tday,Fbuyfee_tday_limit,Fbuyfee_tday_limit_offset,Fscope_upper_limit_offset, "
                    " Fstat_redeem_tdate,Ftotal_redeem_tday,Frisk_ass_flag,Frisk_type,Fbuy_confirm_type,Fstat_flag,Fsave_sett_time,Ffetch_sett_time, "
					" Fsp_full_name,Ffund_brief_name "
                    " FROM fund_db.t_fund_sp_config "
                    " WHERE "
                    " Fspid='%s'  AND " 
                    " Ffund_code='%s'  AND " 
                    " Flstate <> %d " 
                    " %s ",
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    LSTATE_INVALID,
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for (int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Fspid,row[0] ? row[0] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsp_name,row[1] ? row[1] : "", sizeof(data.Fsp_name) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ffund_name,row[3] ? row[3] : "", sizeof(data.Ffund_name) - 1);
            data.Flstate = row[4] ? atoi(row[4]) : 0;
            data.Fpurpose = row[5] ? atoi(row[5]) : 0;
            data.Fbind_valid = row[6] ? atoi(row[6]) : 0;
            data.Fbuy_valid = row[7] ? atoi(row[7]) : 0;
            data.Fredem_valid = row[8] ? atoi(row[8]) : 0;
            data.Fredem_total = row[9] ? atoll(row[9]) : 0;
            strncpy(data.Fcreate_time,row[10] ? row[10] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[11] ? row[11] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[12] ? row[12] : "", sizeof(data.Fmemo) - 1);
            data.Fstandby1 = row[13] ? atoi(row[13]) : 0;
            data.Fstandby2 = row[14] ? atoi(row[14]) : 0;
            strncpy(data.Fstandby3,row[15] ? row[15] : "", sizeof(data.Fstandby3) - 1);
            strncpy(data.Fstandby4,row[16] ? row[16] : "", sizeof(data.Fstandby4) - 1);
            strncpy(data.Fstandby5,row[17] ? row[17] : "", sizeof(data.Fstandby5) - 1);
            strncpy(data.Fstandby6,row[18] ? row[18] : "", sizeof(data.Fstandby6) - 1);
            strncpy(data.Fdebt_charge_bankid,row[19] ? row[19] : "", sizeof(data.Fdebt_charge_bankid) - 1);
            data.Fcurtype = row[20] ? atoi(row[20]) : 0;
            data.Fsp_chargenum = row[21] ? atoll(row[21]) : 0;
            data.Fcft_chargenum = row[22] ? atoll(row[22]) : 0;
            strncpy(data.Fcharge_bankid,row[23] ? row[23] : "", sizeof(data.Fcharge_bankid) - 1);
            strncpy(data.Ftplus_redem_spid,row[24] ? row[24] : "", sizeof(data.Ftplus_redem_spid) - 1);
            strncpy(data.Frealtime_redem_spid,row[25] ? row[25] : "", sizeof(data.Frealtime_redem_spid) - 1);
            strncpy(data.Ftotal_charge_spid,row[26] ? row[26] : "", sizeof(data.Ftotal_charge_spid) - 1);
            strncpy(data.Fredem_day,row[27] ? row[27] : "", sizeof(data.Fredem_day) - 1);
            data.Fredem_total_day = row[28] ? atoll(row[28]) : 0;
            data.Fredem_exflag = row[29] ? atoi(row[29]) : 0;
            strncpy(data.Fchange_charge_spid,row[30] ? row[30] : "", sizeof(data.Fchange_charge_spid) - 1);
            data.Ftype = row[31] ? atoi(row[31]) : 0;
            data.Fclose_flag = row[32] ? atoi(row[32]) : 0;
            data.Ftransfer_flag = row[33] ? atoi(row[33]) : 0;
            data.Ffirst_settlement_date = row[34] ? atoi(row[34]) : 0;
            data.Fnormal_settlement_date = row[35] ? atoi(row[35]) : 0;
            data.Fend_type = row[36] ? atoi(row[36]) : 0;
            data.Fscope_upper_limit = row[37] ? atoll(row[37]) : 0;
            data.Fscope_lower_limit = row[38] ? atoll(row[38]) : 0;
            data.Fscope = row[39] ? atoll(row[39]) : 0;
            strncpy(data.Fstart_date,row[40] ? row[40] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[41] ? row[41] : "", sizeof(data.Fend_date) - 1);
            data.Fduration_type = row[42] ? atoi(row[42]) : 0;
            data.Fduration = row[43] ? atoi(row[43]) : 0;
            data.Frestrict_mode = row[44] ? atoi(row[44]) : 0;
            data.Frestrict_num = row[45] ? atoi(row[45]) : 0;
            data.Fstate = row[46] ? atoi(row[46]) : 0;
            data.Fbuy_first_lower_limit = row[47] ? atoll(row[47]) : 0;
            data.Fbuy_lower_limit = row[48] ? atoll(row[48]) : 0;
            data.Fbuy_add_limit= row[49] ? atoll(row[49]) : 0;
            strncpy(data.Fstat_buy_tdate,row[50] ? row[50] : "", sizeof(data.Fstat_buy_tdate) - 1);
            data.Ftotal_buyfee_tday= row[51] ? atoll(row[51]) : 0;
            data.Fbuyfee_tday_limit= row[52] ? atoll(row[52]) : 0;
            data.Fbuyfee_tday_limit_offset= row[53] ? atoll(row[53]) : 0;
            data.Fscope_upper_limit_offset= row[54] ? atoll(row[54]) : 0;
            strncpy(data.Fstat_redeem_tdate,row[55] ? row[55] : "", sizeof(data.Fstat_redeem_tdate) - 1);
            data.Ftotal_redeem_tday= row[56] ? atoll(row[56]) : 0;
			data.Frisk_ass_flag = row[57] ? atoi(row[57]) : 0;
			data.Frisk_type = row[58] ? atoi(row[58]) : 0;
			data.Fbuy_confirm_type = row[59] ? atoi(row[59]) : 0;
			data.Fstat_flag = row[60]?atoi(row[60]) : 0;
			data.Fsave_sett_time = row[61]?atoi(row[61]) : 0;
			data.Ffetch_sett_time = row[62]?atoi(row[62]) : 0;
			strncpy(data.Fsp_full_name,row[63] ? row[63] : "", sizeof(data.Fsp_full_name) - 1);
			strncpy(data.Ffund_brief_name,row[64] ? row[64] : "", sizeof(data.Ffund_brief_name) - 1);
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


bool queryFundSpAllConfig(CMySQL* pMysql, vector<FundSpConfig>& dataVec,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fspid,Fsp_name,Ffund_code,Ffund_name,Flstate, "
                    " Fpurpose,Fbind_valid,Fbuy_valid,Fredem_valid,Fredem_total, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fstandby1,Fstandby2, "
                    " Fstandby3,Fstandby4,Fstandby5,Fstandby6,Fdebt_charge_bankid, "
                    " Fcurtype,Fsp_chargenum,Fcft_chargenum,Fcharge_bankid,Ftplus_redem_spid, "
                    " Frealtime_redem_spid,Ftotal_charge_spid,Fredem_day,Fredem_total_day,Fredem_exflag, "
                    " Fchange_charge_spid,Ftype,Fclose_flag,Ftransfer_flag,Ffirst_settlement_date, "
                    " Fnormal_settlement_date,Fend_type,Fscope_upper_limit,Fscope_lower_limit,Fscope, "
                    " Fstart_date,Fend_date,Fduration_type,Fduration,Frestrict_mode, "
                    " Frestrict_num,Fstate,Fbuy_first_lower_limit,Fbuy_lower_limit,Fbuy_add_limit, "
                    " Fstat_buy_tdate,Ftotal_buyfee_tday,Fbuyfee_tday_limit,Fbuyfee_tday_limit_offset,Fscope_upper_limit_offset,Fsp_full_name,Ffund_brief_name, "
                    " Fstat_redeem_tdate,Ftotal_redeem_tday,Frisk_ass_flag,Frisk_type,Fbuy_confirm_type,Fstat_flag,Fsave_sett_time,Ffetch_sett_time "
                    " FROM fund_db.t_fund_sp_config "
                    " %s ",
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 )
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
			FundSpConfig data;
			memset(&data, 0,sizeof(FundSpConfig));
			
            strncpy(data.Fspid,row[0] ? row[0] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsp_name,row[1] ? row[1] : "", sizeof(data.Fsp_name) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ffund_name,row[3] ? row[3] : "", sizeof(data.Ffund_name) - 1);
            data.Flstate = row[4] ? atoi(row[4]) : 0;
            data.Fpurpose = row[5] ? atoi(row[5]) : 0;
            data.Fbind_valid = row[6] ? atoi(row[6]) : 0;
            data.Fbuy_valid = row[7] ? atoi(row[7]) : 0;
            data.Fredem_valid = row[8] ? atoi(row[8]) : 0;
            data.Fredem_total = row[9] ? atoll(row[9]) : 0;
            strncpy(data.Fcreate_time,row[10] ? row[10] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[11] ? row[11] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[12] ? row[12] : "", sizeof(data.Fmemo) - 1);
            data.Fstandby1 = row[13] ? atoi(row[13]) : 0;
            data.Fstandby2 = row[14] ? atoi(row[14]) : 0;
            strncpy(data.Fstandby3,row[15] ? row[15] : "", sizeof(data.Fstandby3) - 1);
            strncpy(data.Fstandby4,row[16] ? row[16] : "", sizeof(data.Fstandby4) - 1);
            strncpy(data.Fstandby5,row[17] ? row[17] : "", sizeof(data.Fstandby5) - 1);
            strncpy(data.Fstandby6,row[18] ? row[18] : "", sizeof(data.Fstandby6) - 1);
            strncpy(data.Fdebt_charge_bankid,row[19] ? row[19] : "", sizeof(data.Fdebt_charge_bankid) - 1);
            data.Fcurtype = row[20] ? atoi(row[20]) : 0;
            data.Fsp_chargenum = row[21] ? atoll(row[21]) : 0;
            data.Fcft_chargenum = row[22] ? atoll(row[22]) : 0;
            strncpy(data.Fcharge_bankid,row[23] ? row[23] : "", sizeof(data.Fcharge_bankid) - 1);
            strncpy(data.Ftplus_redem_spid,row[24] ? row[24] : "", sizeof(data.Ftplus_redem_spid) - 1);
            strncpy(data.Frealtime_redem_spid,row[25] ? row[25] : "", sizeof(data.Frealtime_redem_spid) - 1);
            strncpy(data.Ftotal_charge_spid,row[26] ? row[26] : "", sizeof(data.Ftotal_charge_spid) - 1);
            strncpy(data.Fredem_day,row[27] ? row[27] : "", sizeof(data.Fredem_day) - 1);
            data.Fredem_total_day = row[28] ? atoll(row[28]) : 0;
            data.Fredem_exflag = row[29] ? atoi(row[29]) : 0;
            strncpy(data.Fchange_charge_spid,row[30] ? row[30] : "", sizeof(data.Fchange_charge_spid) - 1);
            data.Ftype = row[31] ? atoi(row[31]) : 0;
            data.Fclose_flag = row[32] ? atoi(row[32]) : 0;
            data.Ftransfer_flag = row[33] ? atoi(row[33]) : 0;
            data.Ffirst_settlement_date = row[34] ? atoi(row[34]) : 0;
            data.Fnormal_settlement_date = row[35] ? atoi(row[35]) : 0;
            data.Fend_type = row[36] ? atoi(row[36]) : 0;
            data.Fscope_upper_limit = row[37] ? atoll(row[37]) : 0;
            data.Fscope_lower_limit = row[38] ? atoll(row[38]) : 0;
            data.Fscope = row[39] ? atoll(row[39]) : 0;
            strncpy(data.Fstart_date,row[40] ? row[40] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[41] ? row[41] : "", sizeof(data.Fend_date) - 1);
            data.Fduration_type = row[42] ? atoi(row[42]) : 0;
            data.Fduration = row[43] ? atoi(row[43]) : 0;
            data.Frestrict_mode = row[44] ? atoi(row[44]) : 0;
            data.Frestrict_num = row[45] ? atoi(row[45]) : 0;
            data.Fstate = row[46] ? atoi(row[46]) : 0;
			data.Fbuy_first_lower_limit = row[47] ? atoll(row[47]) : 0;
			data.Fbuy_lower_limit = row[48] ? atoll(row[48]) : 0;
			data.Fbuy_add_limit= row[49] ? atoll(row[49]) : 0;
            strncpy(data.Fstat_buy_tdate,row[50] ? row[50] : "", sizeof(data.Fstat_buy_tdate) - 1);
            data.Ftotal_buyfee_tday= row[51] ? atoll(row[51]) : 0;
            data.Fbuyfee_tday_limit= row[52] ? atoll(row[52]) : 0;
            data.Fbuyfee_tday_limit_offset= row[53] ? atoll(row[53]) : 0;
            data.Fscope_upper_limit_offset= row[54] ? atoll(row[54]) : 0;
	      strncpy(data.Fsp_full_name,row[55] ? row[55] : "", sizeof(data.Fsp_full_name) - 1);
	      strncpy(data.Ffund_brief_name,row[56] ? row[56] : "", sizeof(data.Ffund_brief_name) - 1);
            strncpy(data.Fstat_redeem_tdate,row[57] ? row[57] : "", sizeof(data.Fstat_redeem_tdate) - 1);
            data.Ftotal_redeem_tday= row[58] ? atoll(row[58]) : 0;
			data.Frisk_ass_flag = row[59] ? atoi(row[59]) : 0;
			data.Frisk_type = row[60] ? atoi(row[60]) : 0;
			data.Fbuy_confirm_type = row[61] ? atoi(row[61]) : 0;
			data.Fstat_flag = row[62]?atoi(row[62]) : 0;
			data.Fsave_sett_time = row[63]?atoi(row[63]) : 0;
			data.Ffetch_sett_time = row[64]?atoi(row[64]) : 0;
			dataVec.push_back(data);

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

bool queryFundSpConfig(CMySQL* pMysql, vector<FundSpConfig>& dataVec, const string& spid, bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fspid,Fsp_name,Ffund_code,Ffund_name,Flstate, "
                    " Fpurpose,Fbind_valid,Fbuy_valid,Fredem_valid,Fredem_total, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fstandby1,Fstandby2, "
                    " Fstandby3,Fstandby4,Fstandby5,Fstandby6,Fdebt_charge_bankid, "
                    " Fcurtype,Fsp_chargenum,Fcft_chargenum,Fcharge_bankid,Ftplus_redem_spid, "
                    " Frealtime_redem_spid,Ftotal_charge_spid,Fredem_day,Fredem_total_day,Fredem_exflag, "
                    " Fchange_charge_spid,Ftype,Fclose_flag,Ftransfer_flag,Ffirst_settlement_date, "
                    " Fnormal_settlement_date,Fend_type,Fscope_upper_limit,Fscope_lower_limit,Fscope, "
                    " Fstart_date,Fend_date,Fduration_type,Fduration,Frestrict_mode, "
                    " Frestrict_num,Fstate,Fbuy_first_lower_limit,Fbuy_lower_limit,Fbuy_add_limit, "
                    " Fstat_buy_tdate,Ftotal_buyfee_tday,Fbuyfee_tday_limit,Fbuyfee_tday_limit_offset,Fscope_upper_limit_offset, "
                    " Fstat_redeem_tdate,Ftotal_redeem_tday,Frisk_ass_flag,Frisk_type,Fbuy_confirm_type,Fstat_flag,Fsave_sett_time,Ffetch_sett_time, "
					" Fsp_full_name,Ffund_brief_name "
                    " FROM fund_db.t_fund_sp_config "
                    " WHERE Fspid = '%s' AND "
                    " Flstate <> %d " 
                    " %s ",
                    pMysql->EscapeStr(spid).c_str(),
                    LSTATE_INVALID,
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 )
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
			FundSpConfig data;
			
            strncpy(data.Fspid,row[0] ? row[0] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsp_name,row[1] ? row[1] : "", sizeof(data.Fsp_name) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ffund_name,row[3] ? row[3] : "", sizeof(data.Ffund_name) - 1);
            data.Flstate = row[4] ? atoi(row[4]) : 0;
            data.Fpurpose = row[5] ? atoi(row[5]) : 0;
            data.Fbind_valid = row[6] ? atoi(row[6]) : 0;
            data.Fbuy_valid = row[7] ? atoi(row[7]) : 0;
            data.Fredem_valid = row[8] ? atoi(row[8]) : 0;
            data.Fredem_total = row[9] ? atoll(row[9]) : 0;
            strncpy(data.Fcreate_time,row[10] ? row[10] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[11] ? row[11] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[12] ? row[12] : "", sizeof(data.Fmemo) - 1);
            data.Fstandby1 = row[13] ? atoi(row[13]) : 0;
            data.Fstandby2 = row[14] ? atoi(row[14]) : 0;
            strncpy(data.Fstandby3,row[15] ? row[15] : "", sizeof(data.Fstandby3) - 1);
            strncpy(data.Fstandby4,row[16] ? row[16] : "", sizeof(data.Fstandby4) - 1);
            strncpy(data.Fstandby5,row[17] ? row[17] : "", sizeof(data.Fstandby5) - 1);
            strncpy(data.Fstandby6,row[18] ? row[18] : "", sizeof(data.Fstandby6) - 1);
            strncpy(data.Fdebt_charge_bankid,row[19] ? row[19] : "", sizeof(data.Fdebt_charge_bankid) - 1);
            data.Fcurtype = row[20] ? atoi(row[20]) : 0;
            data.Fsp_chargenum = row[21] ? atoll(row[21]) : 0;
            data.Fcft_chargenum = row[22] ? atoll(row[22]) : 0;
            strncpy(data.Fcharge_bankid,row[23] ? row[23] : "", sizeof(data.Fcharge_bankid) - 1);
            strncpy(data.Ftplus_redem_spid,row[24] ? row[24] : "", sizeof(data.Ftplus_redem_spid) - 1);
            strncpy(data.Frealtime_redem_spid,row[25] ? row[25] : "", sizeof(data.Frealtime_redem_spid) - 1);
            strncpy(data.Ftotal_charge_spid,row[26] ? row[26] : "", sizeof(data.Ftotal_charge_spid) - 1);
            strncpy(data.Fredem_day,row[27] ? row[27] : "", sizeof(data.Fredem_day) - 1);
            data.Fredem_total_day = row[28] ? atoll(row[28]) : 0;
            data.Fredem_exflag = row[29] ? atoi(row[29]) : 0;
            strncpy(data.Fchange_charge_spid,row[30] ? row[30] : "", sizeof(data.Fchange_charge_spid) - 1);
            data.Ftype = row[31] ? atoi(row[31]) : 0;
            data.Fclose_flag = row[32] ? atoi(row[32]) : 0;
            data.Ftransfer_flag = row[33] ? atoi(row[33]) : 0;
            data.Ffirst_settlement_date = row[34] ? atoi(row[34]) : 0;
            data.Fnormal_settlement_date = row[35] ? atoi(row[35]) : 0;
            data.Fend_type = row[36] ? atoi(row[36]) : 0;
            data.Fscope_upper_limit = row[37] ? atoll(row[37]) : 0;
            data.Fscope_lower_limit = row[38] ? atoll(row[38]) : 0;
            data.Fscope = row[39] ? atoll(row[39]) : 0;
            strncpy(data.Fstart_date,row[40] ? row[40] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[41] ? row[41] : "", sizeof(data.Fend_date) - 1);
            data.Fduration_type = row[42] ? atoi(row[42]) : 0;
            data.Fduration = row[43] ? atoi(row[43]) : 0;
            data.Frestrict_mode = row[44] ? atoi(row[44]) : 0;
            data.Frestrict_num = row[45] ? atoi(row[45]) : 0;
            data.Fstate = row[46] ? atoi(row[46]) : 0;
			data.Fbuy_first_lower_limit = row[47] ? atoll(row[47]) : 0;
			data.Fbuy_lower_limit = row[48] ? atoll(row[48]) : 0;
			data.Fbuy_add_limit= row[49] ? atoll(row[49]) : 0;
            strncpy(data.Fstat_buy_tdate,row[50] ? row[50] : "", sizeof(data.Fstat_buy_tdate) - 1);
            data.Ftotal_buyfee_tday= row[51] ? atoll(row[51]) : 0;
            data.Fbuyfee_tday_limit= row[52] ? atoll(row[52]) : 0;
            data.Fbuyfee_tday_limit_offset= row[53] ? atoll(row[53]) : 0;
            data.Fscope_upper_limit_offset= row[54] ? atoll(row[54]) : 0;
            strncpy(data.Fstat_redeem_tdate,row[55] ? row[55] : "", sizeof(data.Fstat_redeem_tdate) - 1);
            data.Ftotal_redeem_tday= row[56] ? atoll(row[56]) : 0;
			data.Frisk_ass_flag = row[57] ? atoi(row[57]) : 0;
			data.Frisk_type = row[58] ? atoi(row[58]) : 0;
			data.Fbuy_confirm_type = row[59] ? atoi(row[59]) : 0;
			data.Fstat_flag = row[60]?atoi(row[60]) : 0;
			data.Fsave_sett_time = row[61]?atoi(row[61]) : 0;
			data.Ffetch_sett_time = row[62]?atoi(row[62]) : 0;
			strncpy(data.Fsp_full_name,row[63] ? row[63] : "", sizeof(data.Fsp_full_name) - 1);
			strncpy(data.Ffund_brief_name,row[64] ? row[64] : "", sizeof(data.Ffund_brief_name) - 1);
			
			dataVec.push_back(data);

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

bool queryFundSpAndFundcodeFromCkv(FundSpConfig& data, bool throwExp)
{
	string key = "spid_fundcode_conf_"+ toString(data.Fspid) + "_" + toString(data.Ffund_code);

	//取kv数据
	CParams kvRspGet;
	if(gCkvSvrOperator->get(key, kvRspGet))
	{
		if(throwExp)
		{
			throw EXCEPTION(ERR_QUERY_FUND_CODE_CONFIG, "query spid and fund_code from ckv failure.");    
		}
		return false;
	}

	strncpy(data.Fsp_name, kvRspGet.getString("Fsp_name").c_str(), sizeof(data.Fsp_name) - 1);
	strncpy(data.Ffund_name, kvRspGet.getString("Ffund_name").c_str(), sizeof(data.Ffund_name) - 1);
	data.Fclose_flag = kvRspGet.getInt("Fclose_flag");
	data.Ftransfer_flag = kvRspGet.getInt("Ftransfer_flag");
	data.Flstate = kvRspGet.getInt("Flstate");
	data.Fbuy_valid = kvRspGet.getInt("Fbuy_valid");
	data.Fredem_valid = kvRspGet.getInt("Fredem_valid");
	data.Fend_type = kvRspGet.getInt("Fend_type");
	data.Fscope_upper_limit = kvRspGet.getLong("Fscope_upper_limit");
	data.Fscope_lower_limit = kvRspGet.getLong("Fscope_lower_limit");
	data.Fduration_type = kvRspGet.getInt("Fduration_type");
	data.Fduration = kvRspGet.getInt("Fduration");
	data.Frestrict_mode = kvRspGet.getInt("Frestrict_mode");
	data.Frestrict_num = kvRspGet.getInt("Frestrict_num");
	data.Fstate = kvRspGet.getInt("Fstate");
	data.Fbuy_first_lower_limit = kvRspGet.getLong("Fbuy_first_lower_limit");
	data.Fbuy_lower_limit = kvRspGet.getLong("Fbuy_lower_limit");
	data.Fbuy_add_limit = kvRspGet.getLong("Fbuy_add_limit");
	data.Fcurtype = kvRspGet.getInt("Fcurtype");
	strncpy(data.Fsp_full_name, kvRspGet.getString("Fsp_full_name").c_str(), sizeof(data.Fsp_full_name) - 1);
	strncpy(data.Ffund_brief_name, kvRspGet.getString("Ffund_brief_name").c_str(), sizeof(data.Ffund_brief_name) - 1);
	data.Ftype = kvRspGet.getInt("Ftype");
	data.Fbuy_confirm_type = kvRspGet.getInt("Fbuy_confirm_type");
	data.Fstat_flag = kvRspGet.getInt("Fstat_flag");
	data.Ffetch_sett_time= kvRspGet.getInt("Ffetch_sett_time");	
	data.Fbuyfee_tday_limit= kvRspGet.getLong("Fbuyfee_tday_limit");
	data.Frisk_ass_flag= kvRspGet.getLong("Frisk_ass_flag");
	data.Frisk_type= kvRspGet.getLong("Frisk_type");

	return true;
}


/**
* 检查基金公司及基金代码是否存在，如果只传入spid 则取第一条有效的fund_code
* 先查询CKV，再查DB
* 必传:spid
* 可传:fund_code
*/
void checkSpidAndFundcode(CMySQL* pMysql, FundSpConfig& data)
{
	char errMsg[128]={0};
	// fund_code存在
	if(0 != strcmp("", data.Ffund_code)&&0 != strcmp("", data.Fspid))
	{
		// 查询DB
		if( queryFundSpAndFundcodeConfig(pMysql,data,false))
		{
			return;
		}else{
			snprintf(errMsg,sizeof(errMsg),"商户DB配置不存在.spid[%s]fundcode[%s]",data.Fspid,data.Ffund_code);
			throw EXCEPTION(ERR_BAD_PARAM, errMsg); 
		}
	}
	
	// 只根据商户号查询DB
	vector<FundSpConfig> fundSpConfigVec;
	if( !queryFundSpConfig(pMysql, fundSpConfigVec, toString(data.Fspid), false))
	{
		snprintf(errMsg,sizeof(errMsg),"商户DB配置不存在.spid[%s]",data.Fspid);
		throw EXCEPTION(ERR_BAD_PARAM, errMsg);    
	}
	// 一个商户号有多个基金配置,告警
	if(fundSpConfigVec.size()>1)
	{
		snprintf(errMsg,sizeof(errMsg),"商户DB配置存在多个.spid[%s]count[%zd]",data.Fspid,fundSpConfigVec.size());
		alert(ERR_BAD_PARAM,errMsg);
	}
	// 选择第一个
	data=fundSpConfigVec[0];	
}

/**
* 检查基金公司及基金代码是否存在，如果只传入spid 则取第一条有效的fund_code
*/
void checkFundSpAndFundcode(CMySQL* pMysql, FundSpConfig& data, bool spMustValid)
{
	if(!(0 == strcmp("", data.Ffund_code)))
	{
		if(!queryFundSpAndFundcodeConfig(pMysql,data,false))
		{
			throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error");    
		}
	}
	else
	{
		vector<FundSpConfig> fundSpConfigVec;
		if( !queryFundSpConfig(pMysql, fundSpConfigVec, toString(data.Fspid), false))
		{
			TRACE_DEBUG("no fund sp config");
			throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error");    
		}

		for(vector<FundSpConfig>::size_type i= 0; i != fundSpConfigVec.size(); ++i)
		{
			FundSpConfig fundSpConfig = fundSpConfigVec[i];
			if(0 == strcmp(fundSpConfig.Fspid, data.Fspid) && fundSpConfig.Fpurpose== 1 && LSTATE_INVALID != fundSpConfig.Flstate)
			{
				if(spMustValid && fundSpConfig.Flstate != LSTATE_VALID)
				{
					continue; //要求基金公司必须是有效状态
				}

				data = fundSpConfig;
				/*
				strncpy(data.Fsp_name, fundSpConfig.Fsp_name, sizeof(data.Fsp_name) - 1);
				strncpy(data.Ffund_name,  fundSpConfig.Ffund_name, sizeof(data.Ffund_name) - 1);
				strncpy(data.Ffund_code,  fundSpConfig.Ffund_code, sizeof(data.Ffund_code) - 1);
				*/
				return;//找到一个即返回

			}
		}
		
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}

}

bool setSupportFundSpAndFundcode(FundSpConfig& data)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
	//string key = gPtrConfig->m_KvCfg.kvAddress + "spid_fundcode_conf_"+ toString(data.Fspid) + "_" + toString(data.Ffund_code);
	string key = "spid_fundcode_conf_"+ toString(data.Fspid) + "_" + toString(data.Ffund_code);

	string szValue;

	CParams kvReqSet;
	//设置要修改的数据szValue
	kvReqSet.setParam("Fspid",data.Fspid);
	kvReqSet.setParam("Fsp_name",data.Fsp_name);
	kvReqSet.setParam("Ffund_code",data.Ffund_code);
	kvReqSet.setParam("Ffund_name",data.Ffund_name);
	kvReqSet.setParam("Fpurpose",data.Fpurpose);
	kvReqSet.setParam("Flstate",data.Flstate);
	kvReqSet.setParam("Fclose_flag",data.Fclose_flag);
	kvReqSet.setParam("Ftransfer_flag",data.Ftransfer_flag);
	kvReqSet.setParam("Fbuy_valid",data.Fbuy_valid);
	kvReqSet.setParam("Fredem_valid",data.Fredem_valid);
	kvReqSet.setParam("Fend_type",data.Fend_type);
	kvReqSet.setParam("Fscope_upper_limit",data.Fscope_upper_limit);
	kvReqSet.setParam("Fscope_lower_limit",data.Fscope_lower_limit);
	kvReqSet.setParam("Fduration_type",data.Fduration_type);
	kvReqSet.setParam("Fduration",data.Fduration);
	kvReqSet.setParam("Frestrict_mode",data.Frestrict_mode);
	kvReqSet.setParam("Frestrict_num",data.Frestrict_num);
	kvReqSet.setParam("Fstate",data.Fstate);
	kvReqSet.setParam("Fbuy_first_lower_limit",data.Fbuy_first_lower_limit);
	kvReqSet.setParam("Fbuy_lower_limit",data.Fbuy_lower_limit);
	kvReqSet.setParam("Fbuy_add_limit",data.Fbuy_add_limit);
      kvReqSet.setParam("Fstat_buy_tdate",data.Fstat_buy_tdate);
      kvReqSet.setParam("Fcurtype",data.Fcurtype);
	  kvReqSet.setParam("Fsp_full_name",data.Fsp_full_name);
      kvReqSet.setParam("Ffund_brief_name",data.Ffund_brief_name);
      kvReqSet.setParam("Ftype",data.Ftype);
      kvReqSet.setParam("Fbuy_confirm_type",data.Fbuy_confirm_type);
      kvReqSet.setParam("Fstat_flag",data.Fstat_flag);
      kvReqSet.setParam("Fbuyfee_tday_limit",data.Fbuyfee_tday_limit);
      kvReqSet.setParam("Fsave_sett_time",data.Fsave_sett_time);
      kvReqSet.setParam("Ffetch_sett_time",data.Ffetch_sett_time);
      kvReqSet.setParam("Frisk_ass_flag",data.Frisk_ass_flag);
      kvReqSet.setParam("Frisk_type",data.Frisk_type);
      kvReqSet.setParam("Fstandby1",data.Fstandby1);
     
    szValue = kvReqSet.pack();

    //将szValue写入ckv
	if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_SPID_FUNDCODE_CONF), key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool setFundSpInfo(FundSpConfig& data)
{
	string key = "fund_spid_conf_"+ toString(data.Fspid);

	string szValue;

	CParams kvReqSet;
	//设置要修改的数据szValue
	kvReqSet.setParam("Fspid",data.Fspid);
	kvReqSet.setParam("Fsp_name",data.Fsp_name);
	kvReqSet.setParam("Ffund_code",data.Ffund_code);
	kvReqSet.setParam("Ffund_name",data.Ffund_name);
	kvReqSet.setParam("Fpurpose",data.Fpurpose);
	kvReqSet.setParam("Flstate",data.Flstate);
	kvReqSet.setParam("Fclose_flag",data.Fclose_flag);
	kvReqSet.setParam("Ftransfer_flag",data.Ftransfer_flag);
	kvReqSet.setParam("Fbuy_valid",data.Fbuy_valid);
	kvReqSet.setParam("Fredem_valid",data.Fredem_valid);
	kvReqSet.setParam("Fend_type",data.Fend_type);
	kvReqSet.setParam("Fscope_upper_limit",data.Fscope_upper_limit);
	kvReqSet.setParam("Fscope_lower_limit",data.Fscope_lower_limit);
	kvReqSet.setParam("Fduration_type",data.Fduration_type);
	kvReqSet.setParam("Fduration",data.Fduration);
	kvReqSet.setParam("Frestrict_mode",data.Frestrict_mode);
	kvReqSet.setParam("Frestrict_num",data.Frestrict_num);
	kvReqSet.setParam("Fstate",data.Fstate);
	kvReqSet.setParam("Fbuy_first_lower_limit",data.Fbuy_first_lower_limit);
	kvReqSet.setParam("Fbuy_lower_limit",data.Fbuy_lower_limit);
	kvReqSet.setParam("Fbuy_add_limit",data.Fbuy_add_limit);
      kvReqSet.setParam("Fstat_buy_tdate",data.Fstat_buy_tdate);
      kvReqSet.setParam("Fcurtype",data.Fcurtype);
      kvReqSet.setParam("Fsp_full_name",data.Fsp_full_name);
      kvReqSet.setParam("Ffund_brief_name",data.Ffund_brief_name);
      kvReqSet.setParam("Ftype",data.Ftype);
      kvReqSet.setParam("Fbuy_confirm_type",data.Fbuy_confirm_type);
      kvReqSet.setParam("Fstat_flag",data.Fstat_flag);
      kvReqSet.setParam("Fsave_sett_time",data.Fsave_sett_time);
      kvReqSet.setParam("Ffetch_sett_time",data.Ffetch_sett_time);
      kvReqSet.setParam("Frisk_ass_flag",data.Frisk_ass_flag);
      kvReqSet.setParam("Frisk_type",data.Frisk_type);
      kvReqSet.setParam("Fbuyfee_tday_limit",data.Fbuyfee_tday_limit);
      kvReqSet.setParam("Fstandby1",data.Fstandby1);
     
    szValue = kvReqSet.pack();

    //将szValue写入ckv
	if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_ALL_SPID_CONF), key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void delSpAndFundcodeFromKv(FundSpConfig& data)
{
	string key = "spid_fundcode_conf_"+ toString(data.Fspid) + "_" + toString(data.Ffund_code);
	gCkvSvrOperator->del(NOT_REUPDATE_CKV(CKV_KEY_SPID_FUNDCODE_CONF),key);
}

bool setAllSupportSpConfig(CMySQL* pMysql)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
    //string key = gPtrConfig->m_KvCfg.kvAddress + "fund_support_sp_all";
	string key = "fund_support_sp_all";

	vector<FundSpConfig> fundSpConfigVec;
	if( !queryFundSpAllConfig(pMysql, fundSpConfigVec, false))
	{
		TRACE_DEBUG("no fund sp config");
		return true;
	}

    string szValue;

	CParams kvReqSet;
	char szParaName[64] = {0};

    //设置要修改的数据szValue
    int total_num = 0;
    for(vector<FundSpConfig>::iterator iter= fundSpConfigVec.begin(); iter != fundSpConfigVec.end(); ++iter)
    {
		FundSpConfig fundSpConfig = *iter;

		snprintf(szParaName, sizeof(szParaName), "Fspid_%d", total_num);
		kvReqSet.setParam(szParaName, fundSpConfig.Fspid);
		snprintf(szParaName, sizeof(szParaName), "Fsp_name_%d", total_num);
		kvReqSet.setParam(szParaName, fundSpConfig.Fsp_name);
		snprintf(szParaName, sizeof(szParaName), "Ffund_code_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Ffund_code);
		snprintf(szParaName, sizeof(szParaName), "Ffund_name_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Ffund_name);
		snprintf(szParaName, sizeof(szParaName), "Fpurpose_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Fpurpose);
		snprintf(szParaName, sizeof(szParaName), "Flstate_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Flstate);
		snprintf(szParaName, sizeof(szParaName), "Fredem_valid_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Fredem_valid);

		kvReqSet.setParam(string("Fclose_flag_"+toString(total_num)).c_str(), fundSpConfig.Fclose_flag);
		kvReqSet.setParam(string("Ftransfer_flag_"+toString(total_num)).c_str(), fundSpConfig.Ftransfer_flag);
		kvReqSet.setParam(string("Fbuy_valid_"+toString(total_num)).c_str(), fundSpConfig.Fbuy_valid);
		kvReqSet.setParam(string("Fend_type_"+toString(total_num)).c_str(), fundSpConfig.Fend_type);
		kvReqSet.setParam(string("Fscope_upper_limit_"+toString(total_num)).c_str(), fundSpConfig.Fscope_upper_limit);
		kvReqSet.setParam(string("Fscope_lower_limit_"+toString(total_num)).c_str(), fundSpConfig.Fscope_lower_limit);
		kvReqSet.setParam(string("Fduration_type_"+toString(total_num)).c_str(), fundSpConfig.Fduration_type);
		kvReqSet.setParam(string("Fduration_"+toString(total_num)).c_str(), fundSpConfig.Fduration);
		kvReqSet.setParam(string("Frestrict_mode_"+toString(total_num)).c_str(), fundSpConfig.Frestrict_mode);
		kvReqSet.setParam(string("Frestrict_num_"+toString(total_num)).c_str(), fundSpConfig.Frestrict_num);
		kvReqSet.setParam(string("Fstate_"+toString(total_num)).c_str(), fundSpConfig.Fstate);
		kvReqSet.setParam(string("Fbuy_first_lower_limit_"+toString(total_num)).c_str(), fundSpConfig.Fbuy_first_lower_limit);
		kvReqSet.setParam(string("Fbuy_lower_limit_"+toString(total_num)).c_str(), fundSpConfig.Fbuy_lower_limit);
		kvReqSet.setParam(string("Fbuy_add_limit_"+toString(total_num)).c_str(), fundSpConfig.Fbuy_add_limit);
		kvReqSet.setParam(string("Fstat_buy_tdate_"+toString(total_num)).c_str(), fundSpConfig.Fstat_buy_tdate);
             kvReqSet.setParam(string("Fcurtype_"+toString(total_num)).c_str(), fundSpConfig.Fcurtype);

		snprintf(szParaName, sizeof(szParaName), "Fsp_full_name_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Fsp_full_name);
		snprintf(szParaName, sizeof(szParaName), "Ffund_brief_name_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Ffund_brief_name);
		snprintf(szParaName, sizeof(szParaName), "Ftype_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Ftype);
		snprintf(szParaName, sizeof(szParaName), "Fbuy_confirm_type_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Fbuy_confirm_type);
		snprintf(szParaName, sizeof(szParaName), "Fstat_flag_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Fstat_flag);
		snprintf(szParaName, sizeof(szParaName), "Fsave_sett_time_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Fsave_sett_time);
		snprintf(szParaName, sizeof(szParaName), "Ffetch_sett_time_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Ffetch_sett_time);
		snprintf(szParaName, sizeof(szParaName), "Frisk_ass_flag_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Frisk_ass_flag);
		snprintf(szParaName, sizeof(szParaName), "Frisk_type_%d", total_num);
		kvReqSet.setParam(szParaName,fundSpConfig.Frisk_type);
		snprintf(szParaName, sizeof(szParaName), "Fbuyfee_tday_limit_%d", total_num);
		 kvReqSet.setParam(szParaName,fundSpConfig.Fbuyfee_tday_limit);
		 snprintf(szParaName, sizeof(szParaName), "Fstandby1_%d", total_num);
		 kvReqSet.setParam(szParaName,fundSpConfig.Fstandby1);

		if(!setSupportFundSpAndFundcode(fundSpConfig))
		{ 
			return false;
		}

		if(!setFundSpInfo(fundSpConfig))
		{

			return false;
		}
		
		total_num += 1;
    }

	kvReqSet.setParam("total_num",
total_num);
    szValue = kvReqSet.pack();

    //将szValue写入ckv
	if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_FUND_SUPPORT_SP_ALL), key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
		
}

bool preCheckSpLoaningEnough(CMySQL* pMysql,const string&spid,const string&fund_code,LONG total_fee)
{
    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));

    strncpy(fundSpConfig.Fspid, spid.c_str(), sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, fund_code.c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(pMysql, fundSpConfig, false))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }

    LONG cftChargeNum = (gPtrConfig->m_AppCfg.over_loading_use_common == 1?fundSpConfig.Fcft_chargenum:0);
    
    //总T+0赎回额度+本次赎回金额小于基金公司垫资+财付通垫资-临界值时标识金额充值可以在子账户减前校验
    if (fundSpConfig.Fredem_total + total_fee < fundSpConfig.Fsp_chargenum+cftChargeNum -gPtrConfig->m_AppCfg.sp_loading_enough_check_befor_subacc)
    {
        return true;
    }
    return false;
}

void checkRedemOverLoading(CMySQL* pMysql, FundSpConfig& fundSpConfig ,LONG total_fee,bool isLocked)
{
    LONG cftChargeNum = (gPtrConfig->m_AppCfg.over_loading_use_common == 1?fundSpConfig.Fcft_chargenum:0);

    LONG tmp_sp_loading_last_allowed_redem_limit = 0;
    LONG tmp_sp_loading_warning_limit = 0;
    if (gPtrConfig->m_spLastAllowFastRedem_Dianzi.find(fundSpConfig.Fspid) != gPtrConfig->m_spLastAllowFastRedem_Dianzi.end())
    {
        tmp_sp_loading_last_allowed_redem_limit = gPtrConfig->m_spLastAllowFastRedem_Dianzi[fundSpConfig.Fspid];
    }
    if (gPtrConfig->m_spWarn_Dianzi.find(fundSpConfig.Fspid) != gPtrConfig->m_spWarn_Dianzi.end())
    {
        tmp_sp_loading_warning_limit = gPtrConfig->m_spWarn_Dianzi[fundSpConfig.Fspid];
    }

    if (tmp_sp_loading_last_allowed_redem_limit <=0)
    {
        tmp_sp_loading_last_allowed_redem_limit = gPtrConfig->m_AppCfg.sp_loading_last_allowed_redem_limit;
    }

    if (tmp_sp_loading_warning_limit <= 0)
    {
        tmp_sp_loading_warning_limit = gPtrConfig->m_AppCfg.sp_loading_warning_limit;
    }
    
    //总T+0赎回额度+本次赎回金额大于基金公司垫资+财付通垫资-临界值时不能再做T+0提现
    if (fundSpConfig.Fredem_total + total_fee > fundSpConfig.Fsp_chargenum+cftChargeNum -gPtrConfig->m_AppCfg.sp_loading_stop_redem_limit)
    {
        //alert(ERR_T0_REDEM_REFUSED, (string("spid:")+fundSpConfig.Fspid+"总垫资不足,停止T+0提现").c_str());
        //垫资额度不足的不能赎回
        throw EXCEPTION(ERR_T0_REDEM_REFUSED, "sp loading is not enough,connot redem t0 for draw."); ;
    }

    if (fundSpConfig.Fredem_total + total_fee >= fundSpConfig.Fsp_chargenum+cftChargeNum -tmp_sp_loading_last_allowed_redem_limit)
    {
        if ((fundSpConfig.Fredem_valid&0x07) == 1 && isLocked)  // 只有赎回确认时才加锁更新赎回标记位
        {
            //更新赎回标记为只能T+1赎回
            fundSpConfig.Fredem_valid = (fundSpConfig.Fredem_valid|0x03); 
            char szTimeNow[MAX_TIME_LENGTH+1] = {0};
            GetTimeNow(szTimeNow);
            strncpy(fundSpConfig.Fmodify_time,szTimeNow,sizeof(fundSpConfig.Fmodify_time)-1);
            updateSpRedemFlag(pMysql, fundSpConfig);
            setAllSupportSpConfig(pMysql);

            char szErrMsg[256] = {0};
            snprintf(szErrMsg, sizeof(szErrMsg), "基金公司垫资额度即将耗尽,停止t+0赎回");        
            alert(ERR_NOT_ENOUGH_LOADING, string("spid:")+string(fundSpConfig.Fspid)+szErrMsg);
            
        }
    }

    /*if(fundSpConfig.Fredem_total > fundSpConfig.Fsp_chargenum - tmp_sp_loading_warning_limit
        && fundSpConfig.Fredem_total < fundSpConfig.Fsp_chargenum - tmp_sp_loading_warning_limit + 100000000)//设置告警区间，避免频繁告警
    {
        //当商户垫资额度还有1000万元耗尽时告警
        char szErrMsg[256] = {0};
        snprintf(szErrMsg, sizeof(szErrMsg), "基金公司垫资额度即将耗尽,请尽快补充");        
        alert(ERR_NOT_ENOUGH_LOADING, string("spid:")+string(fundSpConfig.Fspid)+szErrMsg);
    }*/

    return ;
	
	/*if(fundSpConfig.Fredem_total + total_fee < fundSpConfig.Fsp_chargenum)
	{
		return false;
	}

	if(gPtrConfig->m_AppCfg.over_loading_use_common != 1 )
	{
		return true;//超过基金公司垫资额后不能在使用总垫资迟
	}

	LONG allRedomTotal = 0;
	sumRedomTotal(pMysql, allRedomTotal);

	//是否超过总垫资池
	if(allRedomTotal + total_fee < gPtrConfig->m_AppCfg.total_loading_credit)
	{
		return false;
	}

	return true;*/
}

void querySpConfigCache(CMySQL* pMysql, string spid, SpConfigCache& sp_config)
{
	time_t curtype_cache_time = gPtrConfig->m_spConfigCache[spid].timeout;
	time_t tt = time(NULL);
	if(curtype_cache_time > tt)
	{
		sp_config = gPtrConfig->m_spConfigCache[spid];
		return;
	}
	vector<FundSpConfig> fundSpConfigVec;
	if( !queryFundSpConfig(pMysql, fundSpConfigVec, spid,false))
	{
		TRACE_DEBUG("no fund sp config");
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error");    
	}
	if(fundSpConfigVec.size()>1)
	{
		TRACE_DEBUG("more than one sp config");
		throw EXCEPTION(ERR_BAD_PARAM, "input spid has muti fund_code"); 
	}
	FundSpConfig& fundSpConfig = fundSpConfigVec[0];
	sp_config.curtype = fundSpConfig.Fcurtype;
	sp_config.fund_code = toString(fundSpConfig.Ffund_code);
	sp_config.spid = spid;
	sp_config.close_flag = fundSpConfig.Fclose_flag;
	sp_config.type = fundSpConfig.Ftype;
	sp_config.buy_confirm_type = fundSpConfig.Fbuy_confirm_type;
	sp_config.timeout = tt + 3600 * 12;// 本地缓存12小时
	gPtrConfig->m_spConfigCache[spid]=sp_config;	
}

int querySubaccCurtype(CMySQL* pMysql, string spid)
{
	if(gPtrConfig->m_AppCfg.multi_sp_config == 0)
	{
		//不开启多账户版本
		return CUR_FUND_SP;
	}
	int subacc_curtype = 0;
	
	time_t curtype_cache_time = gPtrConfig->m_sp_subaccCurtype[spid].timeout;
	time_t tt = time(NULL);
	if(curtype_cache_time > tt)
	{
		subacc_curtype = gPtrConfig->m_sp_subaccCurtype[spid].curtype;
		return subacc_curtype;
	}
	
	vector<FundSpConfig> fundSpConfigVec;
	if( !queryFundSpAllConfig(pMysql, fundSpConfigVec, false))
	{
		TRACE_DEBUG("no fund sp config");
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error");    
	}

	bool curtype_exist = false;
	
	for(vector<FundSpConfig>::size_type i= 0; i != fundSpConfigVec.size(); ++i)
	{
		FundSpConfig fundSpConfig = fundSpConfigVec[i];
		if(spid == fundSpConfig.Fspid)
		{
			subacc_curtype = fundSpConfig.Fcurtype;
			curtype_exist = true;
		}

		SpSubaccCurtype sp_subaccCurtype;
		sp_subaccCurtype.curtype = fundSpConfig.Fcurtype;
		sp_subaccCurtype.timeout = tt + 3600 * 12;// 本地缓存12小时
		gPtrConfig->m_sp_subaccCurtype[fundSpConfig.Fspid] = sp_subaccCurtype;
	}

	if(!curtype_exist || subacc_curtype == 0)
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error");    
	}
	
	return subacc_curtype;

}
void updateSpRedemFlag(CMySQL* pMysql, FundSpConfig& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_sp_config SET "
                    " Fredem_valid = %d,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fspid='%s' AND "
                    " Ffund_code='%s'", 
                    data.Fredem_valid,
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

void updateSpBuyfeeTday(CMySQL* pMysql, FundSpConfig& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_sp_config SET "
                    " Fbuyfee_tday_limit = %ld,"
                    "Fbuyfee_tday_limit_offset = %ld,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fspid='%s' AND "
                    " Ffund_code='%s'", 
                    data.Fbuyfee_tday_limit,
                    data.Fbuyfee_tday_limit_offset,
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}
/**
* 检查基金是否可以申购
*/
void checkFundcodePurchaseValid(int buy_valid_flag)
{
	if(!(buy_valid_flag & FUNDCODE_PURCHASE_VALID))
	{
		throw EXCEPTION(ERR_CAN_NOT_PURCHASE, "fundcode can not purchase."); 
	}
	
}

/**
* 检查是否达到基金规模上限及是否满足购买金额要求，达到则更新基金不可申购并同步ckv
* 此检查只是常规检查，非精确，基金允许一定超卖，如果做秒杀类型检查不可使用该方法
*/
void checkFundcodeToScopeUpperLimit(int uid ,const string &sys_time,LONG total_fee, FundSpConfig& data, bool needQueryDB)
{
	if(data.Fclose_flag == CLOSE_FLAG_NORMAL)
	{	
		//不封闭的产品不检查
		return;
	}

	if(total_fee < data.Fbuy_lower_limit || 0 != total_fee % data.Fbuy_add_limit)
	{
		//小于购买金额限制或者不满足步长要求，报错
		throw EXCEPTION(ERR_BAD_PARAM, "input total_fee error"); 
	}
	// 检查可立即确认份额的首次购买
	if(data.Fbuy_first_lower_limit>0&&data.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		// 根据份额判断是否是首次购买
		LONG balance = querySubaccBalance(uid, data.Fcurtype,false);		
		if(balance<data.Fbuy_first_lower_limit&&total_fee<data.Fbuy_first_lower_limit)
		{
			//小于首次购买要求，报错
			throw EXCEPTION(ERR_BAD_PARAM, "用户购买金额不满足商户首次购买要求"); 
		}
	}
	if(needQueryDB && !queryFundSpAndFundcodeConfig(gPtrFundDB, data, false))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}
	
	if(data.Fscope_upper_limit > 0)
	{
        	if(data.Fscope + total_fee > data.Fscope_upper_limit)
        	{
        		throw EXCEPTION(ERR_SCOPE_UPPER_LIMIT, "over scope upper limit.");
        	}
	}
    
	if(data.Fbuyfee_tday_limit > 0)
	{
           //基金配置Fstat_flag=1时按自然日检查限额
           string strStatDay=( SPCONFIG_STAT_DDAY & data.Fstat_flag )? nowdate(sys_time):calculateFundDate(sys_time);
           if (string(data.Fstat_buy_tdate)< strStatDay )
           {
               return;
           }
           if(data.Ftotal_buyfee_tday + total_fee > data.Fbuyfee_tday_limit || (data.Fbuy_valid&FUNDCODE_BUY_DAY_LIMIT))
           {
               throw EXCEPTION(ERR_SCOPE_UPPER_LIMIT, "over day buy upper limit.");
           }
	}

}


/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundSpBuyValid(FundSpConfig& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_sp_config SET "
                    " Fbuy_valid = %d,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fspid='%s' AND "
                    " Ffund_code='%s'", 
                    data.Fbuy_valid,
                    gPtrFundDB->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where条件--------
                    gPtrFundDB->EscapeStr(data.Fspid).c_str(),
                    gPtrFundDB->EscapeStr(data.Ffund_code).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    gPtrFundDB->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (gPtrFundDB->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
//调用该函数前必须先加锁查询出基金配置表
void checkAndUpdateFundScope(FundSpConfig& data, LONG total_fee,const string &trade_date,bool *pNeedRefund,string *refunddesc,bool bPresentBuy,double redeemRate/*=1 */)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};
    GetTimeNow(szTimeNow);

    string stat_buy_tdate=data.Fstat_buy_tdate;
    LONG total_buyfee_tday=data.Ftotal_buyfee_tday;
    LONG scope =  data.Fscope +  total_fee;
    int buy_valid = data.Fbuy_valid;

    //当前交易日大于统计中的交易日(跨交易日情况)
    if (stat_buy_tdate < trade_date)
    {
        stat_buy_tdate = trade_date;
        total_buyfee_tday = total_fee;
    }
    else
    {
        total_buyfee_tday += total_fee;
    }

    if (data.Fbuyfee_tday_limit>0) //有日限额
    {
    	LONG total_redeem_tday=0;
    	if((data.Fstat_flag&SPCONFIG_STAT_NET)&&data.Fstat_redeem_tdate>=trade_date) //计算净申购
    	{
    		total_redeem_tday=data.Ftotal_redeem_tday*redeemRate;
    	}
        if (total_buyfee_tday-total_redeem_tday+data.Fbuyfee_tday_limit_offset>= data.Fbuyfee_tday_limit)
        {
		    if ((buy_valid&(FUNDCODE_BUY_DAY_LIMIT))==0) //只在关闭时告警一次
            {
                alert(ERR_SCOPE_UPPER_LIMIT, string(data.Ffund_code)+"达到基金日购买上限");
            }
            buy_valid = (buy_valid|FUNDCODE_BUY_DAY_LIMIT); 
        }
        else
        {
            buy_valid = (buy_valid&(~FUNDCODE_BUY_DAY_LIMIT));
        }

        if (pNeedRefund != NULL && (total_buyfee_tday-total_redeem_tday> data.Fbuyfee_tday_limit))
        {
            *pNeedRefund = true;
            if (refunddesc)
            {
                *refunddesc = "超过日购买限额";
            }
			char msg[64]={0};
			snprintf(msg,sizeof(msg),"%s达到基金日购买上限退款:[%ld-%ld>%ld]rate[%lf ]",data.Ffund_code,total_buyfee_tday,total_redeem_tday,data.Fbuyfee_tday_limit,redeemRate);
            alert(ERR_SCOPE_UPPER_LIMIT, string(msg));
        }
    }

    if (data.Fscope_upper_limit > 0)
    {
        if(data.Fscope + total_fee+data.Fscope_upper_limit_offset >= data.Fscope_upper_limit)
        {
            //超过基金可申购规模上限，标记基金不可在申购，更新ckv
            buy_valid = (buy_valid&(~FUNDCODE_PURCHASE_VALID));
            alert(ERR_SCOPE_UPPER_LIMIT, string(data.Ffund_code)+"达到基金规模的上限");
        }
        
        if (pNeedRefund != NULL && (data.Fscope + total_fee> data.Fscope_upper_limit))
        {
            *pNeedRefund = true;
            if (refunddesc)
            {
                *refunddesc = "超过购买限额";
            }
            alert(ERR_SCOPE_UPPER_LIMIT, string(data.Ffund_code)+"达到基金规模的上限退款");
        }
        
    }

    if (pNeedRefund != NULL && (*pNeedRefund == true))   
    {
        if (false == bPresentBuy)  
        {
            //退款不累加额度
            scope = data.Fscope;
            total_buyfee_tday = total_buyfee_tday - total_fee;
        }
        else
        {
            *pNeedRefund = false; //赠送申购确认接口不限额
        }
    }

    if (scope == data.Fscope
        && buy_valid == data.Fbuy_valid
        && total_buyfee_tday == data.Ftotal_buyfee_tday
        && stat_buy_tdate == data.Fstat_buy_tdate)
    {
        //没有需要更新的字段
        return;
    }
    
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_sp_config SET "
                    " Fscope = %zd,"
                    " Ftotal_buyfee_tday = %zd,"
                    " Fstat_buy_tdate='%s', "
                    " Fbuy_valid=%d, "
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fspid='%s' AND "
                    " Ffund_code='%s'", 
                    scope,
                    total_buyfee_tday,
                    stat_buy_tdate.c_str(),
                    buy_valid,
                    gPtrFundDB->EscapeStr(szTimeNow).c_str(),
                    //--------where条件--------
                    gPtrFundDB->EscapeStr(data.Fspid).c_str(),
                    gPtrFundDB->EscapeStr(data.Ffund_code).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    gPtrFundDB->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (gPtrFundDB->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }

    if (buy_valid != data.Fbuy_valid  || stat_buy_tdate != data.Fstat_buy_tdate)
    {
        setAllSupportSpConfig(gPtrFundDB); //更新ckv
    }
}


/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void addFundSpScope(FundSpConfig &data, const string &acc_time, LONG fee)
{
	// 无基金规模上限,不需要更新
    if (data.Fscope_upper_limit == 0){
		return;
	}
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_sp_config SET "
                    " Fscope = Fscope + %ld,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fspid='%s' AND "
                    " Ffund_code='%s'", 
                    fee,
                    gPtrFundDB->EscapeStr(acc_time).c_str(),
                    //--------where条件--------
                    gPtrFundDB->EscapeStr(data.Fspid).c_str(),
                    gPtrFundDB->EscapeStr(data.Ffund_code).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    gPtrFundDB->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (gPtrFundDB->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}
