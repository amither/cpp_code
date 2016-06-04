/**
  * FileName: redeem_close_req_service.cpp
  * Author: jessiegao
  * Version :1.0
  * Date: 2015-3-9
  * Description: 基金交易服务 定期产品实时赎回请求 源文件
  */

#include "fund_commfunc.h"
#include "redeem_close_req_service.h"

RedeemCloseReq::RedeemCloseReq(CMySQL* mysql):AbstractRedeemSpReq(mysql)
{
    memset(&m_close_trans, 0, sizeof(FundCloseTrans));

    m_redemTradeExist = false;

}

void RedeemCloseReq::parseBizInputMsg(char* szMsg) throw (CException)
{
    m_params.readLongParam(szMsg, "close_id", 0, MAX_LONG);
    m_params.readIntParam(szMsg, "opt_type", 0,3); // 1:指定金额赎回, 2:全部赎回
    return;
}

/**
  * 检查参数，获取内部参数
  */
void RedeemCloseReq::CheckParams() throw (CException)
{	
	AbstractRedeemSpReq::CheckParams();
	// 验证参数非空
	CHECK_PARAM_EMPTY("cft_bank_billno");
	CHECK_PARAM_EMPTY("cft_trans_id");
	CHECK_PARAM_EMPTY("cft_fetch_id");
	CHECK_PARAM_EMPTY("cft_charge_ctrl_id");
	CHECK_PARAM_EMPTY("close_id");
	CHECK_PARAM_EMPTY("opt_type");

	if(m_fund_sp_config.Fclose_flag!=CLOSE_FLAG_ALL_CLOSE){
        throw CException(ERR_BAD_PARAM, "该接口只支持定期产品", __FILE__, __LINE__);
	}

	//只支持T+1赎回,默认T+1
	if(m_params.getString("fetch_type").empty()){
		m_params.setParam("fetch_type",DRAW_ARRIVE_TYPE_T1);
	}
    if(m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1){
        throw CException(ERR_BAD_PARAM, "定期实时赎回只支持T+1.", __FILE__, __LINE__);
    }

	if ((m_fund_sp_config.Fredem_valid&0x07) ==2) // 停止赎回
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
}

/**
  * 检查基金交易记录是否已经生成
  */
