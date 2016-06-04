/**
  * FileName: abstract_buy_sp_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: ���ࣺ�����깺����
  */


#include "fund_commfunc.h"
#include "abstract_buy_sp_ack_service.h"

AbstractBuySpAck::AbstractBuySpAck(CMySQL* mysql)
{
	m_pFundCon = mysql;

	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
	memset(&m_controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
    memset(&m_freeze_fund,0,sizeof(ST_FREEZE_FUND));

	need_refund = false;
	m_fund_bind_exist =false;
	refund_desc = "";
	m_doSaveOnly = false;
	m_bBuyTotalAdded = false;
	refund_reason = FUND_REFUND_REASON_0;

}

/**
  * �������е��̻�����,�����ظ���ѯ
  */
void AbstractBuySpAck::setSpConfig(FundSpConfig fundSpConfig)
{
	m_fund_sp_config = fundSpConfig;
}

/**
  * service step 1: �����������
  */
void AbstractBuySpAck::parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg)  throw (CException)
{
	// Ҫ�����������ݣ��ײ��ʹ��	
	m_request = rqst;

    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // ����ԭʼ��Ϣ    
    TRACE_DEBUG("[abstract_buy_sp_ack_service] receives: %s", szMsg);

	// ��ȡ����
	m_params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "uin", 0, 64);
	m_params.readStrParam(szMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 0, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readStrParam(szMsg, "sp_billno", 0, 32);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "op_type", 1, 7);
	m_params.readStrParam(szMsg, "bind_serialno", 0, 64);
	m_params.readStrParam(szMsg, "desc", 0, 128);
	m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "pay_time", 0, 20);//��ʽ:2013-12-30 15:21:41
	m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token
    m_params.readStrParam(szMsg, "coupon_id", 0, 32);
	m_params.readLongParam(szMsg,"fund_units",0,MAX_LONG); // �깺ȷ�Ϸݶ�
	m_params.readLongParam(szMsg,"charge_fee",0,MAX_LONG); // ������
	m_params.readStrParam(szMsg,"charge_type",0,1); // �շѷ�ʽ

	GetTimeNow(szTimeNow);
	m_params.setParam("systime", szTimeNow);

	m_acc_time = szTimeNow;

	m_optype = m_params.getInt("op_type");

	if(m_optype == INF_PAY_OK)
	{
		//֧��ȷ��ʱuid����
		m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	}

    parseBizInputMsgComm(szMsg);
    parseBizInputMsg(szMsg); // ��ȡҵ�����

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);


}

/**
* ����������ȡ�ڲ�����
*/
void AbstractBuySpAck::CheckParams() throw (CException)
{	
	if(INF_PAY_OK == m_optype)
	{
	    //��ѯ��У������кŲ���,���û���ѯ�󿨽ӿ�Ҫ�ŵ�����֮�⣬���������ʱ����
        checkBindserialno();
		
	}else if(INF_PUR_SP_REQ_SUC == m_optype)
	{
		CHECK_PARAM_EMPTY("sp_billno");   
	}
	
	// ����շѷ�ʽ����ֵ����
	if(!isDigitString(m_params.getString("charge_type").c_str()))
	{
        throw CException(ERR_BAD_PARAM, string("Param is not a int value:charge_type=")+ m_params.getString("charge_type"), __FILE__, __LINE__);
	}
}

void AbstractBuySpAck::checkBindserialno()throw (CException)
{
    if (m_params.getString("bind_serialno").empty() || m_params.getString("uin").empty() )
    {
        return;
    }
    
    FundPayCard fund_pay_card;
    memset(&fund_pay_card, 0, sizeof(FundPayCard));
    strncpy(fund_pay_card.Fqqid, m_params["uin"], sizeof(fund_pay_card.Fqqid) - 1);
    bool isFundPayCard = queryFundPayCard(m_pFundCon,fund_pay_card, false);

    //��������Ұ����к�һ��
    if(isFundPayCard && m_params.getString("bind_serialno") == fund_pay_card.Fbind_serialno)
    {
        m_params.setParam("pay_bank_type", fund_pay_card.Fbank_type);
        m_params.setParam("pay_card_tail", fund_pay_card.Fcard_tail);
        return ;
    }

    if(!queryBindCardInfo(fund_pay_card.Fqqid, m_params.getString("bind_serialno"), m_bindCareInfo))
    {
        m_bindCareInfo.clear(); //�쳣����������д������˴�ֻ����У��
    }else
    {
        m_params.setParam("pay_bank_type", m_bindCareInfo["bank_type"]);
        m_params.setParam("pay_card_tail", m_bindCareInfo["card_tail"]);
    }
}


/**
* ִ���깺����
*/
void AbstractBuySpAck::excute() throw (CException)
{
	try
	{
		CheckParams();

		/* �������� */
		m_pFundCon->Begin();
		
		/* ckv�����ŵ�����֮�������ύ*/
		gCkvSvrOperator->beginCkvtrans();

		/* ��ѯ�����׼�¼ */
		CheckFundTrade();

		/* �������˻���¼�����û��������������������Ե����û��Ĳ��ɲ������� */
		CheckFundBind();

		/* �������˻��󶨻���˾�����˻���¼ */
		CheckFundBindSpAcc();

		/* ���»����׼�¼ */
		UpdateTradeState();

		/* �ύ���� */
		m_pFundCon->Commit();
		
		/* ���¸���ckv ,��������֮���Ǳ�������ع�ȴд��ckv������*/
        gCkvSvrOperator->commitCkvtrans();
		updateCkvs();

		/* �жϸ������˻� */
		doSave();

        /* �����û��޶� */
        updateExauAuthLimitNoExcp();

	}
	catch (CException& e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s,rollback ckv and db ops", e.file(), e.line(), e.error(), e.what());
		
		//�ع�dbǰ�Ȼع�����ckv
		gCkvSvrOperator->rollBackCkvtrans();

		m_pFundCon->Rollback();

		if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
			&& (ERR_REGOK_ALREADY != (unsigned)e.error()))
		{
			throw;
		}
	}
}

void AbstractBuySpAck::updateExauAuthLimitNoExcp()
{
    //pc����֧���ɹ��ż��޶�
    if ((m_stTradeBuy.Fpay_channel != PAY_TYPE_WEB) || (m_optype != INF_PAY_OK))
    {
        TRACE_DEBUG("pay_channel %d, op_type: %d", m_stTradeBuy.Fpay_channel, m_optype);
        return;
    }

	try
	{
		updateBalanceFetchExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid,m_params.getLong("total_fee"),m_fund_bind.Fcre_id, 
            FUND_BUY_PC_CHANNEL_EXAU_REQ_TYPE);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
	}
}

/*
* ��ѯ�����˻��Ƿ����
*/
void AbstractBuySpAck::CheckFundBind() throw (CException)
{
	if(need_refund)
	{
		return;
	}

	if(	m_optype ==	INF_PAY_SP_INFO_SUC || m_optype == INF_PAY_SP_INFO_TIMEOUT)
	{
		//֧��ȷ�ϵĲ���飬һ��Ҫ�ɹ�
		return;
	}

	if(!m_params.getString("trade_id").empty()) 
	{
		//ʹ��forupdate ��ѯ����ֹ�����޸��˻���Ϣ
		m_fund_bind_exist = QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &m_fund_bind, true);
	}
	else
	{
		m_fund_bind_exist = QueryFundBindByUin(m_pFundCon, m_params.getString("uin").c_str(), &m_fund_bind, true);
	}

	if(!m_fund_bind_exist)
	{
		// ֧��֪ͨ�ӿ��ж��˿�,  ��֧���ӿ��쳣
		if(m_optype !=	INF_PAY_OK)
		{
			throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
		}
		// �ж��˿�
		//����û��Ѿ�ע�����˿�
		if (m_params.getInt("uid") !=0 && checkIsUserUnbind(m_pFundCon, m_params.getInt("uid")))
		{ 
			TRACE_ERROR("user unbind ,so refund");
			refund_desc = "�û��Ѿ�ע��";
			need_refund = true; 
			refund_reason = FUND_REFUND_REASON_1;
		}      
		return; //û��ѯ��������MQ��û�������ܱ���Ҳ�����˿�˿��ڼ�����˾��Ϣ�ĵط�ͳһ����
		
	}
	// �˻�������Ϣ��һ����ֱ�Ӵ��˿���
	if(m_params.getInt("uid") != 0 && m_fund_bind.Fuid != 0 && m_params.getInt("uid") != m_fund_bind.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
			m_fund_bind.Fuid, m_params.getInt("uid"));
		refund_desc = "�깺�����֧��uid��һ��";
		need_refund = true; 
		refund_reason = FUND_REFUND_REASON_2;
	}
	if( !(m_params.getString("uin").empty()) && m_params.getString("uin") != m_fund_bind.Fqqid)
	{
		TRACE_ERROR("Fqqid in db=%s diff with input=%s", 
			m_fund_bind.Fqqid, m_params.getString("uin").c_str());
		refund_desc = "�깺�����֧��uin��һ��";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_3;
	}
	if(m_params.getString("trade_id").empty())
	{
		m_params.setParam("trade_id",m_fund_bind.Ftrade_id);
	}

}

