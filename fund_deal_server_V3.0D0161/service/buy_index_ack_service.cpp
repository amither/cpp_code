/**
  * FileName: buy_index_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#include "fund_commfunc.h"
#include "buy_index_ack_service.h"

BuyIndexAck::BuyIndexAck(CMySQL* mysql):AbstractBuySpAck(mysql)
{	
    TRACE_DEBUG("[BuyIndexAck] init");
}

/**
  * service step 1: 解析输入参数
  */
void BuyIndexAck::parseBizInputMsg(char* szMsg)  throw (CException)
{     
	m_params.readStrParam(szMsg,"biz_attach",0,MAX_PARAM_LEN);
	const char* bizAttach = m_params.getString("biz_attach").c_str();
	if(strcmp(bizAttach,"")==0)
	{
		return;
	}
	m_params.readStrParam(bizAttach,"confirm_date",0,14);
	m_params.readStrParam(bizAttach,"fund_net",0,16);
	m_params.readStrParam(bizAttach,"net_date",0,14);
	m_params.readIntParam(bizAttach,"units_usable",0,2); // 份额类型.1:待可用已确认份额;2:待确认不可用份额; 

	// 指数基金默认只使用前端收费
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_FRONT);
	}
    return;
}
/**
  * 更新CKV
  */
void BuyIndexAck::updateCkvs()
{
	AbstractBuySpAck::updateCkvs();	
	if(m_doSaveOnly)
	{
		//差错补单只补子账户,不需要补ckv		
		return;
	}
	
	if(need_refund)
	{
		return;
	}
	if(INF_PAY_OK == m_optype || INF_PUR_SP_ACK_SUC== m_optype)
	{
		//记录未确认份额		
		//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
		if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
		{
			setFundUnconfirm(m_pFundCon,m_fund_bind.Ftrade_id);
		}

		setFundUnfinishTransCKV(m_pFundCon,m_fund_bind.Ftrade_id);
	}
}
/**
*  不支持用券
*/
void BuyIndexAck::CheckFundTrade() throw (CException)
{
	// 先检查公用特性
	AbstractBuySpAck::CheckFundTrade();
	
	/**
	 * 检查指数型特性
	 */
	
	char szErrMsg[128]={0};
	// 不支持券服务购买
	if("" != string(m_stTradeBuy.Fcoupon_id))
	{
		snprintf(szErrMsg,sizeof(szErrMsg),"[BuyIndexAck]listid[%s] not allow use coupon", m_stTradeBuy.Flistid);
		throw CException(ERR_INDEX_BAD_PARAM, szErrMsg, __FILE__, __LINE__);
	}
	// 不支持合约机
	if(m_stTradeBuy.Fpurpose == PURPOSE_FREEZE)
	{
		snprintf(szErrMsg,sizeof(szErrMsg),"[BuyIndexAck]listid[%s] not allow freeze purpose", m_stTradeBuy.Flistid);
		throw CException(ERR_INDEX_BAD_PARAM, szErrMsg, __FILE__, __LINE__);
	}
	
}
bool BuyIndexAck::CheckParamsForPay() throw (CException)
{
	AbstractBuySpAck::CheckParamsForPay();
	if(need_refund)
	{
		return true;
	}
	// 只告警不拋错，默认修改成0
	if(m_params.getLong("fund_units")!=0)
	{
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "指数型基金支付通知提供份额不为0,默认修改成0[%s][%ld]",m_stTradeBuy.Flistid,m_params.getLong("fund_units")); 
		alert(ERR_INDEX_BAD_PARAM,szMsg);
		m_params.setParam("fund_units",0);
	}
	// 检查指数基金关联数据
	strncpy(m_fundIndexTrans.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	if(queryFundTransProcess(m_pFundCon,m_fundIndexTrans))
	{
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "指数型基金支付通知存在关联指数数据[%s][%s]",m_stTradeBuy.Flistid,m_fund_bind.Ftrade_id);
		throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
	}
	
	return true;
}

