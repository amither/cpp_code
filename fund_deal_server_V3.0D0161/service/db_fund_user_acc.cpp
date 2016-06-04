#include "db_fund_user_acc.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

// 连接基金数据库句柄
extern CMySQL* gPtrFundDB;
extern CftLog* gPtrCkvErrorLog; // 更新ckv出错日志

int getUserAcc(FundUserAcc& queryUserAcc, vector<FundUserAcc> & userAccVec)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
    string key = "fund_user_acc_"+toString(queryUserAcc.Ftrade_id);

    //取kv数据
    CParams kvRspGet;
    int ret = gCkvSvrOperator->get(key, kvRspGet);

	//分基金收益信息
	int total_num = 0 ;
	if(ret==0){
		total_num = atoi(kvRspGet.getString("total_num").c_str());
	}

	int j=-1;
	for(int i = 0; i < total_num; i++)
	{
		FundUserAcc userAcc;
		memset(&userAcc,0,sizeof(FundUserAcc));
		strncpy(userAcc.Ffund_code, kvRspGet.getString(string("fund_code_" + toString(i)).c_str()).c_str(), sizeof(userAcc.Ffund_code) - 1);
		userAcc.Ftype=kvRspGet.getInt(string("type_" + toString(i)).c_str());
		userAcc.Fconfirm=kvRspGet.getInt(string("confirm_" + toString(i)).c_str());
		userAcc.Fcloseid= kvRspGet.getLong(string("closeid_" + toString(i)).c_str());
		strncpy(userAcc.Ftime, kvRspGet.getString(string("time_" + toString(i)).c_str()).c_str(), sizeof(userAcc.Ftime) - 1);
		strncpy(userAcc.Ftrade_id,queryUserAcc.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
		userAcc.Fidx=i+1;
		userAccVec.push_back(userAcc);
		
		if(0==strcmp(queryUserAcc.Ffund_code,userAcc.Ffund_code)&&userAcc.Fcloseid==queryUserAcc.Fcloseid)
		{
			j=i;
		}
	}

	return j;
    
}

int getUserAccMap(const string& trade_id, map<string,FundUserAcc> & userAccMap)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
    string key = "fund_user_acc_"+trade_id;

    //取kv数据
    CParams kvRspGet;
    int ret = gCkvSvrOperator->get(key, kvRspGet);

	//分基金收益信息
	int total_num = 0 ;
	if(ret==0){
		total_num = atoi(kvRspGet.getString("total_num").c_str());
	}
	userAccMap.clear();
	for(int i = 0; i < total_num; i++)
	{
		FundUserAcc userAcc;
		memset(&userAcc,0,sizeof(FundUserAcc));
		string fund_code = kvRspGet.getString(string("fund_code_" + toString(i)).c_str());
		strncpy(userAcc.Ffund_code, fund_code.c_str(), sizeof(userAcc.Ffund_code) - 1);
		userAcc.Ftype=kvRspGet.getInt(string("type_" + toString(i)).c_str());
		userAcc.Fconfirm=kvRspGet.getInt(string("confirm_" + toString(i)).c_str());
		userAcc.Fcloseid= kvRspGet.getLong(string("closeid_" + toString(i)).c_str());
		strncpy(userAcc.Ftime, kvRspGet.getString(string("time_" + toString(i)).c_str()).c_str(), sizeof(userAcc.Ftime) - 1);
		strncpy(userAcc.Ftrade_id,trade_id.c_str(),sizeof(userAcc.Ftrade_id)-1);
//		userAcc.Ftotal_fee= kvRspGet.getLong(string("total_fee_" + toString(i)).c_str());
//		userAcc.Fidx = kvRspGet.getInt(string("idx_" + toString(i)).c_str());
		userAcc.Fidx=i+1;

		// 取fund_code+closeid为key
		string mapKey = fund_code+"_"+toString(userAcc.Fcloseid);
		userAccMap[mapKey]=userAcc;		
	}

	return userAccMap.size();
    
}

/**
 * 使用排序idx
 */