bool AbstractBuySpAck::payNotifyOvertime(string pay_suc_time)
{
	if(pay_suc_time.size() == 14)
	{
		//YYYYMMDDHHMMSS תYYYY-MM-DD HH:MM:SS
		pay_suc_time = changeDatetimeFormat(pay_suc_time);
	}
	int pay_time = toUnixTime(pay_suc_time.c_str());
	if(pay_time + gPtrConfig->m_AppCfg.paycb_overtime_inteval < (int)(time(NULL)) )
	{
		return true;	
	}

	return false;
}

/*
*����Ƿ�󶨻���˾�ʺţ����ҿɽ���
*/
void AbstractBuySpAck::CheckFundBindSpAcc() throw (CException)
{
	if(need_refund && m_optype == INF_PAY_OK)
	{
		//ǰ�淢�ֽ��׵�֧���û��ͷ����û���һ�µģ����˿��Ҫ�ڴ����˴����������˻����Ѿ���ͨ�����ƻ�ʱ������Ĵ����Ѿ��޷�����
		//���ҿ��ܵ��±���������˻��ķ���
		return; 
	}

	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	bool bind_spacc_exist = queryFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false); 
	
	char bindErrMsg[256] = {0};
	unsigned bindErrCode = 0;
	string refundNowMsg = ""; //  �����˿�˵��
	// �����ʺ��˻�������
	if(!bind_spacc_exist||BIND_SPACC_INIT==m_fund_bind_sp_acc.Fstate)
	{
		snprintf(bindErrMsg, sizeof(bindErrMsg), "�û�[%s]���̻�[%s]�ʻ�������", m_params.getString("trade_id").c_str(),m_params.getString("spid").c_str());
		bindErrCode=ERR_NOT_BIND_SP_ACC;
	}	
	// �����˻�������
	else if(LSTATE_FREEZE == m_fund_bind_sp_acc.Flstate)
	{
		snprintf(bindErrMsg, sizeof(bindErrMsg), "�û�[%s]���̻�[%s]�ʻ��Ѷ���", m_params.getString("trade_id").c_str(),m_params.getString("spid").c_str());
		bindErrCode=ERR_SP_ACC_FREEZE;
		refundNowMsg = "�˻�������";
		refund_reason = FUND_REFUND_REASON_5;
	}	
	// �����˻�����ʧ��
	else if(BIND_SPACC_SUC!= m_fund_bind_sp_acc.Fstate)
	{
		snprintf(bindErrMsg, sizeof(bindErrMsg), "�û�[%s]���̻�[%s]�ʻ�����ʧ��", m_params.getString("trade_id").c_str(),m_params.getString("spid").c_str());
		bindErrCode=ERR_FUND_BIND_SPACC_FAIL;
		refundNowMsg = "�˻�����ʧ��";
		refund_reason = FUND_REFUND_REASON_6;
	}	
	// ����������
	else{		
		return;
	}

	// ͳһ���ʧ����־,������ʧ���߼�
	TRACE_ERROR("%s",bindErrMsg);
	
	// ֧��֪ͨ�����˿�
	if(INF_PAY_OK == m_optype&&refundNowMsg!=""){ //�����˿�
		refund_desc = refundNowMsg;
		need_refund = true;
	}
	//֧��֪ͨ��ʱ�˿�
	else if(INF_PAY_OK == m_optype&&payNotifyOvertime(m_params.getString("pay_time"))){
		refund_desc = "֧������һ��ʱ����δ�����ɹ�ת�˿�";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_7;
	}
	//ֱ���״�,֧��֪ͨ�ӳٵȴ�֧���ص�
	else{ 
		throw EXCEPTION(bindErrCode, bindErrMsg);
	}

}


/**
* �������׼�¼�Ƿ��Ѿ�����,��鶩��ǰ��״̬
*/
void AbstractBuySpAck::CheckFundTrade() throw (CException)
{
	// û�й����¼������
	if(!QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_BUY, &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	if (m_stTradeBuy.Fpurpose != PURPOSE_BALANCE_BUY ) //����ͨ���֧��
	{
		if ((INF_PAY_OK == m_optype)) // ����깺
		{
			CHECK_PARAM_EMPTY("cft_trans_id");  
		} 
	}

	// ����״̬��Ч������
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund buy record, lstate is invalid. listid[%s], uid[%d] ", m_stTradeBuy.Flistid, m_stTradeBuy.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy record, lstate is invalid. ", __FILE__, __LINE__);
	}

	//���״̬�Ϸ���
	if(m_stTradeBuy.Fstate<0||m_stTradeBuy.Fstate>=TRADE_STATE_SIZE)
	{
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state is invalid", __FILE__, __LINE__);
	}
	// ����˿����,���������˿������
	if(m_stTradeBuy.Fstate==PURCHASE_APPLY_REFUND||m_stTradeBuy.Fstate==PURCHASE_REFUND_SUC)
	{
		throw CException(ERR_BUY_RECORD_NEED_REFUND, "fund purchase record has refund! ", __FILE__, __LINE__);
	}

	// У��ؼ�����
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}

	if(m_params.getString("spid") != m_stTradeBuy.Fspid)
	{
		gPtrAppLog->error("fund buy, spid is different! spid in db[%s], spid input[%s] ", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy, spid is different!", __FILE__, __LINE__);
	}

	//�����˻���Ϣ��һ����ֱ�Ӵ��˿��ǣ���cookie �û�֧���ص���ʱ���׵��е�uid ��֧���ص��Ĳ�һ���������û�����δ�������˻�
	//�������͵��˿�Ҫ�ǳ�С�ģ���Ϊ���׵����е�uid �������uid ��һ�£��Ƿ����ֱ���˿�
	if(m_params.getInt("uid") != 0 && m_stTradeBuy.Fuid != 0 && m_params.getInt("uid") != m_stTradeBuy.Fuid)
	{
		ST_FUND_BIND m_fund_bind_query; 
		bool bind_exist = QueryFundBindByUid(m_pFundCon, m_stTradeBuy.Fuid, &m_fund_bind_query, false);
		if(!bind_exist)
		{
			throw CException(ERR_FIND_BIND_NOT_EXIST, "QueryFundBindByUid fail!", __FILE__, __LINE__);
		}
		else
		{
			//�м�¼���ɹ���QueryFundBindByUid�鵽��uid,fqqid���õ�������
			if(true == checkInnerBalancePayForReBuy(m_pFundCon,m_params["fund_trans_id"],m_params["total_fee"],m_fund_bind_query.Fqqid))
			{
				m_params.setParam("uin", m_fund_bind_query.Fqqid);
				m_params.setParam("uid", m_fund_bind_query.Fuid);
			}
			else
			{
				//û�м�¼�����˿��߼�
				TRACE_ERROR("uid in db=%d diff with input=%d", m_stTradeBuy.Fuid, m_params.getInt("uid"));
				refund_desc = "�깺��¼uid��֧��uid��һ��";
				need_refund = true; //�����׳��쳣���������ֹ�깺���󣬵����깺ȷ���޷�����
				refund_reason = FUND_REFUND_REASON_8;
			}	
		}
	}

	// �õ��˻������Ϣ
	m_params.setParam("trade_id", m_stTradeBuy.Ftrade_id);
	m_params.setParam("sub_trans_id", m_stTradeBuy.Flistid);//���ڸ������˻�

}