/**
 * 检查并计算ETF基金交易日,记录到m_params中
 */
void BuyIndexAck::CheckPayTransDate() throw(CException)
{	
	//计算交易日期.交易日是T日,份额确认日是T+1日,首次收益日是T+1日
	string trade_date;
	string fund_vdate;	
	vector<string> tDateVec;
	bool hasDate = queryFundTplusNDates(m_pFundCon,tDateVec,m_acc_time,2);
	if(!hasDate){
		//无配置日期
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "指数型基金申购T+1交易日未配置[%s][%s][%zd]",m_stTradeBuy.Flistid,m_acc_time.c_str(),tDateVec.size()); 
		alert(ERR_UNFOUND_TRADE_DATE,szMsg);
		throw CException(ERR_UNFOUND_TRADE_DATE, szMsg, __FILE__, __LINE__);
	}
	// 交易日是T日
	m_params.setParam("trade_date",tDateVec[0]);
	// 份额确认日是T+1日
	string confirmDate = m_params.getString("confirm_date");
	if(confirmDate!=""&&confirmDate!=tDateVec[1])
	{
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "指数型基金申购份额确认日与计算不一致[%s][%s][%s]",m_stTradeBuy.Flistid,confirmDate.c_str(),tDateVec[1].c_str()); 
		throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);		
	}
	
	// 首次收益日是T+1日
	m_params.setParam("fund_vdate",tDateVec[1]);
	// 份额确认日是T+1日
	m_params.setParam("confirm_date",tDateVec[1]);

	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)

	{
		// 检查未确认份额
		strncpy(m_fundUnconfirm.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundUnconfirm.Ftrade_id)-1);
		strncpy(m_fundUnconfirm.Fspid,m_fund_sp_config.Fspid,sizeof(m_fundUnconfirm.Fspid)-1);
		strncpy(m_fundUnconfirm.Ftrade_date,m_params.getString("trade_date").c_str(),sizeof(m_fundUnconfirm.Ftrade_date)-1);
		queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true);
		
		if(m_fundUnconfirm.Flstate==UNCONFIRM_FUND_INVALID)
		{
			char szMsg[128]={0};
			snprintf(szMsg, sizeof(szMsg), "指数型基金申购存在无效未确认金额[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
			throw CException(ERR_UNCONFIM_LSTATE, szMsg, __FILE__, __LINE__);
		}
	}
}
double BuyIndexAck::calRedeemRate()
{
	LONG net = getCacheETFNet(gPtrFundDB,m_fund_sp_config.Fspid,m_fund_sp_config.Ffund_code);
   	return (double)net/10000;
}

