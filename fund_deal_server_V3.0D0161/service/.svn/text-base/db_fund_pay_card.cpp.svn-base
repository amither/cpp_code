#include "db_fund_pay_card.h"
#include "db_fund_bank_config.h"
#include "dbsign.h"
#include "db_code.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 






bool queryFundPayCard(CMySQL* pMysql, FundPayCard& data,  bool lock) //标题
{
    int iRet = 0;
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ftrade_id,Fqqid,Fuid,Fbind_serialno,Fbank_type, "
                    " Fcard_tail,Fbank_id,Fmobile,Fcreate_time,Fmodify_time,Fstandby3 "
                    " FROM fund_db.t_fund_pay_card "
                    " WHERE "
                    " Fqqid='%s' " 
                    " %s ",
                    pMysql->EscapeStr(data.Fqqid).c_str(),
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
            strncpy(data.Ftrade_id,row[0] ? row[0] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fqqid,row[1] ? row[1] : "", sizeof(data.Fqqid) - 1);
            data.Fuid = row[2] ? atoi(row[2]) : 0;
            strncpy(data.Fbind_serialno,row[3] ? row[3] : "", sizeof(data.Fbind_serialno) - 1);
            data.Fbank_type = row[4] ? atoi(row[4]) : 0;
			strncpy(data.Fcard_tail,row[5] ? row[5] : "", sizeof(data.Fcard_tail) - 1);
            strncpy(data.Fbank_id,row[6] ? row[6] : "", sizeof(data.Fbank_id) - 1);

            //长度大于DB_DECODE_CRE_ID_MAX_LENGTH表示加密后的数据才需解密，否则直接返回DB明文数据
            //由于mobile字段外部使用场景很少，故解码错误的异常暂时只记录错误，不抛异常
            if( DB_DECODE_MOBLIE_MAX_LENGTH < strlen(row[7]) ){
                string strDeTo;
                if((iRet = lct_decode(row[0], row[7], strDeTo))){
                    gPtrAppLog->error("[%s][%d] lct_decode Fmobile error! lct_decode db_mobile=%s iRet=%d", 
                                        __FILE__,__LINE__, row[7],iRet);
                }
                strncpy(data.Fmobile, ValiStr((char*)strDeTo.c_str()), sizeof(data.Fmobile)-1);
            }else{
                strncpy(data.Fmobile,row[7] ? row[7] : "", sizeof(data.Fmobile) - 1);
            }
    
            //strncpy(data.Fmobile,row[7] ? row[7] : "", sizeof(data.Fmobile) - 1);
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fsign,row[10] ? row[10] : "", sizeof(data.Fsign) - 1);
            checkSign( "t_fund_pay_card", data);
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