bool setUserAcc(vector<FundUserAcc>& userAccVec)
{
	gPtrAppLog->debug("setUserAcc ckv. userAccVec.size=[%d]", userAccVec.size());
	if(userAccVec.size()<=0){
		return false;
	}
    string key = "fund_user_acc_"+ string(userAccVec[0].Ftrade_id);

	string szValue;
	CParams kvReqSet;

	char szParaName[64] = {0};
	    
	//设置要修改的数据szValue
	for(vector<FundUserAcc>::size_type i= 0; i != userAccVec.size(); ++i)
	{
		FundUserAcc& userAcc = userAccVec[i];
		int j = userAcc.Fidx-1;
		
		snprintf(szParaName, sizeof(szParaName), "idx_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Fidx);

		snprintf(szParaName, sizeof(szParaName), "fund_code_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Ffund_code);
		
		snprintf(szParaName, sizeof(szParaName), "type_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Ftype);
		
		/*
				snprintf(szParaName, sizeof(szParaName), "confirm_%zd", i);
				kvReqSet.setParam(szParaName, userAcc.Fconfirm);
		*/

		snprintf(szParaName, sizeof(szParaName), "closeid_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Fcloseid);
		
		snprintf(szParaName, sizeof(szParaName), "time_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Ftime);
		
//		snprintf(szParaName, sizeof(szParaName), "total_fee_%zd", i);
//		kvReqSet.setParam(szParaName, userAcc.Ftotal_fee);
		

	}
		
	kvReqSet.setParam("total_num",(int)(userAccVec.size()));
    szValue = kvReqSet.pack();
	
	gPtrAppLog->debug("setUserAcc ckv. key=[%s] value=[%s]", key.c_str(), szValue.c_str());

    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_FUND_USER_ACC,key, szValue))
	{
		// 重试
		int ret =gCkvSvrOperator->set(CKV_KEY_FUND_USER_ACC,key, szValue);
		return ret==0;
	}
	else
	{
		return true;
	}
}
/**
  * 使用自然idx
  */
bool setUserAccVec(vector<FundUserAcc>& userAccVec)
{
	gPtrAppLog->debug("setUserAcc ckv. userAccVec.size=[%d]", userAccVec.size());
	if(userAccVec.size()<=0){
		return false;
	}
    string key = "fund_user_acc_"+ string(userAccVec[0].Ftrade_id);

	string szValue;
	CParams kvReqSet;

	char szParaName[64] = {0};
	    
	//设置要修改的数据szValue
	for(vector<FundUserAcc>::size_type i= 0; i != userAccVec.size(); ++i)
	{
		FundUserAcc& userAcc = userAccVec[i];
		
		snprintf(szParaName, sizeof(szParaName), "idx_%zd", i);
		kvReqSet.setParam(szParaName, userAcc.Fidx);

		snprintf(szParaName, sizeof(szParaName), "fund_code_%zd", i);
		kvReqSet.setParam(szParaName, userAcc.Ffund_code);
		
		snprintf(szParaName, sizeof(szParaName), "type_%zd", i);
		kvReqSet.setParam(szParaName, userAcc.Ftype);
/*
		snprintf(szParaName, sizeof(szParaName), "confirm_%zd", i);
		kvReqSet.setParam(szParaName, userAcc.Fconfirm);
*/
		snprintf(szParaName, sizeof(szParaName), "closeid_%zd", i);
		kvReqSet.setParam(szParaName, userAcc.Fcloseid);
		
		snprintf(szParaName, sizeof(szParaName), "time_%zd", i);
		kvReqSet.setParam(szParaName, userAcc.Ftime);

	}
		
	kvReqSet.setParam("total_num",(int)(userAccVec.size()));
    szValue = kvReqSet.pack();
	
	gPtrAppLog->debug("setUserAcc ckv. key=[%s] value=[%s]", key.c_str(), szValue.c_str());

    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_FUND_USER_ACC,key, szValue))
	{
		// 重试
		int ret =gCkvSvrOperator->set(CKV_KEY_FUND_USER_ACC,key, szValue);
		return ret==0;
	}
	else
	{
		return true;
	}
}

// 仅用于对账补单,idx在函数里面设置
bool setUserAccMap(string tradeId, map<string, FundUserAcc>& userAccMap)
{
	gPtrAppLog->debug("setUserAcc ckv. userAccVec.size=[%d]", userAccMap.size());
	if(userAccMap.size()<=0){
		return false;
	}
    string key = "fund_user_acc_"+ tradeId;

	string szValue;
	CParams kvReqSet;

	char szParaName[64] = {0};
	    
	//设置要修改的数据szValue
	int j = 0;
	for(map<string,FundUserAcc>::iterator it = userAccMap.begin(); it != userAccMap.end();it++ )
	{
		FundUserAcc& userAcc = it->second;
		
		snprintf(szParaName, sizeof(szParaName), "fund_code_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Ffund_code);
		
		snprintf(szParaName, sizeof(szParaName), "type_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Ftype);
		/*
		snprintf(szParaName, sizeof(szParaName), "confirm_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Fconfirm);
		*/
		snprintf(szParaName, sizeof(szParaName), "closeid_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Fcloseid);
		
		snprintf(szParaName, sizeof(szParaName), "time_%d", j);
		kvReqSet.setParam(szParaName, userAcc.Ftime);
		
		snprintf(szParaName, sizeof(szParaName), "idx_%d", j);
		kvReqSet.setParam(szParaName, ++j); // idx加一
		

	}
		
	kvReqSet.setParam("total_num",(int)(userAccMap.size()));
    szValue = kvReqSet.pack();
	
	gPtrAppLog->debug("setUserAcc ckv. key=[%s] value=[%s]", key.c_str(), szValue.c_str());

    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_FUND_USER_ACC,key, szValue))
	{
		// 重试
		int ret =gCkvSvrOperator->set(CKV_KEY_FUND_USER_ACC,key, szValue);
		return ret==0;
	}
	else
	{
		return true;
	}
}


/**
 * 更新用户份额,重新排序
*/
bool delUserAcc(const string& tradeId){
	// 全部删除需要手工清空key
    string key = "fund_user_acc_"+ tradeId;
	gPtrAppLog->debug("setUserAcc ckv. key=[%s]", key.c_str());
	int ret=0;
	// 重试
	if(gCkvSvrOperator->del(CKV_KEY_FUND_USER_ACC, key)){
		ret = gCkvSvrOperator->del(CKV_KEY_FUND_USER_ACC, key);
	}
	
	return ret==0;
}


/**
 * 增加用户份额
*/
bool addUserAcc(FundUserAcc& userAcc, bool resort)
{
	vector<FundUserAcc> userAccVec;
	int j = getUserAcc(userAcc,userAccVec);
	int idx = 0;
	if(j<0){ // 没有数据,则需要重新排序
		resort=true;
		idx = userAccVec.size()+1;
	}else{
		idx=userAccVec[j].Fidx;
	}

	for(vector<FundUserAcc>::size_type i= 0; i < userAccVec.size(); i++)
	{
		FundUserAcc& item=userAccVec[i];
		if(item.Fidx<idx&&resort){	
			item.Fidx=item.Fidx+1;
		}
/*		 不处理金额
		else if(item.Fidx==idx){
			userAcc.Ftotal_fee += item.Ftotal_fee;
			item.Ftotal_fee=userAcc.Ftotal_fee;
		} */
	}
	if(resort){
		if(j>=0){ // 重排序:先删除老数据,再insert
			userAccVec.erase(userAccVec.begin()+j);
		}
		userAcc.Fidx=1;
		userAccVec.insert(userAccVec.begin(),userAcc);
	}
	return setUserAcc(userAccVec);
}

/**
 * 更新用户份额,重新排序
*/
bool minusUserAcc(FundUserAcc& userAcc)
{
	vector<FundUserAcc> userAccVec;
	int j = getUserAcc(userAcc,userAccVec);
	int idx = j>=0?userAccVec[j].Fidx:userAccVec.size()+1;
	for(vector<FundUserAcc>::size_type i= 0; i < userAccVec.size(); i++)
	{
		FundUserAcc& item=userAccVec[i];
		if(item.Fidx<idx){
			item.Fidx=item.Fidx+1;
		}
/*		 不处理金额
		else if(item.Fidx==idx){
			userAcc.Ftotal_fee -= item.Ftotal_fee;
		} */
	}
	if(j>=0){
		userAccVec.erase(userAccVec.begin()+j);
	}
	userAcc.Fidx=1;
	userAccVec.insert(userAccVec.begin(),userAcc);
	return setUserAcc(userAccVec);
}

/**
 * 更新用户份额,重新排序
*/
bool removeUserAcc(FundUserAcc& userAcc)
{
	vector<FundUserAcc> userAccVec;
	int j = getUserAcc(userAcc,userAccVec);
	if(j<0||j>=(int)userAccVec.size()){		
        gPtrAppLog->warning("[%s][%d]delUserAcc,userAcc not exist[%s][%s]",__FILE__,__LINE__,userAcc.Ftrade_id,userAcc.Ffund_code);
		return false;
	}
	int idx = userAccVec[j].Fidx;
	for(vector<FundUserAcc>::size_type i= 0; i < userAccVec.size(); i++)
	{
		FundUserAcc& item=userAccVec[i];
		if(item.Fidx>idx){
			item.Fidx=item.Fidx-1;
		}
	}
	userAccVec.erase(userAccVec.begin()+j);
	if(userAccVec.size()>0){
		return setUserAcc(userAccVec);
	}else{
		return delUserAcc(userAcc.Ftrade_id);
	}
	
	
}

/**
 * 更新useracc.Fconfirm待确认申购标志
 * 0X01表示有申购待待确认金额
 * bizId=closelistId
 * needConfirm=true: 存在待确认金额
 */
bool updateUserAccConfirm4Buy(const char* fundCode, const char* tradeId, int bizId, bool needConfirm) throw (CException)
{
	FundUserAcc userAcc;
	memset(&userAcc, 0, sizeof(userAcc));
	strncpy(userAcc.Ftrade_id, tradeId, sizeof(userAcc.Ftrade_id)-1);
	strncpy(userAcc.Ffund_code, fundCode, sizeof(userAcc.Ffund_code)-1);
	userAcc.Fcloseid = bizId;
	userAcc.Ftype = FUND_USER_ACC_TYPE_BUY;
	
	vector<FundUserAcc> userAccVec;
	int j = getUserAcc(userAcc,userAccVec);
	int confirm = 0;
	if(j>=0){
		confirm = userAccVec[j].Fconfirm;
		userAccVec.erase(userAccVec.begin()+j);
	}
	if(needConfirm)  
	{	// 存在待确认申购,标记申购位为1
		userAcc.Fconfirm = confirm|USER_ACC_NEED_CONFIRM_BUY;
	}else  
	{	// 不存在待确认申购,标记申购位为0
		userAcc.Fconfirm = confirm&(~USER_ACC_NEED_CONFIRM_BUY);
	}
	userAccVec.insert(userAccVec.begin(),userAcc);
	return setUserAccVec(userAccVec);	
}

/**
 * 更新useracc.Fconfirm待确认赎回标志
 * 0X10表示有赎回待待确认份额
 * bizId=closelistId
 * needConfirm=true: 存在待确认份额
 */

bool updateUserAccConfirm4Redeem(const char* fundCode, const char* tradeId, int bizId, bool needConfirm) throw (CException)
{
	FundUserAcc userAcc;
	memset(&userAcc, 0, sizeof(userAcc));
	strncpy(userAcc.Ftrade_id, tradeId, sizeof(userAcc.Ftrade_id)-1);
	strncpy(userAcc.Ffund_code, fundCode, sizeof(userAcc.Ffund_code)-1);
	userAcc.Fcloseid = bizId;
	userAcc.Ftype = FUND_USER_ACC_TYPE_REDEM;
	
	vector<FundUserAcc> userAccVec;
	int j = getUserAcc(userAcc,userAccVec);
	int confirm = 0;
	if(j>=0){
		confirm = userAccVec[j].Fconfirm;
		userAccVec.erase(userAccVec.begin()+j);
	}
	if(needConfirm)  
	{	// 存在待确认申购,标记赎回位为1
		userAcc.Fconfirm = confirm|USER_ACC_NEED_CONFIRM_REDEEM;
	}else  
	{	// 不存在待确认申购,标记赎回位为0
		userAcc.Fconfirm = confirm&(~USER_ACC_NEED_CONFIRM_REDEEM);
	}
	userAccVec.insert(userAccVec.begin(),userAcc);
	return setUserAccVec(userAccVec);
}


void updateUserAcc(const ST_TRADE_FUND& stTrade) throw (CException){
	
	//更新子账户购买记录信息到ckv
	FundUserAcc userAcc;
	memset(&userAcc, 0, sizeof(userAcc));
	gPtrAppLog->debug("stTrade.Ftrade_id [%s], state[%s]", stTrade.Ftrade_id, stTrade.Flistid);
	strncpy(userAcc.Ftrade_id, stTrade.Ftrade_id, sizeof(userAcc.Ftrade_id)-1);
	strncpy(userAcc.Ffund_code, stTrade.Ffund_code, sizeof(userAcc.Ffund_code)-1);
	strncpy(userAcc.Ftime, stTrade.Facc_time, sizeof(userAcc.Ftime)-1);
	// userAcc.Ftotal_fee = stTrade.Ftotal_fee;
	userAcc.Fcloseid = stTrade.Fclose_listid;
	bool result = false;
	if(PURCHASE_STATE_ORDER[stTrade.Fstate]>=PURCHASE_STATE_ORDER[PAY_OK]){
		userAcc.Ftype = FUND_USER_ACC_TYPE_BUY;
		// userAcc.Fconfirm = needConfirm?1:0;
		result = addUserAcc(userAcc,true);
	// 13状态之后的允许变更
	}else if(REDEEM_STATE_ORDER[stTrade.Fstate]>=REDEEM_STATE_ORDER[REDEEM_INFO_SUC]){
		userAcc.Ftype = FUND_USER_ACC_TYPE_REDEM;
		// userAcc.Fconfirm = needConfirm?1:0;
		result = minusUserAcc(userAcc);
	}else{
		// 只有申购成功和赎回成功的状态才能更新
		gPtrAppLog->error("unexcept status in updateUserAcc [%s], state[%d]", stTrade.Flistid, stTrade.Fstate);
		throw CException(ERR_SET_CKV, "updateUserAcc CKV fail! ", __FILE__, __LINE__);
		
	}
	if(result==false){
		gPtrAppLog->error("updateUserAcc fail[%s], state[%d]", stTrade.Flistid, stTrade.Fstate);
		throw CException(ERR_SET_CKV, "updateUserAcc CKV fail! ", __FILE__, __LINE__);
	}
}
/**
 活期的逻辑:判断用户当前余额和在途金额
 定期的逻辑
*  1,  查询用户当日有效的定期期次
*  2,  16点30后排除今日收益截至的数据
*/

bool setUserAccToKV(CMySQL* mysql, const string& tradeId, bool needUpdate) throw (CException)
{
	bool ret=false;
	try{		
		string time = GetTimeToday();
		bool isDayEnd = true; // 16点后认为是日终
		if(time<"16:30:00"){
			isDayEnd=false;
		}
		
		ST_FUND_BIND pstRecord;
		memset(&pstRecord,0,sizeof(ST_FUND_BIND));
		strncpy(pstRecord.Ftrade_id,tradeId.c_str(),sizeof(pstRecord.Ftrade_id)-1);
	    mysql->Begin();
		//锁定总余额表，防止用户并发申购的数据没有写入CKV	
		if(!QueryFundBindByTradeid(mysql,tradeId.c_str(),&pstRecord,true,false)){			
        	throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
		}
		
		// 查询用户CKV map
		map<string,FundUserAcc> ckvMap;
		getUserAccMap(tradeId, ckvMap);
		
		map < string,FundUserAcc > dbMap;
		// 通过收益和交易查询活期遗漏增加的CKV数据
		bool isOpenEqual = checkOpenTransUserAcc(mysql,pstRecord,ckvMap,dbMap,isDayEnd);
		
		// 通过定期交易查询定期遗漏增加的CKV数据
		bool isCloseEqual = checkCloseTransUserAcc(mysql,tradeId,ckvMap,dbMap,isDayEnd);

		// 检查是否有多余的数据
		bool isAllCheck = isUserAccKVAllCheck(ckvMap);
		
		// 需要变更,则更新CKV
		if(!isOpenEqual||!isCloseEqual||!isAllCheck){
			char errbuf[256]={0};
			snprintf(errbuf, sizeof(errbuf), "[setUserAccToKV]ckv_unconsistant[%s]DB[%zd]CKV[%zd]",tradeId.c_str(),dbMap.size(),ckvMap.size());
			if(!needUpdate){ // 无需变更,仅抛出告警
	        	throw CException(ERR_CKV_UNCONSIST, errbuf, __FILE__, __LINE__);
			}
        	TRACE_DEBUG("%s",errbuf);
			if(dbMap.size()>0){
				ret=setUserAccMap(tradeId,dbMap);
			}else{
				ret=delUserAcc(tradeId);
			}
		}else{
        	TRACE_DEBUG("[setUserAccToKV]ckv_consistant[%s]DB[%zd]CKV[%zd]",tradeId.c_str(),dbMap.size(),ckvMap.size());
			ret=true;
		}
		mysql->Commit();
	}
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
        mysql->Rollback();
		throw;
    }
	return ret;
	
}
bool addUseSubAccToKV(CMySQL* mysql,const string& trade_id,string& spid) throw (CException)
{
	SpConfigCache sp_config;
	querySpConfigCache(mysql,spid,sp_config);
	
	// 检查是否有份额
	ST_FUND_BIND pstRecord;
	memset(&pstRecord,0,sizeof(ST_FUND_BIND));
	strncpy(pstRecord.Ftrade_id,trade_id.c_str(),sizeof(pstRecord.Ftrade_id)-1);
	QueryFundBindByTradeid(gPtrFundSlaveDB,trade_id.c_str(),&pstRecord);
	if(pstRecord.Fuid==0)
		return false;		
	// 查询子账户
	LONG freezeBalance=0;
	LONG totalfee = querySubaccBalance(pstRecord.Fuid,sp_config.curtype,false,&freezeBalance);
	// 查询未确认资产表
	bool hasUnconfirm = false;
	if(sp_config.buy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
		if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
		{
			hasUnconfirm = queryFundUnconfirmExists(mysql,spid,pstRecord.Ftrade_id);
		}else{
			hasUnconfirm = queryUnfinishTransExistsBySp(mysql,spid,pstRecord.Ftrade_id);
		}
	}
	
	if(totalfee<=0&&freezeBalance<=0&&!hasUnconfirm)
		return false;
	// 查询现有CKV
	map<string,FundUserAcc> ckvMap;
	getUserAccMap(trade_id, ckvMap);
	string normalKey = sp_config.fund_code+"_0";
	if(ckvMap.find(normalKey)!=ckvMap.end()){
		return true;
	}
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};
    GetTimeNow(szTimeNow);
	int idx = 1;
	vector<FundUserAcc> userAccVec;
	// 添加活期
	if(sp_config.close_flag==CLOSE_FLAG_NORMAL){
		FundUserAcc userAcc;
		memset(&userAcc, 0, sizeof(FundUserAcc));
		strncpy(userAcc.Ftrade_id,trade_id.c_str(),sizeof(userAcc.Ftrade_id)-1);
		strncpy(userAcc.Ffund_code,sp_config.fund_code.c_str(),sizeof(userAcc.Ffund_code)-1);
		strncpy(userAcc.Ftime ,szTimeNow,sizeof(userAcc.Ftime)-1);		
		// 检查存在未确认份额
		/* int confirm = 0;
		if(sp_config.buy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)
		{
			bool hasUnconfirm = queryFundUnconfirmExists(mysql,spid,pstRecord.Ftrade_id);
			if(!hasUnconfirm) // 检查存在未确认的赎回份额
			{
				hasUnconfirm = isExistsUnconfirmRedemtion(mysql,pstRecord.Fuid,spid);
			}
			confirm = hasUnconfirm?1:0;;
		} 
		userAcc.Fconfirm=confirm; */
		userAcc.Ftotal_fee = totalfee;
		userAcc.Fidx = idx++;
		userAccVec.push_back(userAcc);
	// 添加定期
	}else if(sp_config.close_flag==CLOSE_FLAG_ALL_CLOSE){
		vector< FundCloseTrans> closeVec;
		string checkDate = toString(GetDateToday());
		queryFundCloseTransForRegProfit(mysql,trade_id,sp_config.fund_code,checkDate,closeVec,false);
		for(vector<FundCloseTrans>::size_type j=0;j<closeVec.size();j++){
			FundCloseTrans& closeTrans = closeVec[j];			
			string closeKey = sp_config.fund_code+"_"+toString(closeTrans.Fid);
			if(ckvMap.find(closeKey)!=ckvMap.end()){
				continue;
			}				
			FundUserAcc userAcc;
			memset(&userAcc, 0, sizeof(FundUserAcc));
			strncpy(userAcc.Ftrade_id,closeTrans.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
			strncpy(userAcc.Ffund_code,sp_config.fund_code.c_str(),sizeof(userAcc.Ffund_code)-1);
			strncpy(userAcc.Ftime ,closeTrans.Facc_time,sizeof(userAcc.Ftime)-1);
			userAcc.Ftotal_fee = closeTrans.Fcurrent_total_fee;
			userAcc.Fcloseid = closeTrans.Fid;
			userAcc.Fidx = idx++;
			userAccVec.push_back(userAcc);
		}
	}else{
		// 不支持类型
		gPtrAppLog->error("unsupport Close Flag[%s], flag[%d]", sp_config.spid.c_str(), sp_config.close_flag);
		return false;
	}
	for(map<string,FundUserAcc>::iterator it = ckvMap.begin(); it != ckvMap.end();it++ )
    {
    	FundUserAcc& userAcc = it->second;
		if(userAcc.Ffund_code[0]=='\0'){
			continue; // 删除垃圾数据
		}
		userAcc.Fidx=idx++;
		userAccVec.push_back(userAcc);
    }
	if(userAccVec.size()>0){
		return setUserAcc(userAccVec);
	}
	return true;
	
}

/**
 * 通过查询子账户和DB赎回单对账活期CKV
 * 1, 余额大于0 应该写入CKV;
 * 2, 存在在途赎回应该写入CKV；
 * 3, 最后一次赎回提现是当前对账日,应该写入CKV
 * 4, 考虑延迟情况,16:30前均任务是15:00前的D日, 入参isDayEnd表示是否是16:30之后
 */
bool checkOpenTransUserAcc(CMySQL* mysql,ST_FUND_BIND& pstRecord,map<string,FundUserAcc>& ckvMap,map<string,FundUserAcc>& dbMap, bool isDayEnd) throw (CException)
{
	gPtrAppLog->debug("[checkOpenTransUserAcc]trade_id=[%s]",pstRecord.Ftrade_id);
	int omit = 0;
	int diff = 0;
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	GetTimeNow(szTimeNow);
	//从ckv查询用户绑定的所有基金公司列表
	vector<FundBindSp> fundBindSpVec;	
	getFundBindAllSpFromKV(pstRecord.Ftrade_id, fundBindSpVec);
	
	// 查询用户最近赎回情况
	map<string,string> redeemTimeMap;
	string checkDate = toString(GetDateToday()); // yyyymmdd
	statOpenRedeemModifyTime(mysql,checkDate,pstRecord.Fuid,redeemTimeMap);

	// 计算最后提现检查时间
	string fetchCheckTime;	
	if(isDayEnd){ // 日终后,检查最后提现时间为当日15点
		fetchCheckTime = changeDateFormat(checkDate)+" 15:00:00";
	}else{ //日终前,检查最后提现时间为前日15点
		fetchCheckTime = changeDateFormat(addDays(checkDate, -1))+" 15:00:00";
	}	
	
	for(vector<FundBindSp>::iterator iter = fundBindSpVec.begin();iter != fundBindSpVec.end(); ++iter)
	{
		string spid = (*iter).Fspid;
		// 查询基金账户信息
		SpConfigCache spConfig;
		querySpConfigCache(mysql,spid,spConfig);
		int curtype = spConfig.curtype;
		
		// 只检查活期
		if(spConfig.close_flag!=CLOSE_FLAG_NORMAL){
			continue;
		}
		
		// 查询子账户余额
		LONG freezeBalance=0;
		LONG balance = querySubaccBalance(pstRecord.Fuid, curtype, false,&freezeBalance);		
		
		// 最后一次赎回提现完成时间(10状态的modify_time)
		string onpass=spConfig.fund_code+"_5";
		string fetched=spConfig.fund_code+"_10";
		string fetchTime="1970-01-01 00:00:00"; //最后提现完成时间,设定初始值
		if(redeemTimeMap.find(fetched)!=redeemTimeMap.end()){
			fetchTime=redeemTimeMap[fetched];
		}
		// 查询未确认资产表
		bool hasUnconfirm = false;
		if(spConfig.buy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)
		{
			//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
			if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
			{
				hasUnconfirm = queryFundUnconfirmExists(mysql,spid,pstRecord.Ftrade_id);
			}else{
				hasUnconfirm = queryUnfinishTransExistsBySp(mysql,spid,pstRecord.Ftrade_id);
			}
		}
		//不添加CKV的情况:
		//1, 余额为0；  2, 不存在在途赎回；3,最后一次赎回提现是日终前	;4,不存在未确认金额
		if(balance<=0&&freezeBalance<=0&&
			redeemTimeMap.find(onpass)==redeemTimeMap.end()&&
			fetchTime<fetchCheckTime&&!hasUnconfirm){
			continue;
		}
		/* 不记录待确认数据
		int confirm = 0;
		if(spConfig.buy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)
		{
			// 查询未确认资产表
			bool hasUnconfirm = queryFundUnconfirmExists(mysql,spid,pstRecord.Ftrade_id);
			if(!hasUnconfirm) // 检查存在未确认的赎回份额
			{
				hasUnconfirm = isExistsUnconfirmRedemtion(mysql,pstRecord.Fuid,spid);
			}
			confirm = hasUnconfirm?1:0;
		}*/
		// 比对CKV和DB数据
		string key = spConfig.fund_code+"_0"; // 活期key
		map<string,FundUserAcc>::iterator it = ckvMap.find(key);
		FundUserAcc userAcc;
		if(it!=ckvMap.end()){ // CKV中存在,标记检查过
			FundUserAcc& userAccKV = it->second;
			userAccKV.checkFlag=1;
		/* 不记录待确认数据
			if(userAccKV.Fconfirm!=confirm) // 检查未确认份额
			{
				diff++;
				userAccKV.Fconfirm=confirm;
			} */
			userAcc=it->second;
		}else{// CKV中不存在的活期
			omit++;
			memset(&userAcc, 0, sizeof(FundUserAcc));
			strncpy(userAcc.Ftrade_id,pstRecord.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
			strncpy(userAcc.Ffund_code,spConfig.fund_code.c_str(),sizeof(userAcc.Ffund_code)-1);
			strncpy(userAcc.Ftime ,szTimeNow,sizeof(userAcc.Ftime)-1);
			userAcc.Fconfirm=1;
			TRACE_ERROR("[checkOpenTransUserAcc]has omit fund[%s][%s][0]",userAcc.Ftrade_id,userAcc.Ffund_code);
		}
		//新增到DBMap中
		if(dbMap.find(key)==dbMap.end()){
			dbMap[key] = userAcc;
		}
	}	
	gPtrAppLog->debug("[checkOpenTransUserAcc]db[%d]ckv[%d]omit[%d]",dbMap.size(),ckvMap.size(),omit);
	return omit==0&&diff==0;	
}
/* 通过DB查询对账
* 1, 昨日有份额(含在途份额)的数据
*      a)  15点前检查昨日入账完成
        b)  15点之后需完成收益检查的工作:排除当日T0赎回和T1提现的份额
        c)  考虑机器时间差和补单因数,增加冗余时间段,15点改成是16点30
*  2, 今日有新申购的数据
bool checkOpenTransUserAcc(CMySQL* mysql,ST_FUND_BIND& pstRecord,map<string,FundUserAcc>& ckvMap,map<string,FundUserAcc>& dbMap, bool checkDelete) throw (CException)
{		
	gPtrAppLog->debug("[checkOpenTransUserAcc]trade_id=[%s]",pstRecord.Ftrade_id);
	int omit = 0;
	// 查询用户的昨日收益
	vector<FundProfit> fundProfitVec;
	queryFundProfitList(mysql, pstRecord.Ftrade_id, fundProfitVec,false);
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	GetTimeNow(szTimeNow);
	string checkDate = toString(GetDateToday()); // yyyymmdd
	string yesDate = getLastDate();
	map<string,LONG> redeemFeeMap;
	map<string,LONG> buyFeeMap;

	// 查询新申购数据
	statOpenBuyFee(mysql,pstRecord.Fuid,checkDate,buyFeeMap);
	if(fundProfitVec.size()<=0&&buyFeeMap.size()<=0){
		return true;
	}
	
	
	// 查询15点前各基金公司的D0赎回总值
	if(checkDelete){
		statOpenD0Redeem(mysql,pstRecord.Fuid,checkDate,redeemFeeMap);
	}
	
	// 查询昨日收益
	for(vector<FundProfit>::size_type i=0;i<fundProfitVec.size();i++)
	{
		FundProfit& profit = fundProfitVec[i];
		// 检查收益日期，太早则非法
		if(string(profit.Frecon_day)<yesDate){
			continue;
		}
		// 收益,金额,在途T+1,可产生收益份额均为0, 非法
		if(profit.Fprofit==0&&profit.Frecon_balance==0&&profit.Fvalid_money==0&&profit.Ftplus_redem_money==0){
			continue;
		}
		SpConfigCache sp_config;
		querySpConfigCache(mysql,string(profit.Fspid),sp_config);
		// 只检查活期
		if(sp_config.close_flag!=CLOSE_FLAG_NORMAL){
			continue;
		}
		string fund_code=string(sp_config.fund_code);
		// 轧差15点前D0赎回和申购的金额
		LONG redeemFee=0;
		LONG buyFee=0;
		if(buyFeeMap.find(fund_code)!=buyFeeMap.end()){
			buyFee=buyFeeMap[fund_code];
			buyFeeMap.erase(fund_code);
		}
		if(redeemFeeMap.find(fund_code)!=redeemFeeMap.end()){
			redeemFee=redeemFeeMap[fund_code];
		}		
		LONG balance=profit.Frecon_balance+buyFee-redeemFee;
		
		// 对于对账余额为0的份额,检查到期提现的金额,确认是否完全退出
		LONG tplusFee=profit.Ftplus_redem_money;
		if(balance<=0&&tplusFee>0){
			LONG fetchFee = statTminusFetchFee(mysql,checkDate,pstRecord.Ftrade_id,sp_config.curtype);
			tplusFee=tplusFee-fetchFee;
		}
		// 当日余额为0,且没有在途T+1, 不记录CKV
		if(balance<=0&&tplusFee<=0){
			continue;
		}

		// 比对CKV和DB数据
		string normalKey = fund_code+"_0";
		map<string,FundUserAcc>::iterator it = ckvMap.find(normalKey);
		FundUserAcc userAcc;
		if(it!=ckvMap.end()){ // CKV中存在,标记检查过
			FundUserAcc& userAccKV = it->second;
			userAccKV.checkFlag=1;
			userAcc=it->second;
		}else{// CKV中不存在的活期
			omit++;
			memset(&userAcc, 0, sizeof(FundUserAcc));
			strncpy(userAcc.Ftrade_id,profit.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
			strncpy(userAcc.Ffund_code,sp_config.fund_code.c_str(),sizeof(userAcc.Ffund_code)-1);
			strncpy(userAcc.Ftime ,szTimeNow,sizeof(userAcc.Ftime)-1);
			TRACE_ERROR("[checkOpenTransUserAcc]has omit fund[%s][%s][0]",userAcc.Ftrade_id,userAcc.Ffund_code);
		}
		//新增到DBMap中
		if(dbMap.find(normalKey)==dbMap.end()){
			dbMap[normalKey] = userAcc;
		}
	}
	
	// 检查新增申购
	for(map<string,LONG>::iterator it = buyFeeMap.begin(); it != buyFeeMap.end();it++ )
    {
    	string fund_code=it->first;
		LONG buyFee=it->second;
		LONG redeemFee=0;
		if(redeemFeeMap.find(fund_code)!=redeemFeeMap.end()){
			redeemFee=redeemFeeMap[fund_code];
		}
		// 金额被15点前的D0全部赎回,删除数据
		if(buyFee-redeemFee<=0&&checkDelete){
			continue;
		}

		// 比对CKV和DB数据
		string normalKey = fund_code+"_0";
		map<string,FundUserAcc>::iterator it = ckvMap.find(normalKey);
		FundUserAcc userAcc;
		if(it!=ckvMap.end()){ // CKV中存在,标记检查过
			FundUserAcc& userAccKV = it->second;
			userAccKV.checkFlag=1;
			userAcc=it->second;
		}else{// CKV中不存在的活期
			omit++;
			memset(&userAcc, 0, sizeof(FundUserAcc));
			strncpy(userAcc.Ftrade_id,pstRecord.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
			strncpy(userAcc.Ffund_code,fund_code.c_str(),sizeof(userAcc.Ffund_code)-1);
			strncpy(userAcc.Ftime ,szTimeNow,sizeof(userAcc.Ftime)-1);
			TRACE_ERROR("[checkOpenTransUserAcc]has omit buy[%s][%s][0]",userAcc.Ftrade_id,userAcc.Ffund_code);
		}
		//新增到DBMap中
		if(dbMap.find(normalKey)==dbMap.end()){
			dbMap[normalKey] = userAcc;
		}
    }
	gPtrAppLog->debug("[checkOpenTransUserAcc]db[%d]ckv[%d]omit[%d]",dbMap.size(),ckvMap.size(),omit);
	return omit==0;
} */

// 检查定期的userAcc数据
bool checkCloseTransUserAcc(CMySQL* mysql,const string& tradeId,map<string,FundUserAcc>& ckvMap,map<string,FundUserAcc>& dbMap, bool isDayEnd) throw (CException)
{
	int omit = 0;
	//定期理财:查询昨日未到期的数据
	vector< FundCloseTrans> closeVec;
	int checkDate = GetDateToday(); // yyyymmdd
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	GetTimeNow(szTimeNow);
	queryFundCloseTransAllByProfitEnd(mysql,tradeId,checkDate,closeVec);
	for(vector<FundCloseTrans>::size_type j=0;j<closeVec.size();j++){
		FundCloseTrans& closeTrans = closeVec[j];
		//当日 客服强制赎回的非法
		if(isDayEnd&&closeTrans.Fuser_end_type==CLOSE_FUND_END_TYPE_ALL_KEFU&&closeTrans.Fstate==CLOSE_FUND_STATE_SUC){
			continue;
		}
		//当日到期的非法
		if(isDayEnd&&atoi(closeTrans.Fprofit_end_date)==checkDate){
			continue;
		}
		
		// 核对CKV数据
		string closeKey = string(closeTrans.Ffund_code)+"_"+toString(closeTrans.Fid);
		map<string,FundUserAcc>::iterator it = ckvMap.find(closeKey);
		FundUserAcc userAcc;
		if(it!=ckvMap.end()){ // CKV中存在,标记检查过
			FundUserAcc& userAccKV = it->second;
			userAccKV.checkFlag=1;
			userAcc = it->second;
		}else{ // CKV 中不存在
			omit++;
			TRACE_ERROR("[checkCloseTransUserAcc]has omit fund[%s][%s][%ld]",closeTrans.Ftrade_id,closeTrans.Ffund_code,closeTrans.Fid);
			memset(&userAcc, 0, sizeof(FundUserAcc));
			strncpy(userAcc.Ftrade_id,closeTrans.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
			strncpy(userAcc.Ffund_code,closeTrans.Ffund_code,sizeof(userAcc.Ffund_code)-1);
			strncpy(userAcc.Ftime ,szTimeNow,sizeof(userAcc.Ftime)-1);
			userAcc.Fcloseid = closeTrans.Fid;
		}
		// 新增到DBMap中
		if(dbMap.find(closeKey)==dbMap.end()){
			dbMap[closeKey]=userAcc;
		}
	}
	gPtrAppLog->debug("[checkCloseTransUserAcc]db[%d]ckv[%d]omit[%d]",dbMap.size(),ckvMap.size(),omit);
	return omit==0;
}
bool isUserAccKVAllCheck(map<string,FundUserAcc>& ckvMap) throw (CException)
{
	int redundancy=0;	
	for(map<string,FundUserAcc>::iterator it = ckvMap.begin(); it != ckvMap.end();it++ )
    {
    	FundUserAcc& userAccKV = it->second;
		if(userAccKV.Ffund_code[0]=='\0'){
			continue; // 删除垃圾数据
		}
		//存在未检查过的数据,	告警
		if(userAccKV.checkFlag!=1){
			redundancy++;
			TRACE_ERROR("[isUserAccKVAllCheck]has unconsist fund[%s][%s][%ld]",userAccKV.Ftrade_id,userAccKV.Ffund_code,userAccKV.Fcloseid);
		}
    }
	gPtrAppLog->debug("[isUserAccKVAllCheck]ckv[%d]redundancy[%d]",ckvMap.size(),redundancy);
 	return redundancy==0;
}