void BuyIndexAck::BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	// 先创建公用特性
	AbstractBuySpAck::BuildFundTradeForPay(stRecord);	
	/**
	 * 创建指数型基金特性
	 */
	if(need_refund)
	{
		return;
	}
	// 记录指数关联单
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	strncpy(m_fundIndexTrans.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Fspid,m_fund_sp_config.Fspid,sizeof(m_fundIndexTrans.Fspid)-1);
	m_fundIndexTrans.Ftype=m_fund_sp_config.Ftype;
	strncpy(m_fundIndexTrans.Ffund_code,m_fund_sp_config.Ffund_code,sizeof(m_fundIndexTrans.Ffund_code)-1);
	strncpy(m_fundIndexTrans.Ftrade_date,stRecord.Ftrade_date,sizeof(m_fundIndexTrans.Ftrade_date)-1);
	strncpy(m_fundIndexTrans.Fconfirm_date, m_params.getString("confirm_date").c_str(),sizeof(m_fundIndexTrans.Fconfirm_date)-1);
	m_fundIndexTrans.Fpur_type=m_stTradeBuy.Fpur_type;
	m_fundIndexTrans.Fpurpose=m_stTradeBuy.Fpurpose;
	m_fundIndexTrans.Ftotal_fee=m_stTradeBuy.Ftotal_fee;
	m_fundIndexTrans.Ffund_units = 0; // 份额未确认
	strncpy(m_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_fundIndexTrans.Ffund_net)-1);
	m_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_BUY_UNCONFIRM;
	m_fundIndexTrans.Flstate = PROCESS_TRANS_LSTATE_VALID;
	strncpy(m_fundIndexTrans.Fcreate_time,m_params.getString("systime").c_str(),sizeof(m_fundIndexTrans.Fcreate_time)-1);
	strncpy(m_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_fundIndexTrans.Fmodify_time)-1);
	strncpy(m_fundIndexTrans.Ffinish_time,"9999-12-31 23:59:59",sizeof(m_fundIndexTrans.Ffinish_time)-1);
	strncpy(m_fundIndexTrans.Facc_time,m_acc_time.c_str(),sizeof(m_fundIndexTrans.Facc_time)-1);
		
		
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		bool hasUnconfirmData = m_fundUnconfirm.Fid>0;
			
		if(hasUnconfirmData)
		{
			// 组装update 对象
			m_update_fundUnconfirm.Ftotal_fee=m_fundUnconfirm.Ftotal_fee+m_stTradeBuy.Ftotal_fee;
			m_update_fundUnconfirm.Fstate=(m_fundUnconfirm.Fstate!=UNCONFIRM_FUND_STATE_ALL)?UNCONFIRM_FUND_STATE_PART:m_fundUnconfirm.Fstate;
			m_update_fundUnconfirm.Flstate = UNCONFIRM_FUND_VALID;
			strncpy(m_update_fundUnconfirm.Fconfirm_date,m_params.getString("confirm_date").c_str(),sizeof(m_update_fundUnconfirm.Fconfirm_date)-1);
			strncpy(m_update_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundUnconfirm.Fmodify_time)-1);
			m_update_fundUnconfirm.Fid = m_fundUnconfirm.Fid;
			strncpy(m_update_fundUnconfirm.Ftrade_id,m_fundUnconfirm.Ftrade_id,sizeof(m_update_fundUnconfirm.Ftrade_id)-1);
			strncpy(m_update_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundUnconfirm.Fmodify_time)-1);
		}else
		{
			// 组装insert 对象
			strncpy(m_fundUnconfirm.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundUnconfirm.Ftrade_id)-1);
			strncpy(m_fundUnconfirm.Fspid,m_fund_sp_config.Fspid,sizeof(m_fundUnconfirm.Fspid)-1);
			strncpy(m_fundUnconfirm.Ftrade_date,stRecord.Ftrade_date,sizeof(m_fundUnconfirm.Ftrade_date)-1);
			strncpy(m_fundUnconfirm.Fconfirm_date,m_params.getString("confirm_date").c_str(),sizeof(m_fundUnconfirm.Fconfirm_date)-1);
			m_fundUnconfirm.Ftotal_fee=m_stTradeBuy.Ftotal_fee;
			m_fundUnconfirm.Fcfm_total_fee=0;
			m_fundUnconfirm.Fcfm_units=0;
			m_fundUnconfirm.Funuse_units=0;
			strncpy(m_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_fundUnconfirm.Ffund_net)-1);
			m_fundUnconfirm.Fstate=UNCONFIRM_FUND_STATE_ALL;
			m_fundUnconfirm.Flstate = UNCONFIRM_FUND_VALID;
			strncpy(m_fundUnconfirm.Fcreate_time,m_acc_time.c_str(),sizeof(m_fundUnconfirm.Fcreate_time)-1);
			strncpy(m_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_fundUnconfirm.Fmodify_time)-1);
		}
	}
}

