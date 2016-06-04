/**
  * FileName: fund_refund_service.h
  * Version :1.0
  * Date: 2015-3-3
  * Description: �������˿�ӿ�
  */


#include "fund_commfunc.h"
#include "fund_refund_service.h"

extern CftLog* gPtrSysLog;

FundRefund::FundRefund(CMySQL* mysql)
{
	m_pFundCon = mysql;

	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fundSpConfig, 0, sizeof(FundSpConfig));

}

/**
* service step 1: �����������
*/
void FundRefund::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szMsg[MAX_MSG_LEN] = {0};
	char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// Ҫ�����������ݣ��ײ��ʹ��
	m_request = rqst;

	// ����ԭʼ��Ϣ
	getDecodeMsg(rqst, szMsg, szSpId);
	m_spid = szSpId;

	TRACE_DEBUG("[fund_refund_service] receives: %s", szMsg);


	// ��ȡ����
	m_params.readStrParam(szMsg, "trade_id", 0, 32);
	m_params.readStrParam(szMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readIntParam(szMsg,"op_type", 1, 2); // 1�����˿� 2�˿����
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "refund_type", 1, 3);//�˿�����1 �˿���п�;2 �˿���ͨ���;3 �˿�Ƹ�ͨ
	m_params.readStrParam(szMsg, "desc", 0, 128);
	m_params.readStrParam(szMsg, "watch_word", 0, 32);
	m_params.readStrParam(szMsg, "client_ip", 0, 16);
	m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token
	m_params.readIntParam(szMsg, "refund_reason", 1, 22);
	m_params.readStrParam(szMsg, "cft_trans_id", 0, 32);

	GetTimeNow(szTimeNow);
	m_params.setParam("systime", szTimeNow);


}

/*
* ���ɻ���ע����token
*/
string FundRefund::GenFundToken()
{
	stringstream ss;
	char buff[128] = {0};

	// ����uid|fund_trans_id|spid|op_type|sp_billno|total_fee|key
	// ��������ԭ��
	ss << m_params["trade_id"] << "|" ;
	ss << m_params["spid"] << "|" ;
	ss << m_params["op_type"] << "|";
	ss << m_params["fund_trans_id"] << "|" ;
	ss << m_params["total_fee"] << "|" ;
	ss << gPtrConfig->m_AppCfg.refund_service_key;

	getMd5(ss.str().c_str(), ss.str().size(), buff);

	return buff;
}

/*
* ����token
*/
void FundRefund::CheckToken() throw (CException)
{
	// ����token
	string token = GenFundToken();

	if (StrUpper(m_params.getString("token")) != StrUpper(token))
	{   
		TRACE_DEBUG("fund authen token check failed, input=%s,real=%s", 
			m_params.getString("token").c_str(),token.c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
	}   
}


/**
* ����������ȡ�ڲ�����
*/
void FundRefund::CheckParams() throw (CException)
{
	// ��֤token
	CheckToken();
}


/**
* ִ���깺����
*/
void FundRefund::excute() throw (CException)
{
	try
	{
		CheckParams();

		/* �������� */
		m_pFundCon->Begin();
		
		/* ckv�����ŵ�����֮�������ύ*/
		gCkvSvrOperator->beginCkvtrans();

		/* �������˻���¼*/
		CheckFundBind();

		/* ��ѯ�����׼�¼ */
		CheckFundTrade();

		/* ��ѯ����˾����*/
		CheckSpConfig();

		/* �˿��߼�*/
		UpdateTradeRefund();

		/* �ύ���� */
		m_pFundCon->Commit();

		updateCkvs();
		
		/* ���¸���ckv ,��������֮���Ǳ�������ع�ȴд��ckv������*/
        gCkvSvrOperator->commitCkvtrans();
	}
	catch (CException& e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		//�ع�dbǰ�Ȼع�����ckv
		gCkvSvrOperator->rollBackCkvtrans();

		m_pFundCon->Rollback();

		if ((ERR_REPEAT_ENTRY != (unsigned)e.error()))
		{
			throw;
		}
	}
}
/**
  * ����CKV
  */
void FundRefund::updateCkvs()
{
	//ָ���ͻ���:��¼δȷ�Ϸݶ�
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		setFundUnconfirm(m_pFundCon, m_params.getString("trade_id"));
	}

	setFundUnfinishTransCKV(m_pFundCon, m_params.getString("trade_id"));
}


