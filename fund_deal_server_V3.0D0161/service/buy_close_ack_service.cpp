/**
  * FileName: buy_close_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: ���ࣺ�����깺����
  */


#include "fund_commfunc.h"
#include "buy_close_ack_service.h"

BuyCloseAck::BuyCloseAck(CMySQL* mysql):AbstractBuySpAck(mysql)
{
    TRACE_DEBUG("[BuyCloseAck] init");
    memset(&m_fundCloseTrans, 0, sizeof(FundCloseTrans));
    memset(&m_fundCloseCycle, 0, sizeof(FundCloseCycle));
    
    m_close_fund_seqno = 0;
    m_bCloseBuyTotalAdded = false;
}

/**
  * service step 1: �����������
  */
void BuyCloseAck::parseBizInputMsg(char* szMsg)  throw (CException)
{
	m_params.readStrParam(szMsg,"biz_attach",0,MAX_PARAM_LEN);
	const char* bizAttach = m_params.getString("biz_attach").c_str();
	if(strcmp(bizAttach,"")==0)
	{
		return;
	}
   
   // ��ȡ����ҵ�����:�����ĳɴ�attach�ֶ��л�ȡ
   m_params.readStrParam(bizAttach, "close_end_day", 0, 8);	
   m_params.readIntParam(bizAttach, "user_end_type", 0, 3);
   m_params.readIntParam(bizAttach, "end_sell_type", 0, 3);
   m_params.readStrParam(bizAttach, "end_transfer_spid", 0, 15);
   m_params.readStrParam(bizAttach, "end_transfer_fundcode", 0, 64);
   m_params.readLongParam(bizAttach,"end_plan_amt",0, MAX_LONG);

}

/**
* ����������ȡ�ڲ�����
*/
void BuyCloseAck::CheckParams() throw (CException)
{
	// �ȼ�鹫������
	AbstractBuySpAck::CheckParams();
	
	// ��鶨������
	if(INF_PAY_OK == m_optype)
	{    
		if(CLOSE_FUND_SELL_TYPE_ANOTHER_FUND == m_params.getInt("end_sell_type"))
		{
			CHECK_PARAM_EMPTY("end_transfer_spid"); 
			CHECK_PARAM_EMPTY("end_transfer_fundcode"); 
		}

		if(CLOSE_FUND_END_TYPE_PATRIAL_REDEM == m_params.getInt("user_end_type") && 0 == m_params.getLong("end_plan_amt"))
		{
			throw EXCEPTION(ERR_BAD_PARAM, "���������ָ����ؽ��"); 
		}
	}
}
bool BuyCloseAck::CheckPayRepeat() throw (CException)
{
	try{
		return AbstractBuySpAck::CheckPayRepeat();
			
	}catch(CException& e)
	{
		if (ERR_REPEAT_ENTRY == (unsigned)e.error()) 
		{
			CheckClosePayRepeat();
		}
		throw;
	}
}

/**
 * ��鶨��֧��ȷ������
 */