void RedeemCloseReq::CheckFundTrade() throw (CException)
{
	AbstractRedeemSpReq::CheckFundTrade();
		
	// 检查定期记录数据
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,true)){
		TRACE_ERROR("unfound fund_close_trans close_id=%ld,trade_id=%s", m_close_trans.Fid,m_close_trans.Ftrade_id);
		throw EXCEPTION(ERR_BAD_PARAM, "unfound fund_close_trans");
	}
	// 判断用户
	if(strcmp(m_params.getString("trade_id").c_str(),m_close_trans.Ftrade_id)!=0){	
		TRACE_ERROR("close_trans[%ld]trade_id[%s][%s] is different", m_close_trans.Fid,m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "close_trans trade_id is different");
	}
	// 判断商户号参数
	if(0!=strcmp(m_close_trans.Fspid,m_params.getString("spid").c_str())){
		TRACE_ERROR("close_trans[%ld] spid[%s] is not equal to input[%s]", m_close_trans.Fid,m_close_trans.Fspid,m_params.getString("spid").c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "close_trans spid is different");
	}
	/*
    m_redemTradeExist = QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		m_params.getInt("bank_type"), &m_stTradeRedem, false);
	*/
	
	gPtrAppLog->debug("fund redem req trade record exist : %d",m_redemTradeExist);
	
	// 有赎回记录,重入检查
	if(m_redemTradeExist){
		m_params.setParam("fund_trans_id", m_stTradeRedem.Flistid);
		CheckFundTradeRepeat();
		return;
	}
	/**
   * 没有赎回记录,首次请求检查
  **/
    // 检查时间    
	string dueDate=string(m_close_trans.Fdue_date);
	string dueTime=dueDate+"150000";	
	string bookStopDate=string(m_close_trans.Fbook_stop_date);
	string bookStopTime=bookStopDate+"150000"; 	
	if(m_params.getString("systime")>=changeDatetimeFormat(dueTime)||
		m_params.getString("systime")<changeDatetimeFormat(bookStopTime)){
		TRACE_ERROR("cannot redem close_trans right now");
		throw EXCEPTION(ERR_CLOSE_REDEM_TIME, "cannot redem close_trans right now");
	}
	// 已发生扫尾赎回,不能再发起其他形式赎回
	if(m_close_trans.Fsell_listid[0]!=0){
		TRACE_ERROR("close_trans sell_listid exists, cannot redem anymore");
		throw EXCEPTION(ERR_CLOSE_REDEM_HAS_ALL_REDEM, "close_trans sell_listid exists, cannot redem anymore");
	}
	// 预约部分赎回和全部顺延才可以发起赎回
	if(m_close_trans.Fuser_end_type!=CLOSE_FUND_END_TYPE_PATRIAL_REDEM
		&&m_close_trans.Fuser_end_type!=CLOSE_FUND_END_TYPE_ALL_EXTENSION){
		TRACE_ERROR("close_trans user_end_type[%d] cannot redem",m_close_trans.Fuser_end_type);
		throw EXCEPTION(ERR_CLOSE_REDEM_USER_END, "close_trans user_end_type cannot redem");
	}
	// 检查状态
	if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING&&m_close_trans.Fstate!=CLOSE_FUND_STATE_REDEM_SUC){
		TRACE_ERROR("close_trans status[%d] cannot redem",m_close_trans.Fstate);
		throw EXCEPTION(ERR_CLOSE_REDEM_STATE, "close_trans status cannot redem");
	}
	// 检查剩余金额
	LONG leftFee=m_close_trans.Fcurrent_total_fee;
	if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_PATRIAL_REDEM&&m_close_trans.Fstate==CLOSE_FUND_STATE_PENDING){
		leftFee=leftFee-m_close_trans.Fend_plan_amt;
	}
	if(leftFee<m_params.getLong("total_fee")){		
		TRACE_ERROR("close_trans has not enough money[%ld] to redem[%ld]",leftFee,m_params.getLong("total_fee"));
		throw EXCEPTION(ERR_CLOSE_REDEM_NOT_ENOUGH_MONEY, "close_trans has not enough money to redem");
	}
	//扫尾赎回金额应该等于剩余全部金额
	if(leftFee!=m_params.getLong("total_fee")&&m_params.getInt("opt_type")==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		TRACE_ERROR("close_trans all redem money[%ld][%ld] invalid",leftFee,m_params.getLong("total_fee"));
		throw EXCEPTION(ERR_CLOSE_REDEM_INVALID_MONEY, "close_trans all redem money invalid");
	}
	// 检查当前是否有未完成的同期次赎回单(不包含预约赎回单)
	bool existsInitRedem = isCloseInitRedemExists(m_pFundCon,m_close_trans.Fid,m_close_trans.Fuid,m_close_trans.Ffund_code,bookStopTime.c_str(),dueTime.c_str());
	if(existsInitRedem){
		TRACE_ERROR("exists unfinished redem[%ld][%d]",m_close_trans.Fid,m_close_trans.Fuid);
		throw EXCEPTION(ERR_CLOSE_REDEM_EXISTS_INIT_REDEM, "exists unfinished redem");
	}
	
	// 检查预约赎回截止日的收益入账是否已经完成
	string bookStopProfitTime=bookStopDate+gPtrConfig->m_AppCfg.close_redem_req_stop_time;
	if(m_params.getString("systime")>=changeDatetimeFormat(bookStopProfitTime)){
		FundReconLog recon;
		memset(&recon,0,sizeof(FundReconLog));
		recon.Frecon_type=RECON_TYPE_PROFIT;
		strncpy(recon.Fspid,m_close_trans.Fspid,sizeof(recon.Fspid));
		strncpy(recon.Frecon_date,m_close_trans.Fbook_stop_date,sizeof(recon.Frecon_date));
		bool existRecon = queryFundSpReconLog(m_pFundCon,recon);
		if(!existRecon){
			TRACE_ERROR("profit is recording,cannot redem right now[%s][%s]",m_close_trans.Fspid,m_close_trans.Fbook_stop_date);
			throw EXCEPTION(ERR_CLOSE_REDEM_PROFIT_RECORDING, "profit is recording,cannot redem right now");
		}
	}
	
}

/**
  * 检查基金交易记录是否已经生成
  */