/**
*����ǰ��״̬���������ͣ�����Ӧ�Ĵ��� 
*/
void AbstractBuySpAck::UpdateTradeState()
{
	switch (m_optype)
	{
	case INF_PUR_SP_REQ_SUC:
	case INF_PUR_SP_REQ_FAIL:
		UpdateFundTradeForReq();
		break;

	case INF_PAY_OK:
		UpdateFundTradeForPay();
		break;

	case INF_PAY_SP_INFO_SUC:
	case INF_PAY_SP_INFO_TIMEOUT:
		UpdateFundTradeForPayAck();
		break;
	case INF_PUR_SP_ACK_SUC:
		UpdateFundTradeForSucAck();
		break;
	default:
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;

	}

}

/**
* ȯ���ѽӿ�
* ����ȯ�깺��״̬����ȯ״̬��ʵ�ʳ�ֵ�Ȳ���
*/
void AbstractBuySpAck::UpdateFundTradeForCoupon() throw (CException)
{
	gPtrAppLog->debug("UpdateFundTradeForCoupon called, listid [%s] cft_trans_id [%s]",m_params.getString("fund_trans_id").c_str(),m_params.getString("cft_trans_id").c_str());

	//ȷ�Ϸ�ȯ״̬Ϊδʹ�ã����򱨴�
	//�Ȳ����ˣ��������ȯ���񸺵�
	//���û��couponid���򲻸�ȯ�����첽��Ϣ
	if("" == string(m_stTradeBuy.Fcoupon_id))
	{
		//TRACE_ERROR("UpdateFundTradeForCoupon, coupon_id empty!");
		return;
	}

	//���û��΢��֧��accid�����޷�����ȯ����
	if(m_params.getString("uin").empty())
	{
		//TRACE_ERROR("UpdateFundTradeForCoupon, uin empty!");
		return;
	}
	
	// ֻ֧��֧��ʵʱȷ�Ϸݶ���ȯ
	if(m_fund_sp_config.Fbuy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		char szErrMsg[128] = {0};
		snprintf(szErrMsg, sizeof(szErrMsg), "[AbstractBuySpAck]�̻�[%s]�깺ȷ������[%d]��֧����ȯ,����[%s]",m_fund_sp_config.Fspid,m_fund_sp_config.Fbuy_confirm_type,m_params.getString("fund_trans_id").c_str());		  
		alert(ERR_BUDAN_TOLONG, szErrMsg);
		return;
	}

	//�����첽�����ȯ���񣬽���ȯ����
	char szMsg[MAX_MSG_LEN + 1] = {0};
	//�ж��˻����� ��Q-1��΢��-2
    string strAcctType="1";
	string::size_type idx = m_params.getString("uin").find("@wx.tenpay.com");
    if ( idx != string::npos ) strAcctType="2";
	// ��װ�ؼ�����
	CUrlAnalyze::setParam(szMsg, "acct_type", strAcctType.c_str(), true);
	CUrlAnalyze::setParam(szMsg, "acct_id", m_params.getString("uin").c_str());
	CUrlAnalyze::setParam(szMsg, "coupon_id", m_stTradeBuy.Fcoupon_id);
	CUrlAnalyze::setParam(szMsg, "listid", m_stTradeBuy.Flistid);
	CUrlAnalyze::setParam(szMsg, "spid", m_stTradeBuy.Fspid);
	CUrlAnalyze::setParam(szMsg, "fund_code", m_stTradeBuy.Ffund_code);
	CUrlAnalyze::setParam(szMsg, "channel_id", m_stTradeBuy.Fchannel_id);
	CUrlAnalyze::setParam(szMsg, "client_ip", m_params.getString("client_ip").c_str());
	CUrlAnalyze::setParam(szMsg, "total_fee", m_stTradeBuy.Ftotal_fee);

	string token = strAcctType +"|";
	token = token + m_params.getString("uin") +"|";
	token = token + m_stTradeBuy.Fcoupon_id + "|";
	token = token + m_stTradeBuy.Flistid + "|";
	token = token + m_stTradeBuy.Fspid + "|";
	token = token + m_stTradeBuy.Ffund_code + "|";
	token = token + m_stTradeBuy.Fchannel_id + "|";
	token = token + m_params.getString("client_ip") + "|";
	token = token + toString(m_stTradeBuy.Ftotal_fee) + "|";
	token = token + gPtrConfig->m_AppCfg.coupon_key;

	char md5token[32+1] = {0};
	getMd5(token.c_str(),token.length(),md5token);
	CUrlAnalyze::setParam(szMsg, "token", md5token);

	sendCouponMsg2Mq(szMsg);

	return;
}

/**
* �깺���������
*/

void AbstractBuySpAck::UpdateFundTradeForReq() throw (CException)
{	
	gPtrAppLog->debug("fund buy pay, state [%d] m_optype[%d]", m_stTradeBuy.Fstate,m_optype);

	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	int state =0;
	if(INF_PUR_SP_REQ_SUC == m_optype)
	{
		if(PAY_INIT == m_stTradeBuy.Fstate)
		{
			//�������
			gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
			throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
		}
		state = PAY_INIT;//�깺����ɹ���ת�������
	}
	else
	{
		if(PUR_REQ_FAIL == m_stTradeBuy.Fstate)
		{
			//�������
			gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
			throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
		}
		state = PUR_REQ_FAIL;
	}

	if(CREATE_INIT != m_stTradeBuy.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_NONE);
	}
	stRecord.Fstate = state;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	strncpy(stRecord.Fcharge_type, m_params.getString("charge_type").c_str(), sizeof(stRecord.Fcharge_type) - 1);
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	stRecord.Fcharge_fee= m_params.getLong("charge_fee");

	if(m_fund_bind.Fuid != 0 && m_stTradeBuy.Fuid != 0)
	{
		//�û����°�uid���еķֿ�ֱ�
		stRecord.Fuid = m_fund_bind.Fuid;
	}

	//����trade_id,���½��׼�¼ʱ��Ҫʹ��
	SCPY(stRecord.Ftrade_id, m_fund_bind.Ftrade_id);

	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"), (m_fund_bind.Fuid == 0 || m_stTradeBuy.Fuid == 0) ? false : true);
}

/**
* ���֧��֪ͨ�������
* ���ɹ�,������������:return true
* ���ʧ��,�����к�������: return false
*/
bool AbstractBuySpAck::CheckPayRepeat() throw (CException)
{
	// ״̬��֧���ɹ�״̬֮ǰ,��������,��������
	if(PURCHASE_STATE_ORDER[PAY_OK]>PURCHASE_STATE_ORDER[m_stTradeBuy.Fstate])
	{
		return true;
	}
	// �����깺ʧ���˿�
	if(m_stTradeBuy.Fstate==PUR_REQ_FAIL)
	{
		need_refund=true;
		refund_desc="�����깺ʧ���˿�";
		refund_reason = FUND_REFUND_REASON_15;
		return true;
	}
	// �������

	// �˿�쳣
	if(m_stTradeBuy.Fstate==PURCHASE_REFUND_SUC||m_stTradeBuy.Fstate==PURCHASE_APPLY_REFUND)
	{
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase can not update! ", __FILE__, __LINE__);
	}
	
	// ����ɹ��Ĳ�������������룬ֻ�������˻�,�����к�������
	if(ERR_TYPE_MSG&& m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		//�������˻�ȷ�Ϸݶ�
		m_params.setParam("subacc_units",m_stTradeBuy.Freal_redem_amt);
		m_params.setParam("subacc_time",m_stTradeBuy.Facc_time);
		m_doSaveOnly=true;
		return false;
	}

	//�������
	gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
	throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
}

/**
 * ������֧�����
* ���ɹ�,������������:return true
* ���ʧ��,�����к�������: return false
 */