void BuyIndexAck::RecordFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	// 先创建公用特性
	AbstractBuySpAck::RecordFundTradeForPay(stRecord);

	if(need_refund)
	{
		return;
	}
	// 创建指数基金关联属性
	insertFundTransProcess(m_pFundCon,m_fundIndexTrans);
	
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// 更新指数型未确认金额	
		bool hasUnconfirmData = m_fundUnconfirm.Fid>0;
		TRACE_DEBUG("hasUnconfirmData:%ld",m_fundUnconfirm.Fid);
		if(hasUnconfirmData)
		{
			updateFundUnconfirmById(m_pFundCon,m_update_fundUnconfirm);
		}else{
			insertFundUnconfirm(m_pFundCon,m_fundUnconfirm);
		}
	}

}
/**
 * 检查份额确认重入参数
 */
void BuyIndexAck::CheckFundTradeRepeatForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{	
	char szMsg[128]={0};
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// 检查未确认份额
		strncpy(m_fundUnconfirm.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundUnconfirm.Ftrade_id)-1);
		strncpy(m_fundUnconfirm.Fspid,m_fund_sp_config.Fspid,sizeof(m_fundUnconfirm.Fspid)-1);
		strncpy(m_fundUnconfirm.Ftrade_date,m_stTradeBuy.Ftrade_date,sizeof(m_fundUnconfirm.Ftrade_date)-1);
		if(!queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true))
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认重入找不到未确认份额[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
			throw CException(ERR_REPEAT_ENTRY_DIFF, szMsg, __FILE__, __LINE__);
		}
		
		// 份额一致
		if(m_params.getLong("fund_units")!=m_stTradeBuy.Freal_redem_amt)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认重入份额不一致[%s][%ld][%ld]",m_fundUnconfirm.Fspid,m_stTradeBuy.Freal_redem_amt,m_params.getLong("fund_units"));  
			throw CException(ERR_REPEAT_ENTRY_DIFF, szMsg, __FILE__, __LINE__);
		}
		
		// 净值一致
		if(m_params.getString("fund_net")!=m_fundUnconfirm.Ffund_net)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认重入净值不一致[%s][%s][%s]",m_fundUnconfirm.Fspid,m_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str());  
			throw CException(ERR_REPEAT_ENTRY_DIFF, szMsg, __FILE__, __LINE__);
		}
	}
	
	//TODO:fundIndexTrans增加逻辑:第二步增加检查新表数据
	// 检查指数基金关联数据
	strncpy(m_fundIndexTrans.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	bool hasProcess=queryFundTransProcess(m_pFundCon,m_fundIndexTrans,true);
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
	{
		if(!hasProcess)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认重入,找不到关联指数数据[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);
			throw CException(ERR_INDEX_BUY_REPEAT, szMsg, __FILE__, __LINE__);
		}
		
		// 份额一致
		if(m_params.getLong("fund_units")!=m_stTradeBuy.Freal_redem_amt||m_params.getLong("fund_units")!=m_fundIndexTrans.Ffund_units)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认重入,份额不一致[%s][%ld][%ld][%ld]",m_fundIndexTrans.Fspid,m_stTradeBuy.Freal_redem_amt,m_fundIndexTrans.Ffund_units,m_params.getLong("fund_units"));  
			throw CException(ERR_INDEX_BUY_REPEAT, szMsg, __FILE__, __LINE__);
		}
		
		// 净值一致
		if(m_params.getString("fund_net")!=m_fundIndexTrans.Ffund_net)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认重入,净值不一致[%s][%s][%s][%s]",m_fundIndexTrans.Fspid,m_fundIndexTrans.Ffund_net,m_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str());  
			throw CException(ERR_INDEX_BUY_REPEAT, szMsg, __FILE__, __LINE__);
		}
	}

	if(hasProcess)
	{
		m_params.setParam("subacc_units",m_fundIndexTrans.Ffund_units);
		m_params.setParam("subacc_time",m_fundIndexTrans.Fsubacc_time);
	}
	
}
/**
  * 检查份额确认
  */
