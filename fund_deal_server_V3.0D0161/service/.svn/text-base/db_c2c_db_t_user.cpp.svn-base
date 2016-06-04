#include "db_c2c_db_t_user.h"


extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


/**
* 查询子账户用户信息
*/
bool querySubaccUserInfo(CMySQL* pMysql, SubaccUser& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fuid,Fqqid,Fcurtype,Fbalance,Fcon "
                    " FROM c2c_db_%02d.t_user_%d " 
                    " WHERE "
                    " Fuid=%d and " 
                    " Fcurtype= %d  " 
                    " %s ",
                    Sdb1(data.Fuid),
                    Stb1(data.Fuid),
                    data.Fuid,
                    data.Fcurtype,
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
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            data.Fuid= row[0] ? atoi(row[0]) : 0;
            strncpy(data.Fqqid,row[1] ? row[1] : "", sizeof(data.Fqqid) - 1);
			data.Fcurtype = row[2] ? atoi(row[2]) : 0;
			data.Fbalance= row[3] ? atoll(row[3]) : 0;
            data.Fcon= row[4] ? atoll(row[4]) : 0;
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


bool querySubaccUserInfo(CMySQL* pMysql, int uid, int curtype, LONG& balance,LONG&con)
{
	SubaccUser subaccUser;
	memset(&subaccUser, 0, sizeof(SubaccUser));
	subaccUser.Fuid = uid;
	subaccUser.Fcurtype = curtype;
	if(!querySubaccUserInfo(pMysql,subaccUser, false))
	{
		return false;
	}

	balance = subaccUser.Fbalance - subaccUser.Fcon;
	con = subaccUser.Fcon;

	return true;
}


/**
* 查询余额通账户余额
*/
LONG querySubaccBalance(int uid , int subacc_curtype, bool throwNotexistExcp,LONG *freezeBalance)
{
    //按uid 找到用户所在的分段信息
    if (freezeBalance)
    {
        *freezeBalance = 0;
    }
    int64_t balance = 0;
    int64_t con = 0;
    string subacc_db;
    string subacc_ttc;
    getSubaccdbAndSubaccttc(uid, subacc_db,subacc_ttc);
    
    //白名单内走子账户CKV
    if(checkUserWhiteList("white_list_fundquerybalance_by_uid",toString(uid)))
    {
        TRACE_DEBUG("uid[%d] in white list will be CKV",
                    uid)
        
        if( 0 == querySubaccBalanceFromCKV(uid,subacc_curtype,&balance,throwNotexistExcp,freezeBalance))
        {
            return balance;
        }
    }
    //TTC查询
    else
    {
        TRACE_DEBUG("uid[%d] not in white list will be TTC",
                    uid)
        if( 0 == querySubaccBalanceFromTTC(uid,subacc_curtype,&balance,subacc_ttc, throwNotexistExcp,freezeBalance))
        {
             return balance;
        }	   
    }

    //以上步骤查询失败则查询子账户DB
    if(!querySubaccUserInfo(subaccDB[subacc_db], uid, subacc_curtype, balance,con))
    {
	    //没查询到记录，什么都不做，balance=0
        TRACE_DEBUG("Fbalance no find in TTC DB uid[%d], curtype[%d]",uid, subacc_curtype);
    }

    TRACE_DEBUG("Fbalance from db[%ld],Fcon from db[%ld]", balance, con);

    if (freezeBalance)
    {
        *freezeBalance = con;
    }
    return balance;
}

void getSubaccdbAndSubaccttc(int uid, string & subacc_db,string & subacc_ttc)
{
    subacc_db = "";
    subacc_ttc = "";

    int seg_index =0;

    if (SUBACC_SPLIT_BY_LAST3 == gPtrConfig->m_AppCfg.subacc_split_by_last3)
    {
        // 后三位
        seg_index = uid % 100;
    }
    else
    {
        seg_index = uid;
    }
	
    for(unsigned int i= 0; i< gPtrConfig->m_subaccUserSegVec.size(); ++i)
    {
	    if(gPtrConfig->m_subaccUserSegVec[i].start_uid<= seg_index && seg_index <= gPtrConfig->m_subaccUserSegVec[i].end_uid)
	    {
		    subacc_db = gPtrConfig->m_subaccUserSegVec[i].subacc_db;
		    subacc_ttc = gPtrConfig->m_subaccUserSegVec[i].subacc_ttc;
		    //TRACE_DEBUG("querySubaccBalance subacc_db[%s],subacc_ttc[%s]", subacc_db.c_str(), subacc_ttc.c_str());
			
		    break;
	    }
		
    }

    if(subacc_db.empty() || subacc_ttc.empty())
    {
	    throw CException(ERR_NULL_XML_NODE, "系统错误，请通知管理员", __FILE__, __LINE__);	
    }
}

int querySubaccBalanceFromCKV(int uid, int subacc_curtype, int64_t * balance, bool throwNotexistExcp,LONG *freezeBalance)
{
    string value;
    int cas = -1;
    string strUid =  toString(uid);
    int col = 0;
    if( subacc_curtype>=89 && subacc_curtype<=99 )
    {
        col = subacc_curtype-80;
    }
    else if( subacc_curtype>=111 && subacc_curtype<=120 )
    {
        col = subacc_curtype-91;
    }
    else
    {
        TRACE_ERROR("[%s][%d]querySubaccBalanceFromCKV subacc_curtype [%d] not in 89-99,111-120", __FILE__, __LINE__, subacc_curtype);
        throw CException(ERR_SUBACC_TYPE_ERROR, "系统繁忙，请通知管理员", __FILE__, __LINE__);
    }

    int ret = gSubAccCkvSvrOperator->get_by_col(strUid, col, value, cas);
   
    /**
     * -13004 ckv说明列操作不合法，可能是取了非法的列号，或者数据内容不是列操作的
     * 此错误表示没有设置相应列数据去获取时报错
     * 例如用户只开通了币种为90的账户,但开了91币种的基金账户，去获取91币种CKV列时就报-13004
     * 如果用户只开通了94币种，去获取91币种时ckv返回retcode=0但内容为空
     */
    if(ret == CKV_E_NO_DATA || ret == CKV_ERR_SO_PARSE_ARGS)
    {
        if(throwNotexistExcp)
	    {
		    throw CException(ERR_CORE_USER_EMPTY, "帐户不存在", __FILE__, __LINE__);
	    }
        if(freezeBalance)
        {
            *freezeBalance = 0;
        }

        
        return 0;
    }
    else if(ret!=0)
    {
        TRACE_ERROR("ERROR get_by_col from CKV key=%s,retcode==%d,col=%d",
                strUid.c_str(), ret,col);
        return -1;
    }
    else
    {
        if(value.empty())
        {
            if(throwNotexistExcp)
	        {
		        throw CException(ERR_CORE_USER_EMPTY, "帐户不存在", __FILE__, __LINE__);
	        }
            if(freezeBalance)
            {
                *freezeBalance = 0;
            }
            balance = 0;
            return 0;
        }
        else
        {
            
            CStr2Map outputMap;
            Tools::StrToMap(outputMap, value,"&","=");

            int64_t tmpBalance = atoll(outputMap["ban"].c_str());
            int64_t con = atoll(outputMap["con"].c_str());
            TRACE_DEBUG("Fbalance from CKV[%ld],Fcon from CKV[%ld]", tmpBalance, con);

            if(freezeBalance)
            {
                *freezeBalance = con;
            }
            *balance = tmpBalance - con;
            return 0;
        }
    }
}

int querySubaccBalanceFromTTC(int uid, int subacc_curtype, int64_t *  balance, const string & subacc_ttc, bool throwNotexistExcp, LONG *freezeBalance)
{
    static ErrorSum ui_err_sum(gPtrConfig->m_AppCfg.ui_ttc_max_err, gPtrConfig->m_AppCfg.ui_ttc_stop_time);

    if(balance != NULL)
    {
        *balance = 0;
    }
    if(freezeBalance != NULL)
    {
        *freezeBalance = 0;
    }

    int nRet = -1;

    if (ui_err_sum.check())
    {
	    char szIP[32] = {0};
	    strncpy(szIP, gPtrConfig->m_subaccTtcCfgVec[subacc_ttc].host.c_str(), sizeof(szIP)-1);	
	    CUserTTC CacheClient(szIP, gPtrConfig->m_subaccTtcCfgVec[subacc_ttc].port, gPtrConfig->m_subaccTtcCfgVec[subacc_ttc].overtime);

	    Result rResult;
	    rResult.Reset();
	    nRet = CacheClient.GetUser(uid, subacc_curtype, &rResult);
		
	    ui_err_sum.update(nRet);
		
	    if (nRet != 0)
	    {
		    TRACE_ERROR("[%s][%d]querySubaccBalance::Commit(). GetUser failed. Uid[%d], Ret[%d], From[%s], Error[%s], SvrIP[%s], Port[%d]", 
					    __FILE__, __LINE__, 
					    uid, nRet, rResult.ErrorFrom(), rResult.ErrorMessage(),gPtrConfig->m_subaccTtcCfgVec[subacc_ttc].host.c_str(), gPtrConfig->m_subaccTtcCfgVec[subacc_ttc].port);
            return -1;

		    //失败了查询数据库，先不报错
		    //throw CException(ERR_CALL_TTC_RELAY, "系统繁忙，请通知管理员", __FILE__, __LINE__);	
	    }                            
	    else                           
	    {                                    
		    int nTotalRows = rResult.TotalRows();
		    if(nTotalRows == 0)
		    {
			    if(throwNotexistExcp)
			    {
				    throw CException(ERR_CORE_USER_EMPTY, "帐户不存在", __FILE__, __LINE__);
			    }
				
			    //账户不存在，金额当0处理
			    return 0;
		    }
		    else if(nTotalRows == 1)
		    {
			    rResult.FetchRow();
			    //iodat["uin"] = rResult.StringValue("Fqqid");
				
			    char szTmpBalance[32] = {0};
			    snprintf(szTmpBalance, sizeof(szTmpBalance)-1, "%lld", rResult.IntValue("Fbalance"));

			    char szTmpFcon[32] = {0};
			    snprintf(szTmpFcon, sizeof(szTmpFcon)-1, "%lld", rResult.IntValue("Fcon"));

			    //TRACE_DEBUG("Fbalance from cache[%d].", rResult.IntValue("Fbalance"));
			    TRACE_DEBUG("Fbalance from cache[%s],Fcon from cache[%s]", szTmpBalance, szTmpFcon);
			    if (freezeBalance)
			    {
			        *freezeBalance = atoll(szTmpFcon);
			    }
				if(NULL != balance)
                {
			        *balance = atoll(szTmpBalance) - atoll(szTmpFcon);//余额减去冻结部分
                }
                return 0;
		    }
		    else
		    {	
			    TRACE_ERROR("[%s][%d]GetUser err.nTotalRows [%d]", __FILE__, __LINE__, nTotalRows);
			    throw CException(ERR_CALL_TTC_RELAY, "系统繁忙，请通知管理员", __FILE__, __LINE__);	
		    }
			                                                           
	    }
    }
    else
    {
	    TRACE_NORMAL("t_user_info ttc fail exceed max, stop query from ttc.");
        return -1;
    }
}

void querySubaccBalanceListFromCKV(int uid, const vector<int> & subacc_curtype_list, vector<SubaccUser> &SubaccListUser)
{
    //如果没有币种信息,则一定查不到数据.
    if(subacc_curtype_list.empty()) 
        return;
    //设置CKV 输入参数
    TKeyGetListNode ckv_node;
    for(size_t i=0; i<subacc_curtype_list.size(); i++)
    {
        TGetNode node;
        int subacc_curtype = subacc_curtype_list[i];
        //CKV列值 从0-511 子账户从80 开始存储,所以减去80,就是 币种 对应的CKV列值
        if( subacc_curtype>=89 && subacc_curtype<=99 )
        {
            node.col= subacc_curtype-80;//89-80 -80
        }
        else if( subacc_curtype>=111 && subacc_curtype<=120 )
        {
            node.col= subacc_curtype-91;//111-120 -91
        }
        else
        {
            TRACE_ERROR("%s|错误,CKV %d币种不在范围内  不在 89-99 111-120  范围",__FUNCTION__,subacc_curtype);
            throw (CException(ERR_SUBACC_TYPE_ERROR,"系统繁忙，请通知管理员", __FILE__, __LINE__));
        }
        ckv_node.v_col_node.push_back(node);
    }
    //批量查询CKV列值  根据子账户 币种来查询!
    ckv_node.key=toString(uid);
    int iRet = gSubAccCkvSvrOperator->get_by_multi_col(ckv_node);
   
    //========查询用户份额列模式 value数值格式==================
    // Key：用户uid
    // Value: 采用ckv列模式保存子账户余额信息，每列保存一种子账户的余额信息.
    // 具体每列数据如下：
    // ban=10000000&con=3000000&ct=80&stat=1&tm=1408608395&vc=1235
    // ban 账户余额
    // con 冻结总金额
    // ct  【curtype_x子账户币种类型】//stat/tm/vc

    //iRet==0 是全部正确的情况
    //iRet>0 是失败的个数,只要不是全部失败即可
    if(iRet>=0 && iRet!=(int)ckv_node.v_col_node.size() )
    {
        for(vector<TGetNode>::iterator it=ckv_node.v_col_node.begin(); it!=ckv_node.v_col_node.end(); it++ )
        {            
            /**
             * -13004 ckv说明列操作不合法，可能是取了非法的列号，或者数据内容不是列操作的
             * 此错误表示没有设置相应列数据去获取时报错
             * 例如用户只开通了币种为90的账户,但开了91币种的基金账户，去获取91币种CKV列时就报-13004
             * 如果用户只开通了94币种，去获取91币种时ckv返回retcode=0但内容为空
             */
            SubaccUser tmpSubaccUser;
            int curtype;
            if(it->col>=9 && it->col<=19)
            {
                curtype=it->col+80;
            }
            else if(it->col>=20 && it->col<=29)
            {
                curtype=it->col+91;
            }

            if(it->retcode == CKV_E_NO_DATA || it->retcode == CKV_ERR_SO_PARSE_ARGS)
            {
                //数据不存在
                //FIX spid没找到 设置为0 
                tmpSubaccUser.Fuid = uid;
                tmpSubaccUser.Fcurtype = curtype;
                tmpSubaccUser.Fbalance = 0;
                tmpSubaccUser.Fcon = 0;
            }
            else if(it->retcode!=0)
            {
                TRACE_ERROR("ERROR get_by_multi_col from CKV key=%s,retcode==%d,node.col=%d",
                    ckv_node.key.c_str(), it->retcode,it->col);
                throw (CException(ERR_SUBACC_CKV_ERROR,"系统错误,CKV获取数据失败", __FILE__, __LINE__));      
            }
            else
            {
                //CKV 返回0 但是数据为空,认为金额为0
                if(it->data.empty())
                {
                    tmpSubaccUser.Fuid = uid;
                    tmpSubaccUser.Fcurtype = curtype;
                    tmpSubaccUser.Fbalance = 0;
                    tmpSubaccUser.Fcon = 0;
                }
                else
                {
                    CStr2Map outputMap;
                    Tools::StrToMap(outputMap, it->data,"&","=");
                    tmpSubaccUser.Fuid = uid;
                    tmpSubaccUser.Fcurtype = curtype;
					tmpSubaccUser.Fcon = toLong(outputMap["con"].c_str());
                    tmpSubaccUser.Fbalance = toLong(outputMap["ban"].c_str()) - tmpSubaccUser.Fcon;
                    
                }
                
            }
            SubaccListUser.push_back(tmpSubaccUser);
        }
    }
    else
    {
        //全部错误 或其他 CKV错误
        TRACE_ERROR("%s|ERROR GetByCol_MultiCol from ckv key=[%s] iRet=%d",__FUNCTION__,ckv_node.key.c_str(),iRet);
        querySubaccBalanceListFromSubDB(uid, subacc_curtype_list, SubaccListUser);
        /*throw (CException(ERR_SUBACC_CKV_ERROR,"系统错误,CKV获取数据失败", __FILE__, __LINE__));*/
    }
  
}

void querySubaccBalanceListFromSubDB(int uid, const vector<int> & subacc_curtype_list, vector<SubaccUser> &SubaccListUser)
{
    string subacc_db;
    string subacc_ttc;
    getSubaccdbAndSubaccttc(uid, subacc_db,subacc_ttc);

    for(size_t i=0; i<subacc_curtype_list.size(); i++)
    {
        int64_t balance = 0;
        int64_t con = 0;
        SubaccUser tmpSubaccUser;

        //以上步骤查询失败则查询子账户DB
        if(!querySubaccUserInfo(subaccDB[subacc_db], uid, subacc_curtype_list[i], balance,con))
        {
	        //没查询到记录，什么都不做，balance=0
            TRACE_DEBUG("Fbalance no find in TTC DB uid[%d], curtype[%d]",uid, subacc_curtype_list[i]);
        }

        TRACE_DEBUG("Fbalance from db[%ld],Fcon from db[%ld]", balance, con);
        tmpSubaccUser.Fuid = uid;
        tmpSubaccUser.Fcurtype = subacc_curtype_list[i];
        tmpSubaccUser.Fbalance = balance;
        tmpSubaccUser.Fcon = con;
        SubaccListUser.push_back(tmpSubaccUser);
    }
}