bool AbstractBuySpAck::CheckBalancePay() throw (CException)
{		
	char errMsg[128]={0};
	if (m_stTradeBuy.Fpurpose != PURPOSE_BALANCE_BUY)
	{
		strncpy(m_stTradeBuy.Facc_time, m_params.getString("systime").c_str(), sizeof(m_stTradeBuy.Facc_time) - 1);
		m_acc_time=m_params.getString("systime");
		return true;
	}

	//����ͨ���֧����ͨ��������˻�����֧��	
	ST_BALANCE_ORDER fetch_data;
	memset(&fetch_data,0,sizeof(ST_BALANCE_ORDER));
	strncpy(fetch_data.Flistid,m_params["fund_trans_id"],sizeof(fetch_data.Flistid)-1);
	strncpy(fetch_data.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(fetch_data.Ftrade_id)-1);
	fetch_data.Ftype = OP_TYPE_BA_BUY;
	//  ��������ˮ�����ݴ���
	if (false == queryFundBalanceOrder(m_pFundCon, fetch_data,  false))
	{
		snprintf(errMsg, sizeof(errMsg), "����깺��������[%s]", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BA_ORDER_NOT_EXIST, errMsg, __FILE__, __LINE__);
	}
	// ���֧��֪ͨʹ�������ˮ���е�acctime
	m_params.setParam("pay_time", fetch_data.Facc_time);
	m_acc_time = fetch_data.Facc_time;
	if(m_acc_time.empty()||m_acc_time.size()!=19)
	{
		snprintf(errMsg, sizeof(errMsg), "֧���ɹ�:����깺��ʱ�䲻��ȷ[%s][%s]", fetch_data.Flistid,fetch_data.Facc_time);
		throw CException(ERR_BA_ORDER_NOT_EXIST, errMsg, __FILE__, __LINE__);
	}	
	//����깺�˿������:�����к�������
	if (m_stTradeBuy.Fstate == PURCHASE_REFUND_SUC)
	{
		need_refund = true;
		return false;
	}
	// ��������ˮ��״̬:5״̬���,6״̬����
	if(fetch_data.Fstate!=FUND_FETCH_SUBACC_OK&&fetch_data.Fstate!=FUND_FETCH_OK)
	{
		snprintf(errMsg, sizeof(errMsg), "֧���ɹ�:����깺��״̬����ȷ[%s][%d]", fetch_data.Flistid,fetch_data.Fstate);
		throw CException(ERR_BA_ORDER_NOT_EXIST, errMsg, __FILE__, __LINE__);
	}

	//����깺���֧��ʱ����15��֮ǰ����ǰʱ�����15���ҳ�����ָ��ʱ�䣬����2�����6Сʱ������Ҫ�˿�
	if ((m_acc_time.substr(11,2)<"15"
		&& m_params.getString("systime").substr(11,2) >= "15" 
		&& (payNotifyOvertime(m_params.getString("pay_time")))) 
		|| (toUnixTime(m_params["systime"])>6*3600+toUnixTime(m_params["pay_time"])) )
	{
		refund_desc = "����ɹ�����һ��ʱ��δ�깺�ɹ��˿�";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_9;
	} 
	strncpy(m_stTradeBuy.Facc_time, m_acc_time.c_str(), sizeof(m_stTradeBuy.Facc_time) - 1);
	
	return true;
}

/**
 * ֧��֪ͨ��¼���û���Ϣ
 */
void AbstractBuySpAck::RecordFundBindPay() throw (CException)
{	
	//ȫ���û��״��깺�����׼�¼��Ftrade_id ������Ϊ�����㿪��������Ϊ�գ�����˿��Ǳ��ڿ��ܵ��û��鿴
	//m_stTradeBuy.Fuid=0 ��ȫ���û��ͻ�Ϊ0������ʵ��������֣����û�����֧��(����)�����ܺ�֧�����ȵ�����m_fund_bind.Fuidд����ʵֵ��
	//��֧���Ķ����󵽣�����m_stTradeBuy.Fuid=0��m_fund_bind.Fuid!=0
	if((m_fund_bind_exist && m_fund_bind.Fuid == 0) || (0 == strcmp("", m_stTradeBuy.Ftrade_id)) || 0 == m_stTradeBuy.Fuid)
	{
		//����ʱû��uid ���޷����û����ױ��д�������֧���ɹ�ʱ�ٴ���
		m_stTradeBuy.Fuid = m_params.getInt("uid");
		try
		{
			//����ʧ�ܲ�����
			InsertTradeUserFund(m_pFundCon, &m_stTradeBuy);
		}
		catch(...)
		{
			gPtrAppLog->error("InsertTradeUserFund error once");
			try
			{
				//����ʧ�ܲ�����
				InsertTradeUserFund(m_pFundCon, &m_stTradeBuy);
			}
			catch(...)
			{
				gPtrAppLog->error("InsertTradeUserFund error two");
			}

		}
	}
}

void AbstractBuySpAck::checkSamePayCard()
{
	//�Ѿ�����Ҫ�˿����
	if(need_refund)
	{
		return;
	}

	if(checkWhitePayUser(m_params.getString("uin")))
	{
		//����������û�һ������ԭ���������˴������߼�
	}
	else if(gPtrConfig->m_AppCfg.check_same_pay_card == 0)
	{
		return ; //���ò����
	}

	if(m_stTradeBuy.Fpur_type == PURTYPE_REWARD_PROFIT || m_stTradeBuy.Fpur_type == PURTYPE_REWARD_SHARE || m_stTradeBuy.Fpur_type == PURTYPE_TRANSFER_PURCHASE)
	{
		return ;
	}

	//����ʧ�������깺�����֧��bind_serialnoΪ�շŹ�
	//����ͨ����깺Ҳ�������֧��bind_serialno Ϊ��
	if (m_params.getString("bind_serialno").empty()
		&& (true == checkInnerBalancePayForReBuy(m_pFundCon,m_params["fund_trans_id"],m_params["total_fee"],"")
		|| m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY))
	{
		return;
	}

	//����û�֧���İ����кź����ݿ��м�¼���Ƿ�һ��
	FundPayCard fund_pay_card;
	memset(&fund_pay_card, 0, sizeof(FundPayCard));
	strncpy(fund_pay_card.Fqqid, m_fund_bind.Fqqid, sizeof(fund_pay_card.Fqqid) - 1);
	strncpy(fund_pay_card.Ftrade_id,  m_fund_bind.Ftrade_id, sizeof(fund_pay_card.Ftrade_id) - 1);
	fund_pay_card.Fuid = m_fund_bind.Fuid;
	strncpy(fund_pay_card.Fcreate_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fcreate_time) - 1);
	strncpy(fund_pay_card.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fmodify_time) - 1);

	if (m_stTradeBuy.Fpay_channel == PAY_TYPE_WEB)  //����֧��ֻУ����������
	{
		if (false == queryFundPayCard(m_pFundCon,fund_pay_card, false))
		{
			need_refund = true;
			refund_desc = "��ȫ��������";
			TRACE_ERROR("checkSamePayCard fail,pay card not exist refund!");
			refund_reason = FUND_REFUND_REASON_12;
		}
		else if (m_params.getInt("bank_type") != fund_pay_card.Fbank_type)
		{
			need_refund = true;
			refund_desc = "����֧�����������밲ȫ���������Ͳ�һ��";
			TRACE_ERROR("checkSamePayCard fail,bank_type check fail  refund! input bank_type=%d,safe_bank_type=%d",
                m_params.getInt("bank_type"), fund_pay_card.Fbank_type);
			refund_reason = FUND_REFUND_REASON_13;
		}
	}
    else  //΢����q֧���鰲ȫ��
	{ 
    	if(!checkPayCard(m_pFundCon, fund_pay_card,  m_params.getString("bind_serialno"),m_bindCareInfo))
    	{
    		gPtrAppLog->warning("must use the same card payment.");
    		if(m_params.getString("bind_serialno").empty())
    		{
    			refund_desc = "�����к�Ϊ�յ����˿�";
			refund_reason = FUND_REFUND_REASON_12;
    		}
    		else
    		{
    			refund_desc = "�ǰ�ȫ��֧��";
			refund_reason = FUND_REFUND_REASON_13;
    		}

    		need_refund = true;
    	}
    }
}