/*
 * ��ѯ�����˻��Ƿ����
 */
void FundRefund::CheckFundBind() throw (CException)
{
	if(m_params.getInt("refund_reason") == FUND_REFUND_REASON_20 || 
		m_params.getInt("refund_reason") == FUND_REFUND_REASON_7 ||
		m_params.getInt("refund_reason") == FUND_REFUND_REASON_6)
	{
		return;	
	}
	
	bool bind_exist;
	//�������û��״��깺��Ȩ�ɹ���֧��ʧ��(��Ȳ��㣬����������)���û�������Ϣ����û��uid
	bind_exist = QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &m_fund_bind, false);
	
	if(!bind_exist)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }

	// ��¼���ڣ�������¼�е�uin
    m_params.setParam("uin", m_fund_bind.Fqqid);
    m_params.setParam("uid", m_fund_bind.Fuid);

}

/**
* �������׼�¼�Ƿ��Ѿ�����,��鶩��ǰ��״̬
*/
void FundRefund::CheckFundTrade() throw (CException)
{
	// û�й����¼������
	if(!QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_BUY, &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}


	// ����״̬��Ч������
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund buy record, lstate is invalid. listid[%s], uid[%d] ", m_stTradeBuy.Flistid, m_stTradeBuy.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy record, lstate is invalid. ", __FILE__, __LINE__);
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

	//���ͨ����ĵ��������˵����ͨ���
	if (m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY  )
	{
		if (REFUND_BALANCE != m_params.getInt("refund_type"))
		{
			gPtrAppLog->error("fund purpose=%d. invalid for refundtype [%d]"
                                        , m_stTradeBuy.Fpurpose,m_params.getInt("refund_type"));
               throw CException(ERR_REFUND_TYPE, " refund type is invaid. ", __FILE__, __LINE__);      
		}
		
	}
	
       // У��ؼ�����
       if(m_params.getInt("refund_reason") != FUND_REFUND_REASON_20 &&
	   	m_params.getInt("refund_reason") != FUND_REFUND_REASON_7 &&
	   	m_params.getInt("refund_reason") != FUND_REFUND_REASON_6)
       {
		if(m_stTradeBuy.Ftrade_id!= m_params.getString("trade_id"))
		{
			gPtrAppLog->error("fund buy, trade_id is different! trade_id in db[%s], trade_id input[%s] ", 
				m_stTradeBuy.Ftrade_id, m_params.getString("trade_id").c_str());
			throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, trade_id is different!", __FILE__, __LINE__);
		}
	
		// У��ؼ�����
		if(m_stTradeBuy.Fuid!= m_params.getInt("uid"))
		{
			gPtrAppLog->error("fund buy, uid is different! uid in db[%d], uid input[%d] ", 
				m_stTradeBuy.Fuid, m_params.getInt("uid"));
			throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, uid is different!", __FILE__, __LINE__);
		}
       }
}

void FundRefund::CheckSpConfig()
{
    strncpy(m_fundSpConfig.Fspid,m_params.getString("spid").c_str(), sizeof(m_fundSpConfig.Fspid) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fundSpConfig,false);
}

void FundRefund::UpdateTradeRefund()
{
    switch (m_params.getInt("op_type"))
    {
    	case OP_TYPE_REFUND_REQ:
		UpdateTradeRefundForReq();
		break;

	case OP_TYPE_REFUND_ACK:
		UpdateTradeRefundForAck();
		break;

	default:
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;
		
    }
}


