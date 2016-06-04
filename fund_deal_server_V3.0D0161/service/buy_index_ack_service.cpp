/**
  * FileName: buy_index_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: ���ࣺ�����깺����
  */


#include "fund_commfunc.h"
#include "buy_index_ack_service.h"

BuyIndexAck::BuyIndexAck(CMySQL* mysql):AbstractBuySpAck(mysql)
{	
    TRACE_DEBUG("[BuyIndexAck] init");
}

/**
  * service step 1: �����������
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
	m_params.readIntParam(bizAttach,"units_usable",0,2); // �ݶ�����.1:��������ȷ�Ϸݶ�;2:��ȷ�ϲ����÷ݶ�; 

	// ָ������Ĭ��ֻʹ��ǰ���շ�
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_FRONT);
	}
    return;
}
/**
  * ����CKV
  */
void BuyIndexAck::updateCkvs()
{
	AbstractBuySpAck::updateCkvs();	
	if(m_doSaveOnly)
	{
		//�����ֻ�����˻�,����Ҫ��ckv		
		return;
	}
	
	if(need_refund)
	{
		return;
	}
	if(INF_PAY_OK == m_optype || INF_PUR_SP_ACK_SUC== m_optype)
	{
		//��¼δȷ�Ϸݶ�		
		//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
		if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
		{
			setFundUnconfirm(m_pFundCon,m_fund_bind.Ftrade_id);
		}

		setFundUnfinishTransCKV(m_pFundCon,m_fund_bind.Ftrade_id);
	}
}
/**
*  ��֧����ȯ
*/
void BuyIndexAck::CheckFundTrade() throw (CException)
{
	// �ȼ�鹫������
	AbstractBuySpAck::CheckFundTrade();
	
	/**
	 * ���ָ��������
	 */
	
	char szErrMsg[128]={0};
	// ��֧��ȯ������
	if("" != string(m_stTradeBuy.Fcoupon_id))
	{
		snprintf(szErrMsg,sizeof(szErrMsg),"[BuyIndexAck]listid[%s] not allow use coupon", m_stTradeBuy.Flistid);
		throw CException(ERR_INDEX_BAD_PARAM, szErrMsg, __FILE__, __LINE__);
	}
	// ��֧�ֺ�Լ��
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
	// ֻ�澯������Ĭ���޸ĳ�0
	if(m_params.getLong("fund_units")!=0)
	{
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���֧��֪ͨ�ṩ�ݶΪ0,Ĭ���޸ĳ�0[%s][%ld]",m_stTradeBuy.Flistid,m_params.getLong("fund_units")); 
		alert(ERR_INDEX_BAD_PARAM,szMsg);
		m_params.setParam("fund_units",0);
	}
	// ���ָ�������������
	strncpy(m_fundIndexTrans.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	if(queryFundTransProcess(m_pFundCon,m_fundIndexTrans))
	{
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���֧��֪ͨ���ڹ���ָ������[%s][%s]",m_stTradeBuy.Flistid,m_fund_bind.Ftrade_id);
		throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
	}
	
	return true;
}

/**
 * ��鲢����ETF��������,��¼��m_params��
 */
void BuyIndexAck::CheckPayTransDate() throw(CException)
{	
	//���㽻������.��������T��,�ݶ�ȷ������T+1��,�״���������T+1��
	string trade_date;
	string fund_vdate;	
	vector<string> tDateVec;
	bool hasDate = queryFundTplusNDates(m_pFundCon,tDateVec,m_acc_time,2);
	if(!hasDate){
		//����������
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "ָ���ͻ����깺T+1������δ����[%s][%s][%zd]",m_stTradeBuy.Flistid,m_acc_time.c_str(),tDateVec.size()); 
		alert(ERR_UNFOUND_TRADE_DATE,szMsg);
		throw CException(ERR_UNFOUND_TRADE_DATE, szMsg, __FILE__, __LINE__);
	}
	// ��������T��
	m_params.setParam("trade_date",tDateVec[0]);
	// �ݶ�ȷ������T+1��
	string confirmDate = m_params.getString("confirm_date");
	if(confirmDate!=""&&confirmDate!=tDateVec[1])
	{
		char szMsg[128]={0};
		snprintf(szMsg, sizeof(szMsg), "ָ���ͻ����깺�ݶ�ȷ��������㲻һ��[%s][%s][%s]",m_stTradeBuy.Flistid,confirmDate.c_str(),tDateVec[1].c_str()); 
		throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);		
	}
	
	// �״���������T+1��
	m_params.setParam("fund_vdate",tDateVec[1]);
	// �ݶ�ȷ������T+1��
	m_params.setParam("confirm_date",tDateVec[1]);

	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)

	{
		// ���δȷ�Ϸݶ�
		strncpy(m_fundUnconfirm.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundUnconfirm.Ftrade_id)-1);
		strncpy(m_fundUnconfirm.Fspid,m_fund_sp_config.Fspid,sizeof(m_fundUnconfirm.Fspid)-1);
		strncpy(m_fundUnconfirm.Ftrade_date,m_params.getString("trade_date").c_str(),sizeof(m_fundUnconfirm.Ftrade_date)-1);
		queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true);
		
		if(m_fundUnconfirm.Flstate==UNCONFIRM_FUND_INVALID)
		{
			char szMsg[128]={0};
			snprintf(szMsg, sizeof(szMsg), "ָ���ͻ����깺������Чδȷ�Ͻ��[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
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
	// �ȴ�����������
	AbstractBuySpAck::BuildFundTradeForPay(stRecord);	
	/**
	 * ����ָ���ͻ�������
	 */
	if(need_refund)
	{
		return;
	}
	// ��¼ָ��������
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
	m_fundIndexTrans.Ffund_units = 0; // �ݶ�δȷ��
	strncpy(m_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_fundIndexTrans.Ffund_net)-1);
	m_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_BUY_UNCONFIRM;
	m_fundIndexTrans.Flstate = PROCESS_TRANS_LSTATE_VALID;
	strncpy(m_fundIndexTrans.Fcreate_time,m_params.getString("systime").c_str(),sizeof(m_fundIndexTrans.Fcreate_time)-1);
	strncpy(m_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_fundIndexTrans.Fmodify_time)-1);
	strncpy(m_fundIndexTrans.Ffinish_time,"9999-12-31 23:59:59",sizeof(m_fundIndexTrans.Ffinish_time)-1);
	strncpy(m_fundIndexTrans.Facc_time,m_acc_time.c_str(),sizeof(m_fundIndexTrans.Facc_time)-1);
		
		
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		bool hasUnconfirmData = m_fundUnconfirm.Fid>0;
			
		if(hasUnconfirmData)
		{
			// ��װupdate ����
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
			// ��װinsert ����
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
	// �ȴ�����������
	AbstractBuySpAck::RecordFundTradeForPay(stRecord);

	if(need_refund)
	{
		return;
	}
	// ����ָ�������������
	insertFundTransProcess(m_pFundCon,m_fundIndexTrans);
	
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// ����ָ����δȷ�Ͻ��	
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
 * ���ݶ�ȷ���������
 */
void BuyIndexAck::CheckFundTradeRepeatForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{	
	char szMsg[128]={0};
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// ���δȷ�Ϸݶ�
		strncpy(m_fundUnconfirm.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundUnconfirm.Ftrade_id)-1);
		strncpy(m_fundUnconfirm.Fspid,m_fund_sp_config.Fspid,sizeof(m_fundUnconfirm.Fspid)-1);
		strncpy(m_fundUnconfirm.Ftrade_date,m_stTradeBuy.Ftrade_date,sizeof(m_fundUnconfirm.Ftrade_date)-1);
		if(!queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true))
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ�������Ҳ���δȷ�Ϸݶ�[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
			throw CException(ERR_REPEAT_ENTRY_DIFF, szMsg, __FILE__, __LINE__);
		}
		
		// �ݶ�һ��
		if(m_params.getLong("fund_units")!=m_stTradeBuy.Freal_redem_amt)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ������ݶһ��[%s][%ld][%ld]",m_fundUnconfirm.Fspid,m_stTradeBuy.Freal_redem_amt,m_params.getLong("fund_units"));  
			throw CException(ERR_REPEAT_ENTRY_DIFF, szMsg, __FILE__, __LINE__);
		}
		
		// ��ֵһ��
		if(m_params.getString("fund_net")!=m_fundUnconfirm.Ffund_net)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ�����뾻ֵ��һ��[%s][%s][%s]",m_fundUnconfirm.Fspid,m_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str());  
			throw CException(ERR_REPEAT_ENTRY_DIFF, szMsg, __FILE__, __LINE__);
		}
	}
	
	//TODO:fundIndexTrans�����߼�:�ڶ������Ӽ���±�����
	// ���ָ�������������
	strncpy(m_fundIndexTrans.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	bool hasProcess=queryFundTransProcess(m_pFundCon,m_fundIndexTrans,true);
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
	{
		if(!hasProcess)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ������,�Ҳ�������ָ������[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);
			throw CException(ERR_INDEX_BUY_REPEAT, szMsg, __FILE__, __LINE__);
		}
		
		// �ݶ�һ��
		if(m_params.getLong("fund_units")!=m_stTradeBuy.Freal_redem_amt||m_params.getLong("fund_units")!=m_fundIndexTrans.Ffund_units)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ������,�ݶһ��[%s][%ld][%ld][%ld]",m_fundIndexTrans.Fspid,m_stTradeBuy.Freal_redem_amt,m_fundIndexTrans.Ffund_units,m_params.getLong("fund_units"));  
			throw CException(ERR_INDEX_BUY_REPEAT, szMsg, __FILE__, __LINE__);
		}
		
		// ��ֵһ��
		if(m_params.getString("fund_net")!=m_fundIndexTrans.Ffund_net)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ������,��ֵ��һ��[%s][%s][%s][%s]",m_fundIndexTrans.Fspid,m_fundIndexTrans.Ffund_net,m_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str());  
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
  * ���ݶ�ȷ��
  */