void insertFundPayCard(CMySQL* pMysql, FundPayCard &data )
{
    int iRet = 0;
    string strEncodeFmobile;
    
    //配置开关:是否写入加密后的敏感数据
    if( 1 == gPtrConfig->m_AppCfg.db_encode_switch )    
    {
        if((iRet = lct_encode(data.Ftrade_id, data.Fmobile, strEncodeFmobile))){
            strEncodeFmobile.assign(data.Fmobile); //加密失败则仍使用原串写入DB
            gPtrAppLog->error("[%s][%d] lct_encode Fmobile error! lct_encode iRet=%d", __FILE__,__LINE__, iRet);
        }else{
            //记录成功加密后密文数据
            gPtrAppLog->debug("[%s][%d] encode mobile data result: Ftrade_id[%s] Fmobile_encode[%s]",
                                __FILE__,__LINE__, data.Ftrade_id, strEncodeFmobile.c_str());
        }
    }
    
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_pay_card("
                    " Ftrade_id,Fqqid,Fuid,Fbind_serialno,Fbank_type, "
                    " Fcard_tail,Fbank_id,Fmobile,Fcreate_time,Fmodify_time,Fstandby3)"
                    " VALUES("
                    " '%s','%s',%d,'%s',%d, "
                    " '%s','%s','%s','%s','%s','%s')",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fqqid).c_str(),
                    data.Fuid,
                    pMysql->EscapeStr(data.Fbind_serialno).c_str(),
                    data.Fbank_type,
                    pMysql->EscapeStr(data.Fcard_tail).c_str(),
                    pMysql->EscapeStr(data.Fbank_id).c_str(),
                    1 == gPtrConfig->m_AppCfg.db_encode_switch?pMysql->EscapeStr(strEncodeFmobile).c_str():pMysql->EscapeStr(data.Fmobile).c_str(),
                    //pMysql->EscapeStr(data.Fmobile).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_pay_card", data)).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}


  

/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundPayCard(CMySQL* pMysql, FundPayCard& data )
{
    int iRet = 0;
    string strEncodeFmobile;
    
    //配置开关:是否写入加密后的敏感数据
    if( 1 == gPtrConfig->m_AppCfg.db_encode_switch )
    {
        if((iRet = lct_encode(data.Ftrade_id, data.Fmobile, strEncodeFmobile))){
            strEncodeFmobile.assign(data.Fmobile); //加密失败则仍使用原串写入DB
            gPtrAppLog->error("[%s][%d] lct_encode Fmobile error! lct_encode iRet=%d", __FILE__,__LINE__, iRet);
        }else{
            //记录成功加密后密文数据
            gPtrAppLog->debug("[%s][%d] encode data result: Ftrade_id[%s] Fmobile_encode[%s]", 
                                __FILE__,__LINE__, data.Ftrade_id, strEncodeFmobile.c_str());
        }
    }
    
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_pay_card SET "
                    " Fbind_serialno='%s',"
                    " Fbank_type=%d,"
                    " Fcard_tail='%s',"
                    " Fbank_id='%s',"
                    " Fmobile='%s',"
                    " Fmodify_time='%s', "
                    " Fstandby3='%s' "
                    " WHERE "
                    " Fqqid='%s'", 
                    pMysql->EscapeStr(data.Fbind_serialno).c_str(),
                    data.Fbank_type,
                    pMysql->EscapeStr(data.Fcard_tail).c_str(),
                    pMysql->EscapeStr(data.Fbank_id).c_str(),
                    1 == gPtrConfig->m_AppCfg.db_encode_switch?pMysql->EscapeStr(strEncodeFmobile).c_str():pMysql->EscapeStr(data.Fmobile).c_str(),
                    //pMysql->EscapeStr(data.Fmobile).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_pay_card", data)).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.Fqqid).c_str()
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

bool queryBindCardInfo(string uin, string bind_serialno, map<string,string> &resMap)
{
	char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};

	string spid = getDefSpid();
	SET_PARAM(szMsg, "MSG_NO", MSG_NO, true);
	SET_PARAM(szMsg, "qqid", uin.c_str());
    SET_PARAM(szMsg, "bind_serialno",bind_serialno.c_str());
	SET_PARAM(szMsg, "offset","0");
	SET_PARAM(szMsg, "limit","1");
	SET_PARAM(szMsg, "bind_type_cond","debit_click|credit_bind_pay");
	SET_PARAM(szMsg, "sp_id",spid.c_str());

	// 发送请求
    gPtrBindQueryRpc->excute("bind_textquery_service", szMsg, strlen(szMsg), szBuf, oLen);

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));
	if(iResult != 0)
	{
		throw EXCEPTION(iResult, szResInfo);
	}

	TStr2StrMap resultMap;
	Tools::StrToMap(resultMap,szBuf,"&","=",1);
	int ret_num = toInt(resultMap["ret_num"].c_str());
	
    if(ret_num != 1)
    {
		return false;
    }

	string itermStr = resultMap["row_1"];
	Tools::StrToMap(resMap,itermStr,"&","=",1);

	string bind_status = resMap["bind_status"];
	//	关联状态：	0 - 未定义	1 - 初始状态	2 - 开启	3 - 关闭（支付关闭，可以提现）	4 - 解除	5 - 银行已激活，用户未激活（兴业银行3001）
	if(bind_status != "2" && bind_status != "3" && bind_status != "4" )
	{
		return false;
	}

	string bank_status = resMap["bank_status"];
	//	银行绑定过程状态：	0 - 未定义 	1 - 预绑定状态(未激活)	2 - 绑定确认(正常)	3 - 解除绑定
	if(bank_status != "2" && bank_status != "3")
	{
		return false;
	}
	
	return true;
}