bool BuyCloseAck::CheckClosePayRepeat() throw (CException)
{
	char errMsg[128]={0};
	// ���������߼�
	if(m_stTradeBuy.Fclose_listid<=0)
	{
		snprintf(errMsg, sizeof(errMsg), "���ڻ���֧��֪ͨ���룬�Ҳ���close_listid[%s]",m_stTradeBuy.Flistid); 
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	// ���룬���ڶ��ڲ�ƷҪ��ѯ���������ڷ��ط�տ�ʼʱ��
	m_fundCloseTrans.Fid = m_stTradeBuy.Fclose_listid;
	strncpy(m_fundCloseTrans.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	if(!queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false))
	{
		snprintf(errMsg, sizeof(errMsg), "���ڻ���֧��֪ͨ���룬�Ҳ�������close_trans[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id); 
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	if(0!=strcmp(m_stTradeBuy.Fend_date,m_fundCloseTrans.Fend_date))
	{
		snprintf(errMsg, sizeof(errMsg), "���ڻ���֧��֪ͨ���룬Fend_date��һ��[%s][%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Fend_date,m_fundCloseTrans.Fend_date); 
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	if(0!=strcmp(m_stTradeBuy.Ftrade_date,m_fundCloseTrans.Ftrans_date))
	{
		snprintf(errMsg, sizeof(errMsg), "���ڻ���֧��֪ͨ���룬Ftrans_date��һ��[%s][%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_date,m_fundCloseTrans.Ftrans_date);
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	m_params.setParam("trans_date", m_stTradeBuy.Ftrade_date);
	throw CException(ERR_REPEAT_ENTRY, "���ڻ���֧��֪ͨ����! ", __FILE__, __LINE__);
}

/**
  *  ֧��֪ͨ������
  */
bool BuyCloseAck::CheckParamsForPay() throw (CException)
{
	bool result = AbstractBuySpAck::CheckParamsForPay();
	// ����ԭ��û�д���fund_units����
	// ������������֧��֪ͨ��ʱ���¼�ݶ�
	if(m_params.getLong("fund_units")==0)
	{
		m_params.setParam("fund_units",m_params.getLong("total_fee"));
	}
	return result;
}

/**
 * ��鲢���㽻����,��¼��m_params��
 */
void BuyCloseAck::CheckPayTransDate() throw(CException)
{	
	//���㽻������	
	strncpy(m_fundCloseCycle.Fdate, calculateFundDate(m_acc_time).c_str(), sizeof(m_fundCloseCycle.Fdate) - 1);
	strncpy(m_fundCloseCycle.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(m_fundCloseCycle.Ffund_code) - 1);
	queryFundCloseCycle(m_pFundCon, m_fundCloseCycle, false);
	string trade_date = toString(m_fundCloseCycle.Ftrans_date);
	string fund_vdate = toString(m_fundCloseCycle.Ffirst_profit_date);
		
	if(	trade_date.empty()){
		//����������
		alert(ERR_UNFOUND_TRADE_DATE,"trade_date unfound:"+string(m_stTradeBuy.Flistid));
		gPtrAppLog->error("close cycle unfound[%s], systime[%s]", m_stTradeBuy.Flistid, m_params.getString("systime").c_str());
		throw CException(ERR_UNFOUND_TRADE_DATE, "trade_date unfound! ", __FILE__, __LINE__);
	}	
	m_params.setParam("trade_date",trade_date);
	m_params.setParam("fund_vdate",fund_vdate);

}
// ����û������ڹ���
void BuyCloseAck::checkUserPermissionBuyCloseFund()
{
	//�Ѿ�����Ҫ�˿����
	if(need_refund)
	{
		return;
	}

	//�Ƿ�ղ�Ʒ�����
	if(m_fund_sp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}

	strncpy(m_fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	strncpy(m_fundCloseTrans.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(m_fundCloseTrans.Ffund_code) - 1);
	try
	{
		m_close_fund_seqno = checkPermissionBuyCloseFund(m_fundCloseTrans, m_fund_sp_config, m_fundCloseCycle.Ftrans_date,m_fundCloseCycle.Fdue_date, true);
	}
	catch(CException& e)
	{
		//��Ȩ�޹��򣬱���˿�
		refund_desc = "�����㹺���ڲ�ƷȨ��";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_14;
	}

	m_params.setParam("trans_date", m_fundCloseCycle.Ftrans_date);
}


bool BuyCloseAck::CheckFundTradeForPay() throw (CException)
{
	// �ȼ�鹫������
	if(!AbstractBuySpAck::CheckFundTradeForPay())
	{
		return false;
	}
	// ��鶨�ڹ�������
	checkUserPermissionBuyCloseFund();
	return true;		
}

void BuyCloseAck::BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	// �ȼ�¼������Ϣ
	AbstractBuySpAck::BuildFundTradeForPay(stRecord);
	
	//���ڲ�Ʒ��¼
	if(need_refund)
	{
		return;
	}

	//�Ƿ�ղ�Ʒֱ�ӷ���
	if(m_fund_sp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}
	
	FundCloseTrans fundCloseTrans;	
	
	//����������£����򴴽��¼�¼
	if(m_close_fund_seqno == m_fundCloseTrans.Fseqno)
	{
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		fundCloseTrans.Fid=m_fundCloseTrans.Fid;
		// ���δ���ò�����
		if(m_params.getInt("user_end_type")!=0){
			fundCloseTrans.Fuser_end_type = m_params.getInt("user_end_type");
			fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		}
		if(m_params.getInt("end_sell_type")!=0){
			fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO ���ݲ�Ʒ�������
			strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
			strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		}
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);

		fundCloseTrans.Fpay_type = m_fundCloseTrans.Fpay_type | CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_fundCloseTrans.Fstart_total_fee + m_stTradeBuy.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_fundCloseTrans.Fcurrent_total_fee +  m_stTradeBuy.Ftotal_fee;

		saveFundCloseTrans(fundCloseTrans,m_fundCloseTrans,stRecord.Flistid,PURTYPE_PURCHASE);

		//�û���¼�������׵���
		strncpy(stRecord.Fend_date, m_fundCloseTrans.Fend_date, sizeof(stRecord.Fend_date) - 1);
		stRecord.Fclose_listid = m_fundCloseTrans.Fid;
	}
	else
	{
		memset(&fundCloseTrans,0,sizeof(FundCloseTrans));
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		strncpy(fundCloseTrans.Fspid, m_stTradeBuy.Fspid, sizeof(fundCloseTrans.Fspid) - 1);
		strncpy(fundCloseTrans.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundCloseTrans.Ffund_code) - 1);
		fundCloseTrans.Fuid =  m_params.getInt("uid");
		fundCloseTrans.Fseqno = m_close_fund_seqno;
		fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO ���ݲ�Ʒ�������
		fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
		strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);

		fundCloseTrans.Fpay_type = CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_stTradeBuy.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_stTradeBuy.Ftotal_fee;
		//�´������δ������ʹ��Ĭ��ֵ
		fundCloseTrans.Fuser_end_type = (m_params.getInt("user_end_type") != 0) ? m_params.getInt("user_end_type") : CLOSE_FUND_END_TYPE_ALL_EXTENSION; //TODO ���ݲ�Ʒ�������

		m_params.setParam("trans_date", m_fundCloseCycle.Ftrans_date);//���ڷ��ز���ʹ��

		strncpy(fundCloseTrans.Ftrans_date, m_fundCloseCycle.Ftrans_date, sizeof(fundCloseTrans.Ftrans_date) - 1);
		strncpy(fundCloseTrans.Ffirst_profit_date, m_fundCloseCycle.Ffirst_profit_date, sizeof(fundCloseTrans.Ffirst_profit_date) - 1);
		strncpy(fundCloseTrans.Fopen_date, m_fundCloseCycle.Fopen_date, sizeof(fundCloseTrans.Fopen_date) - 1);
		strncpy(fundCloseTrans.Fbook_stop_date, m_fundCloseCycle.Fbook_stop_date, sizeof(fundCloseTrans.Fbook_stop_date) - 1);
		strncpy(fundCloseTrans.Fstart_date, m_fundCloseCycle.Fstart_date, sizeof(fundCloseTrans.Fstart_date) - 1);
		strncpy(fundCloseTrans.Fend_date, m_fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fend_date) - 1);
		strncpy(fundCloseTrans.Fdue_date, m_fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fdue_date) - 1);
		strncpy(fundCloseTrans.Fprofit_end_date, m_fundCloseCycle.Fprofit_end_date, sizeof(fundCloseTrans.Fprofit_end_date) - 1);
		strncpy(fundCloseTrans.Fchannel_id, m_stTradeBuy.Fchannel_id, sizeof(fundCloseTrans.Fchannel_id) - 1);
		fundCloseTrans.Fstate= CLOSE_FUND_STATE_PENDING;
		fundCloseTrans.Flstate = LSTATE_VALID;
		strncpy(fundCloseTrans.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fcreate_time) - 1);
		strncpy(fundCloseTrans.Facc_time, m_acc_time.c_str(), sizeof(fundCloseTrans.Facc_time) - 1);//�״βż�¼�����²��޸�

		createFundCloseTrans(fundCloseTrans,stRecord.Flistid,PURTYPE_PURCHASE);

		//�û���¼�������׵���,�õ����ձ�ʶΨһ����
		strncpy(stRecord.Fend_date, m_fundCloseCycle.Fdue_date, sizeof(stRecord.Fend_date) - 1);
		stRecord.Fclose_listid = fundCloseTrans.Fid;
	}

}

// ֧��֪ͨ�̻����֪ͨ
// ��齻�׵�����
void BuyCloseAck::CheckFundTradeForPayAck() throw (CException)
{
	// ���ͨ�ò���
	AbstractBuySpAck::CheckFundTradeForPayAck();

	// ��鶨�ڵ�����
	if(INF_PAY_SP_INFO_SUC != m_optype)
	{
		return;
	}
	if(m_stTradeBuy.Fclose_listid <=0)
	{
		//�Ƿ�ջ��𲻼��
		return;
	}

	m_fundCloseTrans.Fid = m_stTradeBuy.Fclose_listid;
	strncpy(m_fundCloseTrans.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false);

	if(m_params.getString("close_end_day") != m_fundCloseTrans.Fend_date)
	{
		alert(ERR_DIFF_END_DATE, (string("Fid:") + toString(m_fundCloseTrans.Fid) + "����˾���صķ�ս����պͱ��ؼ��㲻һ��").c_str());
		throw CException(ERR_DIFF_END_DATE, "end date diff.", __FILE__, __LINE__); 
	}
}


void BuyCloseAck::updateCkvs()
{
	AbstractBuySpAck::updateCkvs();
	
	if(m_doSaveOnly)
	{
		//�����ֻ�����˻�,����Ҫ��ckv		
		return;
	}
	
	if(INF_PAY_OK == m_optype &&  false == need_refund)
	{
		//��¼����ckv
		setFundCloseTransToKV(m_fund_bind.Ftrade_id, m_stTradeBuy.Ffund_code);
	}
}


/**
  * ����������
  */
void BuyCloseAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	//���ص�trans_dateΪ�ⲿ�����close_start_day��ǰ�û��ͻ���˾Э������⵼��
	CParams bizRequestMsg;
    //����Ҫ�޸ĵ�����szValue
	bizRequestMsg.setParam("close_start_day",m_params.getString("trans_date").c_str());
	CUrlAnalyze::setParam(rqst->odata, "biz_attach",bizRequestMsg.pack().c_str());
}