bool BuyIndexAck::CheckFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// 先检查公用特性: 公共特性的正常返回不return false，由子类确定流程
	AbstractBuySpAck::CheckFundTradeForSucAck(stRecord);
	// 检查业务参数是否存在
	CHECK_PARAM_EMPTY("confirm_date");
	CHECK_PARAM_EMPTY("fund_net");
	CHECK_PARAM_EMPTY("net_date");
	CHECK_PARAM_EMPTY("units_usable");
	
	char szMsg[128]={0};
	int unitsUsable = m_params.getInt("units_usable");
	
	// 判断重入
	if(unitsUsable==BUY_ACK_UNITS_CFM_USEABLE && PURCHASE_SUC==m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==0)
	{		
		//重入错误检查
		CheckFundTradeRepeatForSucAck(stRecord);
		// 事务成功的差错补单不是重入，只更新子账户,不进行后续操作
		if(ERR_TYPE_MSG)
		{
			m_doSaveOnly=true;
			return false;
		}		
		
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}
	// 判断重入
	if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE && PURCHASE_SUC==m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==TRADE_SPETAG_UNITS_UNUSABLE)
	{
		//重入错误
		CheckFundTradeRepeatForSucAck(stRecord);
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}
	
	// 确认份额,金额应该一致
	if(m_params.getLong("total_fee")!=m_stTradeBuy.Ftotal_fee)
	{
		snprintf(szMsg, sizeof(szMsg), "指数型基金份额确认金额不一致[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_params.getLong("total_fee"),m_stTradeBuy.Ftotal_fee); 
		throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
	}
	
	/** 指数型基金不支持直接确认可用份额**/
	// 1: 确认不可用份额
	if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE)
	{
		// 份额应该是未确认
		if(m_stTradeBuy.Freal_redem_amt>0)
		{
			snprintf(szMsg, sizeof(szMsg), "指数型基金份额确认,存在已确认份额[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_params.getLong("fund_units"),m_stTradeBuy.Freal_redem_amt); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
			
	}
	// 2: 修改已确认份额可用
	else if(unitsUsable==BUY_ACK_UNITS_CFM_USEABLE )
	{
		// 检查可以变更状态
		if(PURCHASE_SUC == m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==TRADE_SPETAG_UNITS_UNUSABLE)
		{
			gPtrAppLog->debug("使已确认份额可用,检查状态正确[%s]",m_stTradeBuy.Flistid);
		}else{
			//数据库状态不能更新
			throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
		}
	}else
	{
		snprintf(szMsg, sizeof(szMsg), "指数型基金不支持该类型份额[%s][%d][%ld]",m_stTradeBuy.Flistid,unitsUsable,m_params.getLong("fund_units")); 
		throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);
	}
	
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// 检查未确认份额参数
		strncpy(m_fundUnconfirm.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundUnconfirm.Ftrade_id)-1);
		strncpy(m_fundUnconfirm.Fspid,m_stTradeBuy.Fspid,sizeof(m_fundUnconfirm.Fspid)-1);
		strncpy(m_fundUnconfirm.Ftrade_date,m_stTradeBuy.Ftrade_date,sizeof(m_fundUnconfirm.Ftrade_date)-1);
		if(!queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true))
		{
			char szMsg[128]={0};
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认找不到未确认份额[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);
		}
		
		if(m_fundUnconfirm.Flstate==UNCONFIRM_FUND_INVALID)
		{
			char szMsg[128]={0};
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认存在无效未确认份额[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);
		}
		// 确认份额,未确认金额应该大于待确认金额
		if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE&&m_fundUnconfirm.Ftotal_fee<m_stTradeBuy.Ftotal_fee)
		{
			snprintf(szMsg, sizeof(szMsg), "指数型基金份额确认,未确认金额应该大于待确认金额[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_fundUnconfirm.Ftotal_fee,m_stTradeBuy.Ftotal_fee); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
		// 使份额可用,不可用份额应该大于待可用份额
		if(unitsUsable==BUY_ACK_UNITS_CFM_USEABLE&&m_fundUnconfirm.Funuse_units<m_params.getLong("fund_units"))
		{
			snprintf(szMsg, sizeof(szMsg), "指数型基金份额确认,不可用份额应该大于待可用份额[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_fundUnconfirm.Funuse_units,m_params.getLong("fund_units")); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
		// 检查净值一致性
		if(strcmp(m_fundUnconfirm.Ffund_net,"")!=0&&strcmp(m_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str())!=0)
		{
			snprintf(szMsg, sizeof(szMsg), "指数型基金份额确认,份额净值不一致性[%s][%s][%s]",m_stTradeBuy.Flistid,m_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str()); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
		// 检查申购确认日一致性
		if(m_params.getString("confirm_date")!=""&&m_fundUnconfirm.Fconfirm_date!=m_params.getString("confirm_date"))
		{
			snprintf(szMsg, sizeof(szMsg), "指数型基金份额确认,申购确认日不一致性[%s][%s][%s]",m_stTradeBuy.Flistid,m_fundUnconfirm.Fconfirm_date,m_params.getString("confirm_date").c_str()); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
	}
	
	// 检查指数基金关联数据
	strncpy(m_fundIndexTrans.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	bool hasProcess=queryFundTransProcess(m_pFundCon,m_fundIndexTrans,true);
	//TODO:fundIndexTrans增加逻辑:第二步增加检查新表数据
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
	{
		if(!hasProcess)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认,找不到关联指数数据[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
		
		if(m_fundIndexTrans.Flstate==PROCESS_TRANS_LSTATE_INVALID)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认,关联指数数据无效[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);  
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
		
		// 状态一致
		if(m_fundIndexTrans.Fstate!=PROCESS_TRANS_STATE_BUY_UNCONFIRM&&unitsUsable==BUY_ACK_UNITS_UNUSEABLE)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购确认份额,关联指数状态不正确[%s][%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id,m_fundIndexTrans.Fstate);  
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
		if(m_fundIndexTrans.Fstate!=PROCESS_TRANS_STATE_BUY_CONFIRM&&unitsUsable==BUY_ACK_UNITS_CFM_USEABLE)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF申购使份额可用,关联指数状态不正确[%s][%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id,m_fundIndexTrans.Fstate);  
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
	}
	
	return true;
}

/**
  * 指数型基金申购确认,组装参数
  */ 
void BuyIndexAck::BuildFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// 先记录公共参数
	AbstractBuySpAck::BuildFundTradeForSucAck(stRecord);

	// 指数型基金特性参数
	// 基金净值
	strncpy(stRecord.Ffund_value,m_params.getString("fund_net").c_str(),sizeof(stRecord.Ffund_value)-1);
	stRecord.Freal_redem_amt=m_params.getLong("fund_units");

	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		m_update_fundUnconfirm.Fid = m_fundUnconfirm.Fid;
		strncpy(m_update_fundUnconfirm.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(m_update_fundUnconfirm.Ftrade_id)-1);
		// 判断份额是否可用
		int unitsUsable = m_params.getInt("units_usable");
		LONG fundUnits = m_params.getLong("fund_units");

		// 确认不可用份额:不更新子账户
		if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE) 
		{
			stRecord.Fspe_tag=TRADE_SPETAG_UNITS_UNUSABLE;
			m_params.setParam("subacc_units",0);
			if(m_fundUnconfirm.Ftotal_fee==m_stTradeBuy.Ftotal_fee)
			{
				// 金额全部确认
				m_update_fundUnconfirm.Fstate=UNCONFIRM_FUND_STATE_UNUSABLE;
			}else
			{
				// 金额部分确认
				m_update_fundUnconfirm.Fstate=UNCONFIRM_FUND_STATE_PART;
			}

			m_update_fundUnconfirm.Ftotal_fee = m_fundUnconfirm.Ftotal_fee-m_stTradeBuy.Ftotal_fee;
			m_update_fundUnconfirm.Fcfm_total_fee= m_fundUnconfirm.Fcfm_total_fee+m_stTradeBuy.Ftotal_fee;
			m_update_fundUnconfirm.Fcfm_units= m_fundUnconfirm.Fcfm_units+fundUnits;
			m_update_fundUnconfirm.Funuse_units= m_fundUnconfirm.Funuse_units+fundUnits;
			strncpy(m_update_fundUnconfirm.Fconfirm_date,m_params.getString("confirm_date").c_str(),sizeof(m_update_fundUnconfirm.Fconfirm_date)-1);
			strncpy(m_update_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_update_fundUnconfirm.Ffund_net)-1);
			strncpy(m_update_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundUnconfirm.Fmodify_time)-1);
		}
		// 使已确认份额可用:需要更新子账户
		else 
		{	
			stRecord.Fspe_tag=0;
			m_params.setParam("subacc_units",stRecord.Freal_redem_amt);
			if(m_fundUnconfirm.Fstate==UNCONFIRM_FUND_STATE_UNUSABLE)
			{
				// 修改不可用份额状态
				m_update_fundUnconfirm.Fstate = UNCONFIRM_FUND_STATE_NONE;
			}
			// 修改不可用份额
			m_update_fundUnconfirm.Funuse_units = m_fundUnconfirm.Funuse_units-fundUnits;
			strncpy(m_update_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundUnconfirm.Fmodify_time)-1);
		}
	}
	
	// 更新关联指数数据
	m_update_fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(m_update_fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(m_update_fundIndexTrans.Ftrade_id)-1);
	
	// 判断份额是否可用
	int unitsUsable = m_params.getInt("units_usable");
	LONG fundUnits = m_params.getLong("fund_units");
	// 确认不可用份额:不更新子账户
	if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE) 
	{
		stRecord.Fspe_tag=TRADE_SPETAG_UNITS_UNUSABLE;
		m_params.setParam("subacc_units",0);
		m_update_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_BUY_CONFIRM;
		m_update_fundIndexTrans.Ffund_units= fundUnits;
		strncpy(m_update_fundIndexTrans.Fconfirm_date,m_params.getString("confirm_date").c_str(),sizeof(m_update_fundIndexTrans.Fconfirm_date)-1);
		strncpy(m_update_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_update_fundIndexTrans.Ffund_net)-1);
		strncpy(m_update_fundIndexTrans.Fconfirm_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fconfirm_time)-1);
		strncpy(m_update_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fmodify_time)-1);
	}
	// 使已确认份额可用:需要更新子账户
	else 
	{	
		stRecord.Fspe_tag=0;
		m_params.setParam("subacc_units",stRecord.Freal_redem_amt);
		m_params.setParam("subacc_time",m_params.getString("systime"));
		m_update_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_BUY_USABLE;
		strncpy(m_update_fundIndexTrans.Fsubacc_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fsubacc_time)-1);
		strncpy(m_update_fundIndexTrans.Ffinish_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Ffinish_time)-1);
		strncpy(m_update_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fmodify_time)-1);
	}
}

/**
  *   指数型基金申购确认,更新DB
  */
void BuyIndexAck::RecordFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// 先更新公共参数
	AbstractBuySpAck::RecordFundTradeForSucAck(stRecord);
	
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// 更新未确认金额表
		strncpy(m_update_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundUnconfirm.Fmodify_time)-1);
		updateFundUnconfirmById(m_pFundCon,m_update_fundUnconfirm);
	}
	
	// 更新关联指数表数据
	//updateFundTransProcess(m_pFundCon,m_update_fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,m_update_fundIndexTrans,m_fundIndexTrans);
}

/**
* 打包输出参数
*/
void BuyIndexAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "biz_attach", m_params.getString("biz_attach").c_str());
	return;
}