bool checKifSafePayCardWhiteListUser(const string &uin)
{
    if (gPtrConfig->m_AppCfg.multycardbuy_allow_all_switch)
    {
        return true;
    }
    string whiteListValue=getUserWhiteListValue(uin);
    if (whiteListValue.length()>=2)
    {
        if (whiteListValue[1]=='1') //第二个字节标识对卡进单卡出白名单
        {
            return true;
        }
    }

    return false;
}

/**
* 检查支付使用相同的卡，及绑定序列号相同
* 入参bind_serialno为空返回false
* 本地数据库里没有用户的支付记录则使用bind_serialno查询绑定信息存在且有效，解绑也认为有效，则记录本地数据库，返回认证通过
* 本地数据库有且和传入的bind_serialno一致则返回认证通过,不一致判断卡号是否一致，一致则返回认证通过，否则返回认证失败
*/
bool checkPayCard(CMySQL* pMysql, FundPayCard& fund_pay_card, string bind_serialno,TStr2StrMap &bindCareInfo)
{
	if(bind_serialno.empty())
	{
		return false;
	}

	bool isFundPayCard = queryFundPayCard(pMysql,fund_pay_card, false);

	//如果存在且绑定序列号一致
	if(isFundPayCard && bind_serialno == fund_pay_card.Fbind_serialno)
	{
		return true;
	}

    //查询绑定卡
    if (bindCareInfo["bankid"].empty())
    {
        if(!queryBindCardInfo(fund_pay_card.Fqqid, bind_serialno, bindCareInfo))
        {
            gPtrAppLog->warning("bind_serialno invalid.bind_serialno[%s]", bind_serialno.c_str());
            return false;
        }
    }

	if(isFundPayCard && fund_pay_card.Fbank_id == bindCareInfo["bankid"])
	{
		gPtrAppLog->normal("bind_serialno not equal but bankid equal.");
		//银行卡号一致也认为相同卡支付
		return true;
	}


    //检查支付银行类似是否是理财通支持的银行类型
    FundBankConfig bankCfg;
    memset(&bankCfg,0,sizeof(bankCfg));
    int iBankType = atoi(bindCareInfo["bank_type"].c_str());
    bankCfg.Fbank_type= iBankType;
    if( queryFundBankConfig(pMysql, bankCfg,false) == false)
    {
        char szErrMsg[128];
        gPtrAppLog->error("checkPayCard bank type not configured=%d,qqid=%s,bind_serialno=%s",
				iBankType,fund_pay_card.Fqqid,bind_serialno.c_str());
        snprintf(szErrMsg, sizeof(szErrMsg), "支付成功后申购时判断支付银行卡bank_type=%d没配置,qqid=%s,bind_seriano=%s", 
				iBankType,fund_pay_card.Fqqid,bind_serialno.c_str());
        alert(ERR_PAY_BIND_CARD, szErrMsg);
        throw EXCEPTION(ERR_PAY_BIND_CARD, szErrMsg);
    }


	if(isFundPayCard)
	{
		if(!(0 == strcmp("", fund_pay_card.Fbind_serialno)))
		{
			//本地记录有绑定序列号
			if (checKifSafePayCardWhiteListUser(fund_pay_card.Fqqid))
                    {
                        gPtrAppLog->debug("white list user ,not check safe_card");
                        return true;
                    }         
			gPtrAppLog->warning("not the same card payment.bind_serialno in db[%s], bind_serialno input[%s]", fund_pay_card.Fbind_serialno, bind_serialno.c_str());
                    if (gPtrConfig->m_AppCfg.refund_for_check_bind_serailno_diff==1)
                    {
                        return false;
                    }
			else
                    {
          			//绑定序列号不一致，先告警，避免ckv有白名单错误的话，大量退款
          			string errInfo=string("安全卡绑定序列号校验失败uin:")+fund_pay_card.Fqqid+string(" db:")+fund_pay_card.Fbind_serialno+string(" ,input:")+bind_serialno;
          			alert(ERR_PAY_BIND_CARD, errInfo);
          			throw EXCEPTION(ERR_PAY_BIND_CARD, errInfo);
                    }
		}
		else
		{
			//本地绑定序列号已清除，则更新
			strncpy(fund_pay_card.Fbind_serialno, bindCareInfo["bind_serialno"].c_str(), sizeof(fund_pay_card.Fbind_serialno) - 1);
			fund_pay_card.Fbank_type = atoi(bindCareInfo["bank_type"].c_str()) ;
			strncpy(fund_pay_card.Fcard_tail, bindCareInfo["card_tail"].c_str(), sizeof(fund_pay_card.Fcard_tail) - 1);
			strncpy(fund_pay_card.Fbank_id, bindCareInfo["bankid"].c_str(), sizeof(fund_pay_card.Fbank_id) - 1);
			strncpy(fund_pay_card.Fmobile, bindCareInfo["mobilephone"].c_str(), sizeof(fund_pay_card.Fmobile) - 1);
            //修改时更新modify_time
            string systime = getSysTime();
            strncpy(fund_pay_card.Fmodify_time, systime.c_str(), sizeof(fund_pay_card.Fmodify_time) - 1);
            
			updateFundPayCard(pMysql,fund_pay_card);
			
			setPayCardToKV(pMysql, fund_pay_card);
			return true;
		}
	}
	else
	{
		//本地不存在则插入记录
		strncpy(fund_pay_card.Fbind_serialno, bindCareInfo["bind_serialno"].c_str(), sizeof(fund_pay_card.Fbind_serialno) - 1);
		fund_pay_card.Fbank_type = atoi(bindCareInfo["bank_type"].c_str()) ;
		strncpy(fund_pay_card.Fcard_tail, bindCareInfo["card_tail"].c_str(), sizeof(fund_pay_card.Fcard_tail) - 1);
		strncpy(fund_pay_card.Fbank_id, bindCareInfo["bankid"].c_str(), sizeof(fund_pay_card.Fbank_id) - 1);
		strncpy(fund_pay_card.Fmobile, bindCareInfo["mobilephone"].c_str(), sizeof(fund_pay_card.Fmobile) - 1);

		insertFundPayCard(pMysql,fund_pay_card);

		setPayCardToKV(pMysql,fund_pay_card);
		return true;
	}

}

