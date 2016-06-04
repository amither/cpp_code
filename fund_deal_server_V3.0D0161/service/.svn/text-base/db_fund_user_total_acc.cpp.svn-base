#include "db_fund_user_total_acc.h"
#include "db_fund_sp_config.h"
#include "db_c2c_db_t_user.h"


extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 
// 连接基金数据库句柄
extern CMySQL* gPtrFundDB;

/**
 *  查询绑定基金公司的总份额
 */
LONG queryFundUserTotalAccBalanceFromSubaccCkv(const string & trade_id, int uid)
{
    //从ckv查询用户绑定的所有基金公司列表
	vector<FundBindSp> fundBindSpVec;
	getFundBindAllSpFromKV(trade_id, fundBindSpVec);

    vector<int> vecCurtypeList;
	for(vector<FundBindSp>::iterator iter = fundBindSpVec.begin();iter != fundBindSpVec.end(); ++iter)
	{
		vecCurtypeList.push_back(querySubaccCurtype(gPtrFundDB,(*iter).Fspid));
	}
 
    vector<SubaccUser> SubaccListUser;
    querySubaccBalanceListFromCKV(uid, vecCurtypeList, SubaccListUser);
    LONG total_balance = 0;
    for(vector<SubaccUser>::iterator iter = SubaccListUser.begin(); iter!=SubaccListUser.end(); ++iter)
    {
        total_balance += iter->Fbalance;
    }
    return total_balance;
}

/**
  * 查询用户所有基金总资产
  * 基金总资产= 用户份额* 基金当前净值+ 用户申购未确认金额
  * 注意:这里的资产与首页展示不一致，仅用于后台判断资产上限
  * 因此,加上未确认的申购金额，但不减去用户未确认的赎回份额
  * bool addBalance:是否增加理财通余额,默认true
  * bool addFreeze:是否增加冻结份额,默认true
  */
LONG queryUserTotalAsset(int uid, const string tradeId, bool addBalance, bool addFreeze)
{
    //查询用户绑定的所有基金公司列表
	vector<FundBindSp> fundBindSpVec;
	queryFundBindAllSp(gPtrFundDB, fundBindSpVec, tradeId,false);

	// 从缓存中查询基金公司对应的curtype和type
    map<int,SpConfigCache> spConfigMap;
	vector<int> vecCurtypeList;
	for(vector<FundBindSp>::iterator bindIter = fundBindSpVec.begin();bindIter != fundBindSpVec.end(); ++bindIter)
	{
		string spid=(*bindIter).Fspid;
		SpConfigCache spConfig;
		querySpConfigCache(gPtrFundSlaveDB,spid,spConfig);
		spConfigMap[spConfig.curtype]=spConfig;
		vecCurtypeList.push_back(spConfig.curtype);
	}
	// 加上理财通余额
	if(addBalance)
	{
		BalanceConfigCache balanceConfig = getCacheBalanceConfig(gPtrFundSlaveDB);
		SpConfigCache spConfig;
		spConfig.curtype=balanceConfig.curtype;
		spConfig.spid=balanceConfig.spid;
		spConfig.fund_code=balanceConfig.spid;
		spConfig.type=SPCONFIG_TYPE_BALANCE;
		spConfigMap[spConfig.curtype]=spConfig;
		vecCurtypeList.push_back(spConfig.curtype);
	}
	
 	// 查询出未完成的申购交易金额
	map<string,FundUnfinishAssert> unfinishMap;
	statUnfinishBuyAssetByTradeId(gPtrFundDB,tradeId.c_str(),unfinishMap);

	// 查询出用户子账户份额
    vector<SubaccUser> SubaccListUser;
    querySubaccBalanceListFromCKV(uid, vecCurtypeList, SubaccListUser);
	// 计算用户总资产
    LONG total_assert = 0;
    for(vector<SubaccUser>::iterator subaccIter = SubaccListUser.begin(); subaccIter!=SubaccListUser.end(); ++subaccIter)
    {
    	map<int,SpConfigCache>::iterator spconfigIter = spConfigMap.find(subaccIter->Fcurtype);
		if(spconfigIter==spConfigMap.end())
		{
			// 不该出现的情况:查询不到商户信息
			char errMsg[128];
			snprintf(errMsg, sizeof(errMsg), "查询总资产uid=[%d],查询不到子账户对应的商户号信息.curtype=[%d];balance=[%ld]",
				subaccIter->Fuid,subaccIter->Fcurtype,subaccIter->Fbalance);
			TRACE_ERROR("%s",errMsg);
			alert(ERR_TOTALACC_NOT_FOUND_SPCONFIG,errMsg);
			continue;
		}

		SpConfigCache& spConfig=spconfigIter->second;		
		// 净值为10的4次方
		LONG netValue=getCacheSpNet(gPtrFundSlaveDB,spConfig);
		TRACE_DEBUG("total_assert=%d,will check curtype=[%d],balacne=[%ld],con=[%ld],netValue=[%ld]",
			total_assert,subaccIter->Fcurtype,subaccIter->Fbalance, subaccIter->Fcon,netValue);
		map<string,FundUnfinishAssert>::iterator assertIter = unfinishMap.find(spConfig.fund_code);
		LONG freezeBalance=(addFreeze?subaccIter->Fcon:0);
		if(assertIter!=unfinishMap.end())
		{
			FundUnfinishAssert& unfinishAssert = assertIter->second;
			// (子账户份额+子账户冻结份额)*净值+不可用资产
			total_assert += ((subaccIter->Fbalance+freezeBalance)*netValue/10000
				+ unfinishAssert.Ftotal_fee);
			
			TRACE_DEBUG("[%d]total_assert += ((%ld+%ld)*%ld/10000+%ld)",subaccIter->Fcurtype,
				subaccIter->Fbalance,freezeBalance,netValue,unfinishAssert.Ftotal_fee);
		}else{
			// (子账户份额+子账户冻结份额)*净值
        	total_assert += ((subaccIter->Fbalance+freezeBalance)*netValue/10000);
			
			TRACE_DEBUG("[%d]total_assert += ((%ld+%ld)*%ld/10000)",
				subaccIter->Fcurtype,subaccIter->Fbalance, freezeBalance,netValue);
		}
    }
	TRACE_DEBUG("queryUserTotalAsset[%d][%s][%d][%d]total_assert=[%ld]",uid,tradeId.c_str(),addBalance,addFreeze,total_assert);
	return total_assert;
}