void FundRefund::UpdateTradeRefundForReq()
{
    //У�齻�׵�״̬
    CheckTradeStateForReq();
	
    //У���û�δȷ�Ϸݶ�
    CheckFundUnconfirm();
	
    //�����û�δȷ�Ϸݶ�
    UpdateFundUnconfirm();
	
    //���½��׵�״̬
    UpdateTradeStateForReq();
}

void FundRefund::UpdateTradeRefundForAck()
{
   //У�齻�׵�״̬
    CheckTradeStateForAck();
   
   //���½��׵�״̬	
    UpdateTradeStateForAck();
}

void FundRefund::CheckTradeStateForReq()
{
    if (m_stTradeBuy.Fstate == PURCHASE_APPLY_REFUND ||m_stTradeBuy.Fstate == PURCHASE_REFUND_SUC) 
	{
		 gPtrAppLog->error("fund buy state=%d. repeat entry for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
                throw CException(ERR_REPEAT_ENTRY, "buy list is already refund. ", __FILE__, __LINE__);

	}
	//��8״̬���˿�ӿڣ��˿�ԭ��Ҫ��20����
	if(m_params.getInt("refund_reason") < FUND_REFUND_REASON_20)
	{
		throw CException(ERR_REFUND_REASON, "refund reason invalid. ", __FILE__, __LINE__);
	}
	else if(m_params.getInt("refund_reason") == FUND_REFUND_REASON_20)
	{
		CHECK_PARAM_EMPTY("cft_trans_id");
    		if ((m_stTradeBuy.Fstate != CREATE_INIT && m_stTradeBuy.Fstate != PAY_INIT) || m_params.getInt("refund_type") != REFUND_CARD) 
		{
			 gPtrAppLog->error("fund buy state=%d,type=%d invalid for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_params.getInt("refund_type"),m_stTradeBuy.Flistid);
               	throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);
		}
	}
	else
	{
		if (m_stTradeBuy.Fstate != PAY_OK && m_stTradeBuy.Fstate !=PAY_ACK_SUC ) 
		{
			 gPtrAppLog->error("fund buy state=%d. invalid for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
               	throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);      

		}
	}
}

void FundRefund::CheckTradeStateForAck()
{
    if (m_stTradeBuy.Fstate == PURCHASE_REFUND_SUC ) 
	{
		 gPtrAppLog->error("fund buy state=%d. repeat entry for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
                throw CException(ERR_REPEAT_ENTRY, "buy list is already refund. ", __FILE__, __LINE__);

	}
    if (m_stTradeBuy.Fstate != PURCHASE_APPLY_REFUND ) 
	{
		 gPtrAppLog->error("fund buy state=%d. invalid for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
               throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);      

	}
}
void FundRefund::UpdateTradeStateForReq()
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fstate = PURCHASE_APPLY_REFUND;
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	stRecord.Frefund_type=m_params.getInt("refund_type");
	stRecord.Frefund_reason=m_params.getInt("refund_reason");

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
* ���½��׵�״̬
*/
void FundRefund::UpdateTradeStateForAck()
{
       ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fstate = PURCHASE_REFUND_SUC;
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

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
* ��ѯ�û��Ƿ���δȷ�Ϸݶ�
*/
void FundRefund::CheckFundUnconfirm()
{
	if(m_fundSpConfig.Fbuy_confirm_type == 0)
	{
		return;
	}
	
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
	    strncpy(m_fundUnconfirm.Ftrade_id,m_stTradeBuy.Ftrade_id, sizeof(m_fundUnconfirm.Ftrade_id) - 1);
	    strncpy(m_fundUnconfirm.Fspid,m_stTradeBuy.Fspid, sizeof(m_fundUnconfirm.Fspid) - 1);
	    strncpy(m_fundUnconfirm.Ftrade_date,m_stTradeBuy.Ftrade_date, sizeof(m_fundUnconfirm.Ftrade_date) - 1);
	    if(!queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true))
	    {
	        gPtrAppLog->error("unconfirm fund not exist.");
	        throw CException(ERR_REFUND_REQUEST, "unconfirm fund not exist. ", __FILE__, __LINE__); //add errorcode     
	    }

	    if(m_fundUnconfirm.Flstate== UNCONFIRM_FUND_INVALID)
		{
			gPtrAppLog->error("unconfirm flstate invalid.");
			throw CException(ERR_UNCONFIM_LSTATE, "unconfirm flstate invalid.", __FILE__, __LINE__);
		}
		//�˿���ܴ���δȷ�Ϸݶ�
	    if(m_fundUnconfirm.Ftotal_fee < m_params.getLong("total_fee"))
		{
	        gPtrAppLog->error("not enough unconfirm money.");
	        throw CException(ERR_REFUND_REQUEST, "not enough unconfirm money.", __FILE__, __LINE__); //add errorcode     
			
	    }
	}
	
	// ���ָ������δ�������
	strncpy(m_fundIndexTrans.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	hasTransProcess = queryFundTransProcess(m_pFundCon,m_fundIndexTrans,true);

	//TODO:fundIndexTrans�����߼�:�ڶ������Ӽ���±�����
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
	{
		char szMsg[128]={0};
		if(hasTransProcess)
		{
			if(m_fundIndexTrans.Flstate==PROCESS_TRANS_LSTATE_INVALID)
			{
				snprintf(szMsg, sizeof(szMsg), "ָ���ͻ����˿�,ָ���������ݷǷ�[%s][%s]",m_stTradeBuy.Flistid,m_fund_bind.Ftrade_id);
				throw CException(ERR_INDEX_REFUND_TRANS, szMsg, __FILE__, __LINE__);
			}
			if(m_fundIndexTrans.Fstate==PROCESS_TRANS_STATE_BUY_CONFIRM||m_fundIndexTrans.Fstate==PROCESS_TRANS_STATE_BUY_USABLE)
			{
				snprintf(szMsg, sizeof(szMsg), "ָ���ͻ����˿�,ָ����������״̬�������˿�[%s][%s][%d]",m_stTradeBuy.Flistid,m_fund_bind.Ftrade_id,m_fundIndexTrans.Fstate);
				throw CException(ERR_INDEX_REFUND_TRANS, szMsg, __FILE__, __LINE__);
			}
		}
	}
}

/**
* ����δȷ�Ϸݶ�
*/

void FundRefund::UpdateFundUnconfirm()
{
	if(m_fundSpConfig.Fbuy_confirm_type == 0)
	{
		return;
	}

	if(!hasTransProcess)
	{
		return;	
	}
	
	//TODO:fundUnconfirm ɾ���߼�:������ɾ��unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
	    m_fundUnconfirm.Ftotal_fee=m_fundUnconfirm.Ftotal_fee-m_params.getLong("total_fee");
	    strncpy(m_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_fundUnconfirm.Fmodify_time)-1);
	    updateFundUnconfirmById(m_pFundCon,m_fundUnconfirm);
	}

	//  ����ָ�����������
	FundTransProcess fundIndexTrans;	
	fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(fundIndexTrans.Ftrade_id)-1);
	fundIndexTrans.Fstate = PROCESS_TRANS_STATE_BUY_CONFIRM_FAIL;
	strncpy(fundIndexTrans.Ffinish_time,m_params.getString("systime").c_str(),sizeof(fundIndexTrans.Ffinish_time)-1);
	strncpy(fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(fundIndexTrans.Fmodify_time)-1);
	
	//updateFundTransProcess(m_pFundCon,fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,fundIndexTrans,m_fundIndexTrans);

   
}

/**
* ����������
*/
void FundRefund::packReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
	CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "uin", m_params.getString("uin").c_str());
	rqst->olen = strlen(rqst->odata);
	return;
}