/**
 * 组装用户安全卡ckv值
 * @param fund_pay_card 用户安卡信息
 * @param value 组装的ckv值串
 * @return 0-成功 其它-失败
 */
int packUsrPayCardCkvValue(const FundPayCard& fund_pay_card, string &value)
{
    CParams kvReqSet;

    //设置要修改的数据szValue
    kvReqSet.setParam("Ftrade_id",fund_pay_card.Ftrade_id);
	kvReqSet.setParam("Fqqid",fund_pay_card.Fqqid);
	kvReqSet.setParam("Fbind_serialno",fund_pay_card.Fbind_serialno);
	kvReqSet.setParam("Fbank_type",fund_pay_card.Fbank_type);
	kvReqSet.setParam("Fcard_tail",fund_pay_card.Fcard_tail);
	kvReqSet.setParam("Fcreate_time",fund_pay_card.Fcreate_time);
	kvReqSet.setParam("Fmodify_time",fund_pay_card.Fmodify_time);

	char buff[128] = {0};
	string card_sign = string(fund_pay_card.Fqqid) + string(fund_pay_card.Fbank_id);
	getMd5(card_sign.c_str(), card_sign.size(), buff);

	//给支付中心专用签名
	if(0 == strcmp("", fund_pay_card.Fbank_id))
	{
		kvReqSet.setParam("Fcard_sign","");
	}
	else
	{
		kvReqSet.setParam("Fcard_sign",buff);
	}

    value = kvReqSet.pack();

    return 0;
}