bool BuyIndexAck::CheckFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// �ȼ�鹫������: �������Ե��������ز�return false��������ȷ������
	AbstractBuySpAck::CheckFundTradeForSucAck(stRecord);
	// ���ҵ������Ƿ����
	CHECK_PARAM_EMPTY("confirm_date");
	CHECK_PARAM_EMPTY("fund_net");
	CHECK_PARAM_EMPTY("net_date");
	CHECK_PARAM_EMPTY("units_usable");
	
	char szMsg[128]={0};
	int unitsUsable = m_params.getInt("units_usable");
	
	// �ж�����
	if(unitsUsable==BUY_ACK_UNITS_CFM_USEABLE && PURCHASE_SUC==m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==0)
	{		
		//���������
		CheckFundTradeRepeatForSucAck(stRecord);
		// ����ɹ��Ĳ�����������룬ֻ�������˻�,�����к�������
		if(ERR_TYPE_MSG)
		{
			m_doSaveOnly=true;
			return false;
		}		
		
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}
	// �ж�����
	if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE && PURCHASE_SUC==m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==TRADE_SPETAG_UNITS_UNUSABLE)
	{
		//�������
		CheckFundTradeRepeatForSucAck(stRecord);
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}
	
	// ȷ�Ϸݶ�,���Ӧ��һ��
	if(m_params.getLong("total_fee")!=m_stTradeBuy.Ftotal_fee)
	{
		snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���ݶ�ȷ�Ͻ�һ��[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_params.getLong("total_fee"),m_stTradeBuy.Ftotal_fee); 
		throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
	}
	
	/** ָ���ͻ���֧��ֱ��ȷ�Ͽ��÷ݶ�**/
	// 1: ȷ�ϲ����÷ݶ�
	if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE)
	{
		// �ݶ�Ӧ����δȷ��
		if(m_stTradeBuy.Freal_redem_amt>0)
		{
			snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���ݶ�ȷ��,������ȷ�Ϸݶ�[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_params.getLong("fund_units"),m_stTradeBuy.Freal_redem_amt); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
			
	}
	// 2: �޸���ȷ�Ϸݶ����
	else if(unitsUsable==BUY_ACK_UNITS_CFM_USEABLE )
	{
		// �����Ա��״̬
		if(PURCHASE_SUC == m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==TRADE_SPETAG_UNITS_UNUSABLE)
		{
			gPtrAppLog->debug("ʹ��ȷ�Ϸݶ����,���״̬��ȷ[%s]",m_stTradeBuy.Flistid);
		}else{
			//���ݿ�״̬���ܸ���
			throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
		}
	}else
	{
		snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���֧�ָ����ͷݶ�[%s][%d][%ld]",m_stTradeBuy.Flistid,unitsUsable,m_params.getLong("fund_units")); 
		throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);
	}
	
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// ���δȷ�Ϸݶ����
		strncpy(m_fundUnconfirm.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundUnconfirm.Ftrade_id)-1);
		strncpy(m_fundUnconfirm.Fspid,m_stTradeBuy.Fspid,sizeof(m_fundUnconfirm.Fspid)-1);
		strncpy(m_fundUnconfirm.Ftrade_date,m_stTradeBuy.Ftrade_date,sizeof(m_fundUnconfirm.Ftrade_date)-1);
		if(!queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true))
		{
			char szMsg[128]={0};
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ���Ҳ���δȷ�Ϸݶ�[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);
		}
		
		if(m_fundUnconfirm.Flstate==UNCONFIRM_FUND_INVALID)
		{
			char szMsg[128]={0};
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ�ϴ�����Чδȷ�Ϸݶ�[%s][%s][%s]",m_fundUnconfirm.Fspid,m_stTradeBuy.Flistid,m_fundUnconfirm.Ftrade_date);  
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);
		}
		// ȷ�Ϸݶ�,δȷ�Ͻ��Ӧ�ô��ڴ�ȷ�Ͻ��
		if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE&&m_fundUnconfirm.Ftotal_fee<m_stTradeBuy.Ftotal_fee)
		{
			snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���ݶ�ȷ��,δȷ�Ͻ��Ӧ�ô��ڴ�ȷ�Ͻ��[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_fundUnconfirm.Ftotal_fee,m_stTradeBuy.Ftotal_fee); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
		// ʹ�ݶ����,�����÷ݶ�Ӧ�ô��ڴ����÷ݶ�
		if(unitsUsable==BUY_ACK_UNITS_CFM_USEABLE&&m_fundUnconfirm.Funuse_units<m_params.getLong("fund_units"))
		{
			snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���ݶ�ȷ��,�����÷ݶ�Ӧ�ô��ڴ����÷ݶ�[%s][%ld][%ld]",m_stTradeBuy.Flistid,m_fundUnconfirm.Funuse_units,m_params.getLong("fund_units")); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
		// ��龻ֵһ����
		if(strcmp(m_fundUnconfirm.Ffund_net,"")!=0&&strcmp(m_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str())!=0)
		{
			snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���ݶ�ȷ��,�ݶֵ��һ����[%s][%s][%s]",m_stTradeBuy.Flistid,m_fundUnconfirm.Ffund_net,m_params.getString("fund_net").c_str()); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
		// ����깺ȷ����һ����
		if(m_params.getString("confirm_date")!=""&&m_fundUnconfirm.Fconfirm_date!=m_params.getString("confirm_date"))
		{
			snprintf(szMsg, sizeof(szMsg), "ָ���ͻ���ݶ�ȷ��,�깺ȷ���ղ�һ����[%s][%s][%s]",m_stTradeBuy.Flistid,m_fundUnconfirm.Fconfirm_date,m_params.getString("confirm_date").c_str()); 
			throw CException(ERR_INDEX_BAD_PARAM, szMsg, __FILE__, __LINE__);	
		}
	}
	
	// ���ָ�������������
	strncpy(m_fundIndexTrans.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	bool hasProcess=queryFundTransProcess(m_pFundCon,m_fundIndexTrans,true);
	//TODO:fundIndexTrans�����߼�:�ڶ������Ӽ���±�����
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
	{
		if(!hasProcess)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ��,�Ҳ�������ָ������[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
		
		if(m_fundIndexTrans.Flstate==PROCESS_TRANS_LSTATE_INVALID)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ��,����ָ��������Ч[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);  
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
		
		// ״̬һ��
		if(m_fundIndexTrans.Fstate!=PROCESS_TRANS_STATE_BUY_UNCONFIRM&&unitsUsable==BUY_ACK_UNITS_UNUSEABLE)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ȷ�Ϸݶ�,����ָ��״̬����ȷ[%s][%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id,m_fundIndexTrans.Fstate);  
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
		if(m_fundIndexTrans.Fstate!=PROCESS_TRANS_STATE_BUY_CONFIRM&&unitsUsable==BUY_ACK_UNITS_CFM_USEABLE)
		{
			snprintf(szMsg, sizeof(szMsg), "ETF�깺ʹ�ݶ����,����ָ��״̬����ȷ[%s][%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id,m_fundIndexTrans.Fstate);  
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
	}
	
	return true;
}

/**
  * ָ���ͻ����깺ȷ��,��װ����
  */ 
void BuyIndexAck::BuildFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// �ȼ�¼��������
	AbstractBuySpAck::BuildFundTradeForSucAck(stRecord);

	// ָ���ͻ������Բ���
	// ����ֵ
	strncpy(stRecord.Ffund_value,m_params.getString("fund_net").c_str(),sizeof(stRecord.Ffund_value)-1);
	stRecord.Freal_redem_amt=m_params.getLong("fund_units");

	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		m_update_fundUnconfirm.Fid = m_fundUnconfirm.Fid;
		strncpy(m_update_fundUnconfirm.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(m_update_fundUnconfirm.Ftrade_id)-1);
		// �жϷݶ��Ƿ����
		int unitsUsable = m_params.getInt("units_usable");
		LONG fundUnits = m_params.getLong("fund_units");

		// ȷ�ϲ����÷ݶ�:���������˻�
		if(unitsUsable==BUY_ACK_UNITS_UNUSEABLE) 
		{
			stRecord.Fspe_tag=TRADE_SPETAG_UNITS_UNUSABLE;
			m_params.setParam("subacc_units",0);
			if(m_fundUnconfirm.Ftotal_fee==m_stTradeBuy.Ftotal_fee)
			{
				// ���ȫ��ȷ��
				m_update_fundUnconfirm.Fstate=UNCONFIRM_FUND_STATE_UNUSABLE;
			}else
			{
				// ����ȷ��
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
		// ʹ��ȷ�Ϸݶ����:��Ҫ�������˻�
		else 
		{	
			stRecord.Fspe_tag=0;
			m_params.setParam("subacc_units",stRecord.Freal_redem_amt);
			if(m_fundUnconfirm.Fstate==UNCONFIRM_FUND_STATE_UNUSABLE)
			{
				// �޸Ĳ����÷ݶ�״̬
				m_update_fundUnconfirm.Fstate = UNCONFIRM_FUND_STATE_NONE;
			}
			// �޸Ĳ����÷ݶ�
			m_update_fundUnconfirm.Funuse_units = m_fundUnconfirm.Funuse_units-fundUnits;
			strncpy(m_update_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundUnconfirm.Fmodify_time)-1);
		}
	}
	
	// ���¹���ָ������
	m_update_fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(m_update_fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(m_update_fundIndexTrans.Ftrade_id)-1);
	
	// �жϷݶ��Ƿ����
	int unitsUsable = m_params.getInt("units_usable");
	LONG fundUnits = m_params.getLong("fund_units");
	// ȷ�ϲ����÷ݶ�:���������˻�
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
	// ʹ��ȷ�Ϸݶ����:��Ҫ�������˻�
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
  *   ָ���ͻ����깺ȷ��,����DB
  */
void BuyIndexAck::RecordFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// �ȸ��¹�������
	AbstractBuySpAck::RecordFundTradeForSucAck(stRecord);
	
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		// ����δȷ�Ͻ���
		strncpy(m_update_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundUnconfirm.Fmodify_time)-1);
		updateFundUnconfirmById(m_pFundCon,m_update_fundUnconfirm);
	}
	
	// ���¹���ָ��������
	//updateFundTransProcess(m_pFundCon,m_update_fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,m_update_fundIndexTrans,m_fundIndexTrans);
}

/**
* ����������
*/
void BuyIndexAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "biz_attach", m_params.getString("biz_attach").c_str());
	return;
}