void AbstractBuySpAck::checkUserTotalShare() throw (CException)
{
		//�Ѿ�����Ҫ�˿����
    if(need_refund)
    {
        return;
    }
	
    int uid = (m_fund_bind.Fuid == 0) ? m_params.getInt("uid") : m_fund_bind.Fuid;

    //����깺�ͻ���Ͷ�������޶�
    if (m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY || m_stTradeBuy.Fpurpose == PURPOSE_ACTION_BUY)
    {
        return;
    }
      

    LONG currentTotalAsset = queryUserTotalAsset(uid,m_fund_bind.Ftrade_id);
    if (true == isUserAssetOverLimit(m_fund_bind.Fasset_limit_lev,currentTotalAsset, m_params.getLong("total_fee")))
    {
        //����ʧ�������깺���޶�
        if (true == checkInnerBalancePayForReBuy(gPtrFundDB, m_params.getString("fund_trans_id"), m_params.getString("total_fee"),""))
        {
            return;
        }
        
        TRACE_ERROR("user total Asset Over Limit!");
        refund_desc = "�������ʲ��޶�";
        need_refund = true; //�û����еķݶ�,�����ݶ�������ƣ�ת�˿�
        refund_reason = FUND_REFUND_REASON_11;
    }
}

/**
 * ���Ԥ�����������
 */
void AbstractBuySpAck::checkFreezePrepayCard() throw (CException)
{
	// ���Ṻ������ż��Ԥ�����ͺ�Լ��
	if (m_stTradeBuy.Fpurpose != PURPOSE_FREEZE)
	{
		return;
	}

    strncpy(m_freeze_fund.Ffreeze_id,m_stTradeBuy.Flistid,sizeof(m_freeze_fund.Ffreeze_id)-1);
	if(!queryFundFreeze(m_pFundCon,m_freeze_fund,true))
	{
        //�쳣�����Ϊ�˲�Ӱ�������̣�����ֻ�澯
        alert(ERR_UPDATE_FREEZE_BILL, (string(m_stTradeBuy.Flistid)+" purpose=5 but payed Ok but query Freeze bill fail!").c_str());
        memset(&m_freeze_fund,0,sizeof(ST_FREEZE_FUND));
        return;
	}
	// ��������Ԥ�����̻������
    if (string(m_freeze_fund.Fspid) != gPtrConfig->m_AppCfg.wx_wfj_spid)
    {
    	return;
    }
	ST_FUND_CONTROL_INFO controlParams;	
    //����΢������������
    memset(&controlParams,0,sizeof(ST_FUND_CONTROL_INFO));
	strncpy(controlParams.Fspid,m_freeze_fund.Fspid,sizeof(controlParams.Fspid)-1);
	strncpy(controlParams.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(controlParams.Ftrade_id)-1);
	strncpy(controlParams.Fcard_no,m_freeze_fund.Fpre_card_no,sizeof(controlParams.Fcard_no)-1);
	// ֧��֪ͨ����Ԥ�������ų�ͻ�����˿�
	if(!checkWxPreCardBuy(m_pFundCon,controlParams,m_controlInfo,true))
	{
		need_refund = true;
	}
	
}

/**
 * ��鲢���㽻����,��¼��m_params��
 */
void AbstractBuySpAck::CheckPayTransDate() throw(CException)
{	
	//���㽻������
	string trade_date;
	string fund_vdate;
	getTradeDate(m_pFundCon,m_acc_time, trade_date,fund_vdate);
	if(	trade_date.empty()){
		//����������
		alert(ERR_UNFOUND_TRADE_DATE,"trade_date unfound:"+string(m_stTradeBuy.Flistid));
		gPtrAppLog->error("trade_date unfound[%s], systime[%s]", m_stTradeBuy.Flistid, m_params.getString("systime").c_str());
		throw CException(ERR_UNFOUND_TRADE_DATE, "trade_date unfound! ", __FILE__, __LINE__);
	}
	m_params.setParam("trade_date",trade_date);
	m_params.setParam("fund_vdate",fund_vdate);

}
/**
 *  ֧��֪ͨ����깺���׵�����
 */
bool AbstractBuySpAck::CheckParamsForPay() throw (CException)
{
	if(need_refund)
	{
		return true;
	}else if(PUR_REQ_FAIL == m_stTradeBuy.Fstate)
	{
		//����ΪԤ�깺ʧ�ܵģ�ֱ��ת���˿�
		refund_desc = "�깺ʧ��ת�˿�";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_10;
		return true;
	}else if(PAY_INIT != m_stTradeBuy.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	
	// �ӳ�ȷ���̻���֧�ֹ����Լ��
	if (m_stTradeBuy.Fpurpose == PURPOSE_FREEZE && m_fund_sp_config.Fbuy_confirm_type != SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		char szErrMsg[128]={0};
		snprintf(szErrMsg,sizeof(szErrMsg),"freeze purpose buy[%s] not allow use delay confirm sp[%s]",m_stTradeBuy.Flistid,m_fund_sp_config.Fspid);
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, szErrMsg, __FILE__, __LINE__);
	}
	
	// ʵʱȷ�����͵ķݶ��ʼ��Ϊ���
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM&&m_params.getLong("fund_units")==0)
	{
		m_params.setParam("fund_units",m_stTradeBuy.Ftotal_fee);
	}
	return true;
}

void AbstractBuySpAck::checkSpconfigBuyOverFull(const string& tradeDate)
{
	if(need_refund)
	{
		return;
	}

	//δ��������ֱ�ӷ���
	if(m_fund_sp_config.Fbuyfee_tday_limit <= 0&&m_fund_sp_config.Fscope_upper_limit <= 0)
	{
		return;
	}
	if(tradeDate=="")
	{
		//��Ӧ�÷���
		throw EXCEPTION(ERR_BAD_PARAM, "checkSpconfigBuyOverFull tradeDate is error"); 
	}

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));

	strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, false))
	{
		//��Ӧ�÷���
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}

	bool overfull=false;
	// ��������ͳ������
	string strStatDay=( SPCONFIG_STAT_DDAY & fundSpConfig.Fstat_flag )? nowdate(m_params.getString("systime").c_str()):tradeDate;
	

	//�����޶����Ѿ���������ô���Ϊ����
	if (fundSpConfig.Fbuyfee_tday_limit>0) 
	{
		double total_redeem_tday=0;
		if((fundSpConfig.Fstat_flag&SPCONFIG_STAT_NET)&&fundSpConfig.Fstat_redeem_tdate>=strStatDay) 
		{   //���㾻�깺
			total_redeem_tday=fundSpConfig.Ftotal_redeem_tday*calRedeemRate();
		}
		if(m_stTradeBuy.Ftotal_fee+fundSpConfig.Ftotal_buyfee_tday-total_redeem_tday> fundSpConfig.Fbuyfee_tday_limit)
		{
	    	overfull = true;
		}
	}

	 //�����޶����Ѿ���������ô���Ϊ����
	if (fundSpConfig.Fscope_upper_limit > 0) 
	{
	    if(fundSpConfig.Fscope + m_stTradeBuy.Ftotal_fee> fundSpConfig.Fscope_upper_limit)
	    {
		 overfull = true;
	    }
	}

	//�ж�Ϊ�Ѿ����ޣ���Ҫ���ۼ��޶�����˿�
	if (overfull==true)
	{
	 	updateSpconfigTotalBuy(strStatDay,true);
	}
}

void AbstractBuySpAck::updateSpconfigTotalBuy(const string& strStatDay,bool RefunAllowed)
{
	if(need_refund)
	{
		return;
	}

	// δ��������ֱ�ӷ���
	// 0 ��ʾ����Ҫ��¼
	// -1��ʾ��Ҫ��¼������Ҫ����
	if(m_fund_sp_config.Fbuyfee_tday_limit == 0&&m_fund_sp_config.Fscope_upper_limit == 0)
	{
		return;
	}

    //����Ѿ��ۼӹ����Ͳ������ۼ�ֱ�ӷ���
    if (m_bBuyTotalAdded == true)
    {
        return;
    }

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));

	strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//��Ӧ�÷���
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}
	double redeemRate = calRedeemRate();
	checkAndUpdateFundScope(fundSpConfig, m_stTradeBuy.Ftotal_fee,strStatDay,(RefunAllowed?(&need_refund):NULL),(RefunAllowed?(&refund_desc):NULL)
                                                    ,(fundSpConfig.Fclose_flag!= CLOSE_FLAG_NORMAL && fundSpConfig.Fpurpose==101),redeemRate);

    m_bBuyTotalAdded = true;

}
/**
  * �����깺�������ʹ����ؽ�����
  */