/**
*设置cache
*/
bool setPayCardToKV(CMySQL* mysql, FundPayCard& fund_pay_card, bool needQuery)
{

	string key = "pay_card_" + toString(fund_pay_card.Fqqid);

	if(needQuery)
	{
		bool ret = queryFundPayCard(mysql,fund_pay_card,true);
        //如果查询不到则删除key，只有用户注销后才会查询不到
        if (false == ret) {
            if(gCkvSvrOperator->del(CKV_KEY_PAY_CARD,key)) {
        		return false;
            } else {
        		return true;
        	}
        }
	}

    string szValue;
	packUsrPayCardCkvValue(fund_pay_card, szValue);

    //将szValue写入ckv
    if(gCkvSvrOperator->set(CKV_KEY_PAY_CARD, key, szValue))
    {
		return false;
    }
	else
	{
		return true;
	}
}

bool delPayCardToKV(string uin)
{
	string key = "pay_card_" + uin;

    gPtrAppLog->debug("delPayCardToKV key=%s",key.c_str());

	//将szValue写入ckv
    if(gCkvSvrOperator->del(CKV_KEY_PAY_CARD,key))
    {
		return false;
    }
	else
	{
		return true;
	}
}


/**
 * 记录安全卡更新日志流水
 * @param pMysql 
 * @param old 
 * @param new_bind_serialno 
 * @param memo 
 */
void saveFundPayCardLog(CMySQL* pMysql, const FundPayCard &old, const string &new_bind_serialno, const string &memo)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    
    string desc;
    if (memo.length() > 255)
        desc = memo.substr(0, 255);
    else
        desc = memo;
    
    // 构造SQL
    string cur_time = getSysTime();
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_pay_card_log("
                    " Ftrade_id,Fqqid,Fuid,Fbind_serialno,Fbank_type, "
                    " Fcard_tail,Fbank_id,Fmobile,Fnew_bind_serialno,Fmemo,Fcreate_time,Fmodify_time)"
                    " VALUES("
                    " '%s','%s',%d,'%s',%d, "
                    " '%s','%s','%s','%s','%s','%s','%s')",
                    pMysql->EscapeStr(old.Ftrade_id).c_str(),
                    pMysql->EscapeStr(old.Fqqid).c_str(),
                    old.Fuid,
                    pMysql->EscapeStr(old.Fbind_serialno).c_str(),
                    old.Fbank_type,
                    pMysql->EscapeStr(old.Fcard_tail).c_str(),
                    pMysql->EscapeStr(old.Fbank_id).c_str(),
                    pMysql->EscapeStr(old.Fmobile).c_str(),
                    pMysql->EscapeStr(new_bind_serialno).c_str(),
                    pMysql->EscapeStr(desc).c_str(),
                    pMysql->EscapeStr(cur_time).c_str(),
                    pMysql->EscapeStr(cur_time).c_str() );
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

void disableFundPayCard(CMySQL* pMysql, const FundPayCard &data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_pay_card SET "
                    " Fqqid ='%s', "
                    " Fbind_serialno='',"
                    " Fbank_type=0,"
                    " Fcard_tail='',"
                    " Fbank_id='',"
                    " Fmobile='', "
                    " Fstandby3='%s' "
                    " WHERE "
                    " Fqqid='%s'", 
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_pay_card", data)).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.Fqqid).c_str()
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

void recoveryFundPayCard(CMySQL* pMysql, const FundPayCard &data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_pay_card SET "
                    " Fqqid ='%s',Fsign ='%s' "
                    " WHERE "
                    " Ftrade_id='%s'", 
                    pMysql->EscapeStr(data.Fqqid).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_pay_card", data)).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.Ftrade_id).c_str()
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