void RedeemCloseReq::CheckFundTradeRepeat() throw (CException)
{
	if(0 != strcmp(m_stTradeRedem.Fspid, m_params.getString("spid").c_str())){
		TRACE_ERROR("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeRedem.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}
	if(0 != strcmp(m_stTradeRedem.Ffund_code, m_params.getString("fund_code").c_str())){
		TRACE_ERROR("fund trade exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_stTradeRedem.Ffund_code, m_params.getString("fund_code").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fund_code is different!", __FILE__, __LINE__);
	}

	if(m_stTradeRedem.Ftotal_fee != m_params.getLong("total_fee")){
		TRACE_ERROR("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, total_fee is different!", __FILE__, __LINE__);
	}

	if(0 != strcmp(m_stTradeRedem.Fcft_trans_id, m_params.getString("cft_trans_id").c_str())){
		TRACE_ERROR("fund trade exists, cft_trans_id is different! cft_trans_id in db[%s], cft_trans_id input[%s] ", 
			m_stTradeRedem.Fcft_trans_id, m_params.getString("cft_trans_id").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, cft_trans_id is different!", __FILE__, __LINE__);
	}
    
    if (m_stTradeRedem.Floading_type != DRAW_NOT_USE_LOADING){
        TRACE_ERROR("fund trade exists, Floading_type is confict! Floading_type in db[%d], fetch_type input[%s] ", 
    			m_stTradeRedem.Floading_type, m_params.getString("fetch_type").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fetch_type is confict!", __FILE__, __LINE__);
    }
	if(LSTATE_INVALID == m_stTradeRedem.Flstate){
		TRACE_ERROR("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			m_stTradeRedem.Flistid, m_stTradeRedem.Ftrade_id, m_stTradeRedem.Fpur_type);
		throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
	}
	if(m_params.getInt("opt_type")!=m_stTradeRedem.Fopt_type){
		TRACE_ERROR("all redem fund trade exists, opt_type diff");
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, opt_type diff");
	}
	if(m_params.getLong("close_id")!=m_stTradeRedem.Fclose_listid){
		TRACE_ERROR("all redem fund trade exists, close_id diff");
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, close_id diff");
	}
	if(m_close_trans.Fuid!=m_stTradeRedem.Fuid){
		TRACE_ERROR("all redem fund trade exists, uid diff");
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, uid diff");
	}
	
	// 扫尾赎回重入关联检查
	if(m_params.getInt("opt_type")==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		// 赎回请求重入
		if(m_close_trans.Fsell_listid[0]==0&&m_stTradeRedem.Fstate==REDEM_ININ){
			gPtrAppLog->debug("all redem fund req has finished");
		// 赎回确认重入
		}else if(0==strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)
			&&m_stTradeRedem.Fstate==REDEM_SUC){
			gPtrAppLog->debug("all redem fund ack has finished");
		}else{
			TRACE_ERROR("all redem fund trade exists, relate close_trans invalid");
			throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem fund trade exists, relate close_trans invalid");
		}
	}
	// 部分赎回重入检查
	if(m_params.getInt("opt_type")==FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM
		&&0==strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)){
			TRACE_ERROR("all redem fund trade exists, input opt_type invalid");
			throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem fund trade exists, input opt_type invalid");
	}

    if(m_stTradeRedem.Fstate == REDEM_ININ || m_stTradeRedem.Fspe_tag == TRADE_RECORD_TIMEOUT)
    {
		//成功单超时的单，也报正常重入
		throw CException(ERR_REPEAT_ENTRY, "fund redem trade exist. ", __FILE__, __LINE__);
    }
	else if(m_stTradeRedem.Fstate == REDEM_SUC)
	{
		//到基金公司赎回已成功的单重入报特殊错误码，便于前置机识别后不再发起到基金公司的请求
		throw CException(ERR_REDEM_SP_SUC_REPEAT_ENTRY, "fund redem from sp success. ", __FILE__, __LINE__);
	}
	else
    {
		//其它状态的赎回不可以重入
		throw CException(ERR_REDEM_REPEAT_ENTRY, "fund redem trade exist.but state cannot be redem now()", __FILE__, __LINE__);
    }
}

/**
  * 生成基金赎回记录，状态: 初始赎回状态
  */
void RedeemCloseReq::BuildFundTrade() throw (CException)
{
	AbstractRedeemSpReq::BuildFundTrade();
    // Ffund_vdate T+1到账日期: 赎回到账日为收益到期日+1日
    string vDate = addDays(m_close_trans.Fprofit_end_date,1);
    strncpy(m_stTradeRedem.Ffund_vdate, vDate.c_str(), sizeof(m_stTradeRedem.Ffund_vdate)-1);

    // 定期相关记录	
    m_stTradeRedem.Fclose_listid = m_close_trans.Fid;
    m_stTradeRedem.Fopt_type = m_params.getInt("opt_type");
    //Freal_redem_amt当前还不知道,设置total，入账的时候再加上扫尾
    m_stTradeRedem.Freal_redem_amt =m_params.getLong("total_fee");
    strncpy(m_stTradeRedem.Fend_date,m_close_trans.Fend_date, sizeof(m_stTradeRedem.Fend_date) - 1);
}

/**
  * 打包输出参数
  */
void RedeemCloseReq::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "end_date", m_close_trans.Fend_date);
}