double AbstractBuySpAck::calRedeemRate()
{
	return 1;
}

/**
 * ֧��֪ͨ�������
 */
bool AbstractBuySpAck::CheckFundTradeForPay() throw (CException)
{
	if(!CheckPayRepeat())
	{
		// ���벻���к�������
		return false;
	}
	if(!CheckBalancePay())
	{
		// ����˿���к�������
		return false;
	}
	// ������һ����
	CheckParamsForPay();
	
	// ��齻����
	CheckPayTransDate();
	
	// ��¼���û�����Ϣ
	RecordFundBindPay();

	//����û�֧���İ����кź����ݿ��м�¼���Ƿ�һ��
	checkSamePayCard();

	//����û����еķݶ�,�����ݶ��������ת�˿�
	checkUserTotalShare();

	// ���Ԥ�����������
	checkFreezePrepayCard();

	// ����޶�,����ڽ����ռ��֮��
	checkSpconfigBuyOverFull(m_params.getString("trade_date"));
	return true;
	
}
// ��װ֧��������׵�
void AbstractBuySpAck::BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	strncpy(stRecord.Facc_time, m_acc_time.c_str(), sizeof(stRecord.Facc_time) - 1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);
	strncpy(stRecord.Fsub_trans_id, m_stTradeBuy.Flistid, sizeof(stRecord.Fsub_trans_id) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;

	//����Ҫ�˿��ʱ��֧���ص���uid �����Ǻͷ��������ʱ���uid��һ�£��紮cookie������
	stRecord.Fuid = (need_refund && m_stTradeBuy.Fuid !=0 ) ? m_stTradeBuy.Fuid : m_params.getInt("uid");

	strncpy(stRecord.Ftrade_date,m_params.getString("trade_date").c_str(), sizeof(stRecord.Ftrade_date) - 1);//������
	strncpy(stRecord.Ffund_vdate,m_params.getString("fund_vdate").c_str(), sizeof(stRecord.Ffund_vdate) - 1);//����ֵ����,�ñ��깺�״β������������
	strncpy(stRecord.Fmemo, need_refund ? refund_desc.c_str() : m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);//�˿�ļ�¼�˿�ԭ��
    if (m_stTradeBuy.Fpay_channel == PAY_TYPE_WEB) //����֧��ȡbiz_attach�е�bank_type
        stRecord.Fbank_type=m_params.getInt("bank_type");
    else
        stRecord.Fbank_type=m_params.getInt("pay_bank_type");
	strncpy(stRecord.Fcard_no, m_params["pay_card_tail"], sizeof(stRecord.Fcard_no) - 1);
	//����trade_id,���½��׼�¼ʱ��Ҫʹ��
	strncpy(stRecord.Ftrade_id, m_fund_bind.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
	// ֧��֪ͨ������������:stRecord.Fcharge_fee= m_params.getLong("charge_fee");

	//ǰ�������ж���Ҫ�˿�ģ��Ǽǽ��׵�״̬Ϊת���˿�
	//�ӳٵ��˴�������Ϊ��ֹ���׵��ѳɹ���ȴ���û����˿�
	if(need_refund)
	{
		stRecord.Fstate = PURCHASE_APPLY_REFUND;
		stRecord.Frefund_reason = refund_reason;
		stRecord.Frefund_type = REFUND_CARD;
		if (m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY)
		{
			stRecord.Fstate = PURCHASE_REFUND_SUC;
			stRecord.Frefund_type = REFUND_BALANCE;
		}
		// �����˿�ز���Ϊ���������
		int result = ERR_BUY_RECORD_NEED_REFUND;
		m_params.setParam("result",result);
		return;
	}
	
	if(m_fund_bind.Fuid == 0)
	{
		//����ʱû��uid ���޷����û����ױ��д�������֧���ɹ�ʱ�ٴ���
		m_stTradeBuy.Fuid = m_params.getInt("uid");

		stRecord.Fuid = m_params.getInt("uid");

		//���»����˻���������д��uid
		ST_FUND_BIND fundBind;
		memset(&fundBind, 0, sizeof(ST_FUND_BIND));
		strncpy(fundBind.Ftrade_id,m_fund_bind.Ftrade_id, sizeof(fundBind.Ftrade_id) - 1);
		fundBind.Fuid = m_params.getInt("uid");
		UpdateFundBind(m_pFundCon, fundBind, m_fund_bind, m_params.getString("systime"));

		// �����û�fundBind ��CKV
		memset(&fundBind, 0, sizeof(ST_FUND_BIND));
		strncpy(fundBind.Fqqid,m_fund_bind.Fqqid, sizeof(fundBind.Fqqid) - 1);
		setFundBindToKV(m_pFundCon, fundBind, true);

	}

	stRecord.Fstate = PAY_OK;
	// ֧��ȷ�Ϸݶ�,ֱ�Ӹ��·ݶ�
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		stRecord.Freal_redem_amt= m_params.getLong("fund_units");
		m_params.setParam("subacc_units",stRecord.Freal_redem_amt);
		m_params.setParam("subacc_time",m_acc_time);
	}
	
	
}

// ���º�Լ������״̬
void AbstractBuySpAck::updateFundFreezeBill(const string& fund_vdate ) throw(CException)
{
  // û�鵽���ᵥ
	if(m_freeze_fund.Ffreeze_id[0]==0)
	{
        //�쳣�����Ϊ�˲�Ӱ�������̣�����ֻ�澯
        alert(ERR_UPDATE_FREEZE_BILL, (string(m_stTradeBuy.Flistid)+" purpose=5 but payed Ok but query Freeze bill fail!").c_str());
        return;
	}
    ST_FREEZE_FUND freezeData;
    memset(&freezeData,0,sizeof(ST_FREEZE_FUND));
    queryFundFreeze(m_pFundCon,freezeData,false);
    
    ST_FREEZE_FUND freezeDataSet;
    memset(&freezeDataSet,0,sizeof(ST_FREEZE_FUND));
    freezeDataSet.Fstate = FUND_FREEZE_PAY_BUYED;
    strncpy(freezeDataSet.Ffreeze_id,m_stTradeBuy.Flistid,sizeof(freezeDataSet.Ffreeze_id)-1);
    strncpy(freezeDataSet.Fmodify_time,m_params["systime"],sizeof(freezeDataSet.Fmodify_time)-1);
    if (0 == updateFundFreeze(m_pFundCon,freezeDataSet,freezeData))
    {
        //�쳣���,��Ӧ�ó���
        return;
    }
	// ��������Ԥ�����̻�������
    if (string(m_freeze_fund.Fspid) != gPtrConfig->m_AppCfg.wx_wfj_spid)
    {
    	return;
    }
	
    //΢��������Ԥ�����̻���ʱ����
    if(m_controlInfo.Ftrade_id[0]==0)
    {
        ST_FUND_CONTROL_INFO controldata;
        memset(&controldata,0,sizeof(ST_FUND_CONTROL_INFO));
        strncpy(controldata.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(controldata.Ftrade_id));
        controldata.Ftype=1;
        strncpy(controldata.Fuin,m_fund_bind.Fqqid,sizeof(controldata.Fuin));
        strncpy(controldata.Ffund_spid,m_stTradeBuy.Fspid,sizeof(controldata.Ffund_spid));
        controldata.Fcur_type=m_stTradeBuy.Fcur_type;
        strncpy(controldata.Fspid,m_freeze_fund.Fspid,sizeof(controldata.Fspid));
        strncpy(controldata.Ffirst_profit_day,fund_vdate.c_str(),sizeof(controldata.Ffirst_profit_day));
        strncpy(controldata.Fcreate_time,m_params["systime"],sizeof(controldata.Fcreate_time));
        strncpy(controldata.Fmodify_time,m_params["systime"],sizeof(controldata.Fmodify_time));
        strncpy(controldata.Fcard_no,m_freeze_fund.Fpre_card_no,sizeof(controldata.Fcard_no));
        strncpy(controldata.Fcard_partner,m_freeze_fund.Fpre_card_partner,sizeof(controldata.Fcard_partner));
        controldata.Ftotal_fee=m_stTradeBuy.Ftotal_fee;
        controldata.Flstate=1;
		insertFundControlInfo(m_pFundCon,controldata);
    }else{
    	addFundControlBalance(m_pFundCon,m_stTradeBuy.Ftotal_fee,m_params.getString("systime"),m_fund_bind.Ftrade_id);
    }
}

// ����֧���������
void AbstractBuySpAck::RecordFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	//���׼�¼û�еȵ�Ԥ�깺������һ��ʱ��ת�˿�ģ��������û����ױ�����ֹ�൥�������⵼��û��trade_idҲû�м�¼���û��ֱ�����
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"), (0 == strcmp("", m_stTradeBuy.Ftrade_id)) ? false : true);

	// �˿�������˻�
	if(need_refund)
	{
		return;
	}

	//���ڹ����Լ�����깺��Ҫ�޸Ķ��ᵥ״̬Ϊ֧���ɹ�
	if (m_stTradeBuy.Fpurpose == PURPOSE_FREEZE)
	{
	    updateFundFreezeBill(m_params.getString("fund_vdate"));
	}
}
// ͬ��֧��������ݵ�ȫ�ֱ���
void AbstractBuySpAck::SyncFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	// ��װ�仯�Ĳ���
	m_stTradeBuy.Fuid = m_params.getInt("uid");
	m_stTradeBuy.Fstate= stRecord.Fstate;
	strncpy(m_stTradeBuy.Fcft_trans_id, stRecord.Fcft_trans_id, sizeof(m_stTradeBuy.Fcft_trans_id) - 1);
	strncpy(m_stTradeBuy.Fsub_trans_id, stRecord.Fsub_trans_id, sizeof(m_stTradeBuy.Fsub_trans_id) - 1);
	strncpy(m_stTradeBuy.Ftrade_date, stRecord.Ftrade_date, sizeof(m_stTradeBuy.Ftrade_date) - 1);
	strncpy(m_stTradeBuy.Facc_time, stRecord.Facc_time, sizeof(m_stTradeBuy.Facc_time) - 1);
	strncpy(m_stTradeBuy.Ffund_vdate, stRecord.Ffund_vdate, sizeof(m_stTradeBuy.Ffund_vdate) - 1);
	strncpy(m_stTradeBuy.Fmemo, stRecord.Fmemo, sizeof(m_stTradeBuy.Fmemo) - 1);
	m_stTradeBuy.Fclose_listid = stRecord.Fclose_listid;
}

/**
* ֧���ɹ�������,�ȸ��½��׵�Ϊ����ɹ��������ӻ����˻����������ʧ��ͨ��������ɡ�
* ֧��ʧ�ܽ��������
*/
void AbstractBuySpAck::UpdateFundTradeForPay() throw (CException)
{
	if(!CheckFundTradeForPay())
	{	// ��鲻ͨ��������������
		return;
	}

	// �������ױ������ҵ������������
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	BuildFundTradeForPay(stRecord);

	// ��������
	RecordFundTradeForPay(stRecord);

	// ���ݸ��³ɹ�ͬ�������ȫ�ֱ���
	SyncFundTradeForPay(stRecord);

	// ����MQ��Ϣ
	sendFundBuy2MqMsg(m_stTradeBuy);

   /*�������޶���޶�ϸ���Ƴ��������Գ������˿
       �������޶�������������ñ����Ա��������������*/
   updateSpconfigTotalBuy(stRecord.Ftrade_date,false);
	
}


// ֧��֪ͨ�̻�������
// ������
void AbstractBuySpAck::CheckFundTradeForPayAck() throw (CException)
{
	// �ж��Ƿ�״̬�Ѿ�֪ͨȷ�ϳɹ�
	bool isStatePayAck = false;
	if(PAY_ACK_SUC==m_stTradeBuy.Fstate||PURCHASE_SUC==m_stTradeBuy.Fstate)
	{
		isStatePayAck=true;
	}

	//����: �ɹ��Ҳ�Ϊ��ʱ״̬
	if(isStatePayAck&& TRADE_RECORD_TIMEOUT != m_stTradeBuy.Fspe_tag)
	{
		//�������
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	//����: �ɹ��Ҹ��³�ʱ
	if(isStatePayAck&&INF_PAY_SP_INFO_TIMEOUT == m_optype)
	{
		//�������
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	//  ���Ա��״̬:1, ֧��ȷ��;2,�ɹ����ҳ�ʱ
	if(!(PAY_OK == m_stTradeBuy.Fstate || 
		(isStatePayAck && TRADE_RECORD_TIMEOUT == m_stTradeBuy.Fspe_tag)))
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

}

void AbstractBuySpAck::BuildFundTradeForPayAck(ST_TRADE_FUND& stRecord) throw (CException)
{
	if(INF_PAY_SP_INFO_TIMEOUT == m_optype)
	{	
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���
	}
	else
	{
		stRecord.Fspe_tag = 0;//��ʱ�����ɹ���Ҫ����ʱ״̬�޸ģ������²�ͣ����
	}
	// ����֧��֪ͨ���
	if(m_fund_sp_config.Fbuy_confirm_type == SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		// ʵʱȷ�Ϸݶ����Ϊ����״̬
		stRecord.Fstate = PURCHASE_SUC;
	}else{
		// �ӳ�ȷ�Ϸݶ����Ϊ֧��֪ͨ״̬
		stRecord.Fstate = PAY_ACK_SUC;
	}
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

	// ֧��֪ͨȷ�ϲ�����������:stRecord.Fcharge_fee= m_params.getLong("charge_fee");
}

/**
* ֧��֪ͨ�̻�������
* ������˾���깺ȷ����Ϊ��Ԥ�깺������Ϊ��һ�����깺ȷ�ϲ��ܳ���ʧ�ܣ���ͬ�ϱ�֤��
* �깺ȷ�ϳ�ʱ���ɹ���������ʱ��ǣ��Ȳ�����
*/
void AbstractBuySpAck::UpdateFundTradeForPayAck() throw (CException)
{
	// ���֧��֪ͨ����
	CheckFundTradeForPayAck();

	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	BuildFundTradeForPayAck(stRecord);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
	
	//�깺ȷ�ϳɹ��󣬸�ȯ�����첽���󣬾�����ȯ��ֵ����
	UpdateFundTradeForCoupon();
}

//��ʱ�̻�������
// ������
bool AbstractBuySpAck::CheckFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// �ж��Ƿ�״̬�Ѿ��깺ȷ�ϳɹ�
	if(PURCHASE_SUC==m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==0)
	{
		// ����ɹ��Ĳ�������������룬ֻ�������˻�,�����к�������
		if(ERR_TYPE_MSG)
		{
			// �������˻�ȷ�Ϸݶ�,����ʹ��modify_time����
			// (ҵ�����¼���˻�ʱ��������ʵ��)
			m_params.setParam("subacc_units",m_stTradeBuy.Freal_redem_amt);
			m_params.setParam("subacc_time",m_stTradeBuy.Fmodify_time);
			m_doSaveOnly=true;
			return false;
		}
		//�������
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	//  ���Ա��״̬: 
	//  1, ֧��֪ͨ�ɹ�
	//  2, �깺ȷ�ϳɹ�spe_tag��Ϊ0
	if(PAY_ACK_SUC != m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==0)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	return true;
}

void AbstractBuySpAck::BuildFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{

	stRecord.Fstate = PURCHASE_SUC;
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);		
	if(strcmp(m_stTradeBuy.Fcoding,"")==0)
	{
    	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	}
	stRecord.Fcharge_fee= m_params.getLong("charge_fee");
	stRecord.Freal_redem_amt= m_params.getLong("fund_units");
	//�������˻�ȷ�Ϸݶ�,Ĭ��ʹ�õ�ǰʱ���������˻�,��¼��modify_time
	m_params.setParam("subacc_units",stRecord.Freal_redem_amt);
	m_params.setParam("subacc_time",m_params.getString("systime"));
}

void AbstractBuySpAck::RecordFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}


/**
* �̻�ȷ�ϳɹ�������
* ״̬��12��3
* �������˻�
*/
void AbstractBuySpAck::UpdateFundTradeForSucAck() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	// ����깺ȷ�ϲ���
	if(!CheckFundTradeForSucAck(stRecord))
	{
		return;
	}

	// ��װ�깺ȷ�Ͻ��
	BuildFundTradeForSucAck(stRecord);

	// �����깺ȷ�Ͻ��
	RecordFundTradeForSucAck(stRecord);
}


/**
* ���˻���ֵ
*/
void AbstractBuySpAck::doSave() throw(CException)
{
	// ������˻��ݶ�
	LONG subaccUnits = m_params.getLong("subacc_units");
	gPtrAppLog->debug("doSave, listid[%s],subacc_units[%ld]  ", m_params.getString("sub_trans_id").c_str(),subaccUnits);
	if(subaccUnits<=0)
	{
		return;
	}
	bool needCallErr=true;
	string subaccTime = m_params.getString("subacc_time");
	if(subaccTime.empty())
	{
		char szErrMsg[256] = {0};
		snprintf(szErrMsg, sizeof(szErrMsg), "[%s][%s]�깺�����˻�ʱ��δ��ֵ,ʹ�õ�ǰʱ��,ʧ�ܲ��ײ��,������ѭ��",m_stTradeBuy.Flistid,subaccTime.c_str());
		alert(ERR_SUBACC_TIME_EMPTY, szErrMsg);
		subaccTime=m_params.getString("systime");
		needCallErr = false;
	}

	try
	{
		if (ERR_SUBACC_NOT_EXIST == SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("sub_trans_id"), subaccUnits,"�����깺", subaccTime.c_str(), 1))
             {
        			//�����˻��ӳٵ�֧���ɹ������ٻʱ�깺�����ѹ��
        			createSubaccUser(m_request, m_params.getString("spid"), m_params.getString("uin"), m_params.getString("client_ip"));

                    SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("sub_trans_id"), subaccUnits,"�����깺", subaccTime.c_str(), 1);
             }      

	}
	catch(CException& e)
	{
		if(!needCallErr) // �������ֱ��throw �쳣
		{
			throw;
		}
		//���˻���Ӧ��ʧ�ܣ����м�Ǯʧ�ܵĶ��������

		//���֧���ص�����һ�����ʱ�����ϣ����˻������쳣�����澯
		//ʹ�ö����ɹ�ʱ��Ϊ����������֧���ص����ˣ�ֱ�Ӹ澯��û�в���10����
		if(payNotifyOvertime(subaccTime.c_str()))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal[%s][%s]�깺֧���ص�����10�������˻���δ�ɹ�",m_stTradeBuy.Flistid,m_params.getString("sub_trans_id").c_str());
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			if(ERR_TYPE_MSG)
			{
				return; //�������һ��ʱ��δ�ɹ��ķ��͸澯���ٲ�����������ѭ������ѩ��
			}

		}

		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// ����Ϣ�����
		callErrorRpc(m_request, gPtrSysLog);

		if(ERR_TYPE_MSG)
		{
			throw;//���Բ��������ֱ���׳��쳣����ֹ�������ִ��
		}

	}

}

void AbstractBuySpAck::updateCkvs()
{
	if(m_doSaveOnly)
	{
		//�������ֻ�����˻�,����Ҫ��ckv		
		return;
	}
	
	// �ݶ�ȷ��,�������˻��������˻����ʲ�
	if(m_params.getLong("subacc_units")>0)
	{
		//�����û��ֲ�
		updateUserAcc(m_stTradeBuy);

	// �ݶ�δȷ��,֧���ɹ������ֲ�
	}else if(INF_PAY_OK == m_optype &&  false == need_refund)	
	{
		//�����û��ֲ���ȷ���ʲ�
		updateUserAcc(m_stTradeBuy);	
	}

}

void AbstractBuySpAck::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	char szMsg[MAX_MSG_LEN + 1] = {0};

	// ��װ�ؼ�����
	CUrlAnalyze::setParam(szMsg, "Flistid", fundTradeBuy.Flistid, true);
	CUrlAnalyze::setParam(szMsg, "Fspid", fundTradeBuy.Fspid);
	CUrlAnalyze::setParam(szMsg, "Fuin", m_params.getString("uin").c_str());
	CUrlAnalyze::setParam(szMsg, "Fcoding", fundTradeBuy.Fcoding);
	CUrlAnalyze::setParam(szMsg, "Ftrade_id", fundTradeBuy.Ftrade_id);
	CUrlAnalyze::setParam(szMsg, "Fuid", fundTradeBuy.Fuid);
	CUrlAnalyze::setParam(szMsg, "Ffund_code", fundTradeBuy.Ffund_code);
	CUrlAnalyze::setParam(szMsg, "Fpur_type", fundTradeBuy.Fpur_type);
	CUrlAnalyze::setParam(szMsg, "Ftotal_fee", fundTradeBuy.Ftotal_fee);
	CUrlAnalyze::setParam(szMsg, "Fstate", fundTradeBuy.Fstate);
	CUrlAnalyze::setParam(szMsg, "Ftrade_date", fundTradeBuy.Ftrade_date);
	CUrlAnalyze::setParam(szMsg, "Ffund_vdate", fundTradeBuy.Ffund_vdate);
	CUrlAnalyze::setParam(szMsg, "Fcreate_time", fundTradeBuy.Fcreate_time);
	CUrlAnalyze::setParam(szMsg, "Fmodify_time", fundTradeBuy.Fmodify_time);
	CUrlAnalyze::setParam(szMsg, "Fcft_trans_id", fundTradeBuy.Fcft_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_charge_ctrl_id", fundTradeBuy.Fcft_charge_ctrl_id);
	CUrlAnalyze::setParam(szMsg, "Fsp_fetch_id", fundTradeBuy.Fsp_fetch_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_bank_billno", fundTradeBuy.Fcft_bank_billno);
	CUrlAnalyze::setParam(szMsg, "Fsub_trans_id", fundTradeBuy.Fsub_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcur_type", fundTradeBuy.Fcur_type);
	CUrlAnalyze::setParam(szMsg, "Fpurpose", fundTradeBuy.Fpurpose);
	CUrlAnalyze::setParam(szMsg, "Facc_time", fundTradeBuy.Facc_time);
	CUrlAnalyze::setParam(szMsg, "Fchannel_id", fundTradeBuy.Fchannel_id);
	CUrlAnalyze::setParam(szMsg, "Fmemo", fundTradeBuy.Fmemo);

	sendMsg2Mq(szMsg);
}

/**
  * ����������
  */
void AbstractBuySpAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
		CUrlAnalyze::setParam(rqst->odata, "result", m_params.getInt("result"), true);
		CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
		CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
		CUrlAnalyze::setParam(rqst->odata, "acc_time",m_stTradeBuy.Facc_time);
		CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
		CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
		CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
		CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
		CUrlAnalyze::setParam(rqst->odata, "sp_billno", m_stTradeBuy.Fcoding);
		CUrlAnalyze::setParam(rqst->odata, "fund_code", m_stTradeBuy.Ffund_code);
		CUrlAnalyze::setParam(rqst->odata, "state", m_stTradeBuy.Fstate);
		CUrlAnalyze::setParam(rqst->odata, "agree_risk", getAgreeRiskType(m_fund_sp_config,m_fund_bind));

		packBizReturnMsg(rqst);
		
    rqst->olen = strlen(rqst->odata);
    return;
}

void AbstractBuySpAck::parseBizInputMsgComm(char* szMsg) throw (CException)
{
    m_params.readStrParam(szMsg,"biz_attach",0,MAX_PARAM_LEN);
	const char* bizAttach = m_params.getString("biz_attach").c_str();
	if(strcmp(bizAttach,"")==0)
	{
		return;
	}
    
	m_params.readIntParam(bizAttach,"bank_type",0,MAX_INTEGER);

}