/**
  * FileName: abstract_redeem_sp_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-3-12
  * Description: �����׷��� �������ȷ��
  */

#include "fund_commfunc.h"
#include "abstract_redeem_sp_ack_service.h"

AbstractRedeemSpAck::AbstractRedeemSpAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));
    memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
    
    m_need_updateExauAuthLimit = false;
    m_subAccDrawOk = false;
	m_balanceChanged = false;
}

/**
  * service step 1: �����������
  */
void AbstractRedeemSpAck::parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg)  throw (CException)
{
	// Ҫ�����������ݣ��ײ��ʹ��
    m_request = rqst;
    
    TRACE_DEBUG("[abstract_redeem_sp_ack_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
    m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
    m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
    m_params.readStrParam(szMsg, "spid", 10, 15);
    m_params.readStrParam(szMsg, "sp_billno", 0, 32);
    m_params.readStrParam(szMsg, "fund_code", 0, 64);
    m_params.readIntParam(szMsg, "op_type", 1, 6);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "desc", 0, 128);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

	m_params.readLongParam(szMsg,"redeem_total_fee",0,MAX_LONG);
	m_params.readLongParam(szMsg,"redeem2usr_fee",0,MAX_LONG);
	m_params.readLongParam(szMsg,"charge_fee",0,MAX_INTEGER);
	m_params.readStrParam(szMsg,"charge_type",0,1); // �շѷ�ʽ

    m_optype = m_params.getInt("op_type");
	
    parseBizInputMsg(szMsg); // ��ȡҵ�����

    char szTimeNow[MAX_TIME_LENGTH+1] = {0};
    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}
/**
  * �������е��̻�����,�����ظ���ѯ
  */
void AbstractRedeemSpAck::setSpConfig(FundSpConfig fundSpConfig)
{
    m_fund_sp_config = fundSpConfig;
}

/**
  * ����������ȡ�ڲ�����
  */
void AbstractRedeemSpAck::CheckParams() throw (CException)
{
    if(INF_REDEM_SP_ACK_SUC == m_optype)
    {
        CHECK_PARAM_EMPTY("sp_billno");   
    }
	
	// ����շѷ�ʽ����ֵ����
	if(!isDigitString(m_params.getString("charge_type").c_str()))
	{
        throw CException(ERR_BAD_PARAM, string("Param is not a int value:charge_type=")+ m_params.getString("charge_type"), __FILE__, __LINE__);
	}

}

/**
  * ִ���깺����
  */
void AbstractRedeemSpAck::excute() throw (CException)
{
    try
    {
        CheckParams();

        CheckFundBind();

         /* �������� */
        m_pFundCon->Begin();

        /* ckv�����ŵ�����֮�������ύ */
        gCkvSvrOperator->beginCkvtrans();

        /* ��ѯ�����׼�¼ */
        CheckFundTrade();

        /* ���˻������»�����״̬ */
        UpdateTradeState();

        /* �ύ���� */
        m_pFundCon->Commit();

        gCkvSvrOperator->commitCkvtrans();

        updateExauAuthLimitNoExcp();

        /* ���¸���ckv ,��������֮���Ǳ�������ع�ȴд��ckv������*/
        updateCkvs();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
		
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


/*
 * ��ѯ�����˻��Ƿ����
 */
void AbstractRedeemSpAck::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}


/**
  * �������׼�¼�Ƿ��Ѿ�����
  */
void AbstractRedeemSpAck::CheckFundTrade() throw (CException)
{
	// û�н��׼�¼������
	if(!QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		m_params.getInt("bank_type"), &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, cft_bank_billno[%s]  ", m_params.getString("cft_bank_billno").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	// ����״̬��Ч������
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund buy pay, lstate is invalid. listid[%s], uid[%d] ", m_stTradeBuy.Flistid, m_stTradeBuy.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy pay, lstate is invalid. ", __FILE__, __LINE__);
	}

	// ���״̬�Ϸ���
	if(m_stTradeBuy.Fstate<=0||m_stTradeBuy.Fstate>=TRADE_STATE_SIZE)
	{   
		char errMsg[128] = {0};
		snprintf(errMsg,sizeof(errMsg),"��ض���״̬�쳣[%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Fstate);
		throw CException(ERR_TRADE_INVALID, errMsg, __FILE__, __LINE__);
	}

/*   �ŵ�����У��

	// У��ؼ�����
	LONG checkFee=m_stTradeBuy.Ftotal_fee;
	// ֻ���Ѿ��ۼ����˻��ɹ�����ص�, �������ֽ������ؽ�һ�����
	if((m_stTradeBuy.Fstate==REDEM_SUC||m_stTradeBuy.Fstate==REDEM_FINISH)&&m_stTradeBuy.Freal_redem_amt>0){
		checkFee=m_stTradeBuy.Freal_redem_amt;
	}
	if(checkFee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
*/

	if(m_stTradeBuy.Fuid!=m_params.getInt("uid"))
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_stTradeBuy.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}

	if( (0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str())))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}

	// �õ����������Ϣ
	m_params.setParam("uid", m_stTradeBuy.Fuid);
	m_params.setParam("trade_id", m_stTradeBuy.Ftrade_id);

}

/**
*�����������ͣ�����Ӧ�Ĵ��� 
*/
void AbstractRedeemSpAck::UpdateTradeState()
{
	switch (m_optype)
    {       
        case INF_REDEM_SP_ACK_SUC: // ������˾��سɹ�
			UpdateRedemTradeForSpInfoSuc();
            break;
			
		case INF_REDEM_SP_ACK_TIMEOUT: // ������˾��س�ʱ
            UpdateRedemTradeForInfoTimeout();
            break;
			
		case INF_REDEM_SP_ACK_FAIL: // ������˾���ʧ��
            UpdateRedemTradeForInfoFail();
            break;
                    
		case INF_REDEM_BALANCE_ACK_SUC: //�ӳ�����ʹ��:����˾֪ͨ��طݶ�ȷ�ϳɹ�
            UpdateRedemTradeForAckSuc();
            break;
			
		case INF_REDEM_BALANCE_ACK_FAIL: //�ӳ�����ʹ��:����˾֪ͨ��طݶ�ʧ�ܳɹ�
            UpdateRedemTradeForAckFail();
            break;

		case INF_REDEM_SP_ACK_FINISH: // // ��ص��������
            UpdateRedemTradeForFinish();
            break;
			
        default:
            throw CException(ERR_BAD_PARAM, "op_type invalid", __FILE__, __LINE__);
            break;

    }
}

/**
 *  ʵʱ֪ͨ����˾�ɹ�
 */ 
void AbstractRedeemSpAck::UpdateRedemTradeForSpInfoSuc() throw (CException)
{
	//��ʱ�����Ķ���ֱ��ȥ����
	if(TRADE_RECORD_TIMEOUT == m_stTradeBuy.Fspe_tag)
	{
		UpdateRedemTradeForBudan();
		return;
	}
	
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		// ʵʱȷ������:ֱ�Ӹ���Ϊ��سɹ�
		UpdateRedemTradeForSuc();
		return;
	}else
	{
		// �ӳ�ȷ������:��֪ͨ�ɹ�
		UpdateRedemTradeForInfoSuc();
	}
}

void AbstractRedeemSpAck::CheckRedemTradeForInfoSuc() throw (CException)
{
	char errMsg[128]={0};
	
	// �������	
	if(REDEEM_STATE_ORDER[REDEEM_INFO_SUC]<=REDEEM_STATE_ORDER[m_stTradeBuy.Fstate])
	{            
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}
	
	// ��֧�ֺ�Լ����������
	if (PURPOSE_UNFREEZE_FOR_FETCH == m_stTradeBuy.Fpurpose)
	{
		snprintf(errMsg,sizeof(errMsg),"�������֪ͨ���ۿ�, ��֧�ֺ�Լ������[%s]",m_stTradeBuy.Flistid);
        throw CException(ERR_BAD_PARAM, errMsg, __FILE__, __LINE__);
	}

	// ���֪ͨ��֧�ֵ�֧���
	if(m_stTradeBuy.Floading_type==DRAW_USE_LOADING)
	{
		snprintf(errMsg,sizeof(errMsg),"���֪ͨ��֧�ֵ���[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Fspid);
        throw CException(ERR_BAD_PARAM, errMsg, __FILE__, __LINE__);
	}

	// ���ݶ�һ����
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		snprintf(errMsg,sizeof(errMsg),"���֪ͨ�ݶһ��[%s][%s][%ld][%ld]",m_stTradeBuy.Flistid,m_stTradeBuy.Fspid,m_stTradeBuy.Ftotal_fee,m_params.getLong("total_fee"));
		throw CException(ERR_BAD_PARAM, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// ֻ����4״̬�Ķ���
	if(REDEM_ININ!=m_stTradeBuy.Fstate)
	{
		snprintf(errMsg,sizeof(errMsg),"���֪ͨ����״̬�Ƿ�[%s][%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Fspid,m_stTradeBuy.Fstate);
		throw CException(ERR_BAD_PARAM, errMsg, __FILE__, __LINE__);	
	}
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_NONE);
	}
	return;
}

/**
  *  ��װ��ص�
  */
void AbstractRedeemSpAck::BuildRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
		if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
		{
			stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���		
		}else{
			stRecord.Fspe_tag = 0;
		}
        stRecord.Fstate = REDEEM_INFO_SUC;
        strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
        strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
        stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
        stRecord.Fuid = m_stTradeBuy.Fuid;
        //����trade_id,���½��׼�¼ʱ��Ҫʹ��
        SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
        strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

        stRecord.Fpurpose = m_stTradeBuy.Fpurpose;
        stRecord.Floading_type = m_stTradeBuy.Floading_type;
        strncpy(stRecord.Fcharge_type, m_params.getString("charge_type").c_str(), sizeof(stRecord.Fcharge_type) - 1);
}

/**
  *  �������ȷ������
  */
void AbstractRedeemSpAck::RecordRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
    UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
 *  ��¼�����ص�ͳ������
 */
void AbstractRedeemSpAck::RecordSpconfigForInfoSuc() throw (CException)
{
    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));

    strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
    {
    	//��Ӧ�÷���
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }

    //��������ۼƶ��,t+1������ز��ۼƣ�����������ͨ��ء����ѡ�t+0��ض�Ҫ�ۼ�
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    updateFundSpRedomTotal(m_pFundCon, fundSpConfig, m_stTradeBuy.Ftotal_fee,m_stTradeBuy.Floading_type,m_stTradeBuy.Facc_time);
	
}

/**
 *  ʵʱ֪ͨ����˾�ɹ�
 */ 
void AbstractRedeemSpAck::UpdateRedemTradeForInfoSuc() throw (CException)
{
	/**
	  *�ӳ�ȷ������:����Ϊ���֪ͨ�ɹ�
         */
	// ���״̬
	CheckRedemTradeForInfoSuc();
	
	// ���������˻�,��������󶳽�
	//doDrawReq();

	// ���˻����³ɹ�,һ��Ҫ�ѵ����ɹ�
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	try
	{
    	// ��װ���׵�
    	BuildRedemTradeForInfoSuc(stRecord);
		// ��¼���׵�
		RecordRedemTradeForInfoSuc(stRecord);
		//��¼spconfig����
		//���ﲻ��Ҫ������,�ŵ�����¼,��������ʱ�����
		RecordSpconfigForInfoSuc();
    }
    catch(CException& e)
    {
        alert(e.error(), string(stRecord.Flistid)+string("������˻��Ѿ�����ɹ���������ʧ��:ԭ��")+e.what());
        //���˻��Ѿ��ɹ�����Ҫ�ײ����
        if(ERR_TYPE_MSG)
        {
            //���Բ����ʱ�����ص�����ʱ�䳬��10���ӵĸ澯
            int inteval = (int)(time(NULL) - toUnixTime(m_stTradeBuy.Facc_time));	
            gPtrAppLog->warning("fund_deal_server.inteval:%ds", inteval);
			
            //����10������δ�ɹ��澯,�澯���������ѹ�ƣ������ײ��
            if(inteval >= 600 )
            {	
                char szErrMsg[256] = {0};
                snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.��ز��������%d����δ�ɹ�[%s]" ,inteval,stRecord.Flistid);
                alert(ERR_BUDAN_TOLONG, szErrMsg);
                throw;
            }
        }
        // ����Ϣ�����
        callErrorRpc(m_request, gPtrSysLog);
        
        throw;
    }
    
	//���׼�¼��MQ,��װ�仯�Ĳ���
	SyncRedemTradeForSuc(stRecord);

	//TODO : ȷ���Ƿ���MQ
	// ����MQ��Ϣ
	sendFundBuy2MqMsg(m_stTradeBuy);
	
}

void AbstractRedeemSpAck::updateWxPrePayUserBalance()throw (CException)
{
    //���������û�ֱ�ӷ���
    if (isWxPrePayCardBusinessUser(m_pFundCon,m_params["spid"],m_fund_bind.Ftrade_id) == false)
    {
        return;
    } 
    
    ST_FUND_CONTROL_INFO controlInfo;
    memset(&controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
    strncpy(controlInfo.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(controlInfo.Ftrade_id)),
    controlInfo.Ftype=1;
    if (false == queryFundControlInfo(m_pFundCon,controlInfo,true)) //������ѯ
    {
        //�����ܷ���
        throw CException(ERR_DB_UNKNOW, "queryFundControlInfo fail", __FILE__, __LINE__);
    }

    if (m_stTradeBuy.Fpurpose == PURPOSE_UNFREEZE_FOR_FETCH) //�̻���ؿۿ�
    {
        ST_UNFREEZE_FUND unFreezedata;
        memset(&unFreezedata,0,sizeof(ST_UNFREEZE_FUND));
        strncpy(unFreezedata.Funfreeze_id,m_stTradeBuy.Fcft_trans_id,sizeof(unFreezedata.Funfreeze_id)-1);
        if (false==queryFundUnFreezeByUnfreezeid(m_pFundCon,unFreezedata,false))
        {
            throw CException(ERR_QUERY_UNFREEZE_BILL, "query unfreeze bill fail ", __FILE__, __LINE__);
        }

        if ((string(unFreezedata.Fspid) == gPtrConfig->m_AppCfg.wx_wfj_spid)) //΢��������Ԥ�����̻���
        {
            if (controlInfo.Ftotal_fee  < m_params.getLong("total_fee")) //΢������������ۿ������������޽��
            {
                throw CException(ERR_USER_BALANCE_CONTROLED, "wx wfj controled, not enough money", __FILE__, __LINE__);
            }

            subFundControlBalance(m_pFundCon, m_params.getLong("total_fee") ,m_params["systime"],m_fund_bind.Ftrade_id);

            //���¶��ᵥ״̬Ϊ����ɹ�����������������سɹ�
            ST_UNFREEZE_FUND unFreezedataSet;
            memset(&unFreezedataSet,0,sizeof(ST_UNFREEZE_FUND));
            strncpy(unFreezedataSet.Funfreeze_id,m_stTradeBuy.Fcft_trans_id,sizeof(unFreezedataSet.Funfreeze_id)-1);
            unFreezedataSet.Fstate = FUND_UNFREEZE_OK;
            strncpy(unFreezedataSet.Fmodify_time,m_params["systime"],sizeof(unFreezedataSet.Fmodify_time)-1);
            strncpy(unFreezedataSet.Facc_time,m_params["systime"],sizeof(unFreezedataSet.Facc_time)-1);
            updateFundUnFreeze(m_pFundCon,unFreezedataSet, unFreezedata);
        }
        
    }
    else   //�û�������ز���������޽��
    {
        LONG balance = querySubaccBalance(m_params.getInt("uid"),querySubaccCurtype(m_pFundCon, m_params.getString("spid")),true);
        if ( balance<m_params.getLong("total_fee")+controlInfo.Ftotal_fee) //�������û�������ؽ��������ز��ֵĽ��
        {
            throw CException(ERR_USER_BALANCE_CONTROLED, "wx wfj controled, not enough money", __FILE__, __LINE__);
        }
    }
    return;
}

bool AbstractRedeemSpAck::CheckRedemTradeForSuc() throw (CException)
{	
	if(REDEM_SUC == m_stTradeBuy.Fstate || REDEM_FINISH == m_stTradeBuy.Fstate)
	{            
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	// ���ݶ�һ����
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// ���״̬�Ƿ���Ը���:ֻ����4��5״̬�Ľ��׵�
	if( REDEM_ININ != m_stTradeBuy.Fstate)
	{
		//��Ϊ��س�ʼ״̬���������ٴθ��³ɹ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
    }
	// ����Ĭ����������ȡ��ʽ
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_NONE);
	}
	return true;
}

/**
  *  ���˻����
  */
void AbstractRedeemSpAck::DrawSubacc() throw (CException)
{
	
	//�������˻�������ʱ���Ƚϳ��������ڵ��ʳ���ʱ�����˻������ŵ����������ñ�֮ǰ,������������
	if ((m_stTradeBuy.Floading_type == DRAW_USE_LOADING)
		&& false == preCheckSpLoaningEnough(m_pFundCon,m_stTradeBuy.Fspid,m_stTradeBuy.Ffund_code,m_stTradeBuy.Ftotal_fee))
	{
		//����˾��֧�����ü��
		checkSpLoaning();
		//�ȼ��˻��������ʧ��ֱ�ӱ����ع�����
		doDrawResult(SUBACC_FETCH_RESULT_OK);
	}
	else
	{
		//�ȼ��˻��������ʧ��ֱ�ӱ����ع�����
		doDrawResult(SUBACC_FETCH_RESULT_OK);
		//����˾��֧�����ü��
		try
		{
			checkSpLoaning();
		}
		catch(CException& e)
		{
			alert(e.error(), string("������˻��Ѿ����ɹ�����������ʧ��(���ܵ��µ��ʹ�������):ԭ��")+e.what());
		}
	}
}

/**
  *  ��װ��ص�
  */
void AbstractRedeemSpAck::BuildRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
        if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
        {
            stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���		
        }
        else
        {
            stRecord.Fspe_tag = 0;//��ʱ�����ɹ���Ҫ����ʱ״̬�޸ģ������²�ͣ����
        }

        stRecord.Fstate = REDEM_SUC;
        strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
        strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
        stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
        stRecord.Fuid = m_stTradeBuy.Fuid;
        //����trade_id,���½��׼�¼ʱ��Ҫʹ��
        SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
        strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

        stRecord.Fpurpose = m_stTradeBuy.Fpurpose;
        stRecord.Floading_type = m_stTradeBuy.Floading_type;
        strncpy(stRecord.Fcharge_type, m_params.getString("charge_type").c_str(), sizeof(stRecord.Fcharge_type) - 1);
}

/**
  *  �������ȷ������
  */
void AbstractRedeemSpAck::RecordRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
    UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
  *  ͬ������ɹ��Ĳ�����ȫ�ֱ���
  */
void AbstractRedeemSpAck::SyncRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
	//���׼�¼��MQ,��װ�仯�Ĳ���
	m_stTradeBuy.Fstate= stRecord.Fstate;
	strncpy(m_stTradeBuy.Fcoding, stRecord.Fcoding, sizeof(m_stTradeBuy.Fcoding) - 1);
	strncpy(m_stTradeBuy.Fmemo, stRecord.Fmemo, sizeof(m_stTradeBuy.Fmemo) - 1);
	m_stTradeBuy.Fpurpose = stRecord.Fpurpose;
}

/**
*	������˾��سɹ�����:����Ϊ5״̬
*	�������˻����
*/
void AbstractRedeemSpAck::UpdateRedemTradeForSuc() throw (CException)
{
	// ����˾�Ѿ�֪ͨ��������ٴ�ȷ��:����13��5״̬�Ľ��׵�
	if(REDEEM_INFO_SUC == m_stTradeBuy.Fstate)
	{
		UpdateRedemTradeForAckSuc();
		return;
	}
	// ������	
	if(!CheckRedemTradeForSuc())
	{
		return;
	}

    //΢��Ԥ����������Ŀ��ʱ����
    updateWxPrePayUserBalance();

	//�����˻�
	DrawSubacc();
	
    //�����˻��ɹ�������ҵ��ɹ���������ص�ʧ�ܷ���������������ɹ�
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
    try
    {
    	// ��װ���׵�
    	BuildRedemTradeForSuc(stRecord);
		// ��¼���׵�
		RecordRedemTradeForSuc(stRecord);
    }
    catch(CException& e)
    {
        alert(e.error(), string("������˻��Ѿ����ɹ���������ʧ��:ԭ��")+e.what());
        //���˻��Ѿ��ɹ�����Ҫ�ײ����
        if(ERR_TYPE_MSG)
        {
            //���Բ����ʱ�����ص�����ʱ�䳬��10���ӵĸ澯
            int inteval = (int)(time(NULL) - toUnixTime(m_stTradeBuy.Facc_time));	
            gPtrAppLog->warning("fund_deal_server.inteval:%ds", inteval);
			
            //����10������δ�ɹ��澯,�澯���������ѹ�ƣ������ײ��
            if(inteval >= 600 )
            {	
                char szErrMsg[256] = {0};
                snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.��ز��������%d����δ�ɹ�" ,inteval);
                alert(ERR_BUDAN_TOLONG, szErrMsg);
                throw;
            }
        }
        // ����Ϣ�����
        callErrorRpc(m_request, gPtrSysLog);
        
        throw;
    }
    
    // �����仯
    m_balanceChanged = true;

	//���׼�¼��MQ,��װ�仯�Ĳ���
	SyncRedemTradeForSuc(stRecord);
	
	// ����MQ��Ϣ
	sendFundBuy2MqMsg(m_stTradeBuy);
}


bool AbstractRedeemSpAck::CheckRedemTradeForAckSuc() throw (CException)
{	
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		throw CException(ERR_REDEM_DRAW_REFUSE, "ʵʱ�ݶ�ȷ�����Ͳ����ڻ���˾֪ͨ", __FILE__, __LINE__);
	}
	
	if(REDEM_SUC == m_stTradeBuy.Fstate || REDEM_FINISH == m_stTradeBuy.Fstate)
	{            
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	// ���ݶ�һ����
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// ���״̬�Ƿ���Ը���:ֻ����13��5״̬�Ľ��׵�
	if( REDEEM_INFO_SUC!= m_stTradeBuy.Fstate)
	{
		throw CException(ERR_REDEM_DRAW_REFUSE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
    }
	// �����
	CHECK_PARAM_EMPTY("redeem_total_fee"); //����ܽ��
	CHECK_PARAM_EMPTY("redeem2usr_fee"); //��ظ��û����
	CHECK_PARAM_EMPTY("charge_fee"); //��ظ��û����
	// �����һ����
	if(m_params.getLong("charge_fee")+m_params.getLong("redeem2usr_fee")!=m_params.getLong("redeem_total_fee") )
	{
        char szErrMsg[128] = {0};
        snprintf(szErrMsg, sizeof(szErrMsg), "���ȷ�Ͻ�һ��[%s][%ld][%ld][%ld]" ,m_stTradeBuy.Flistid,m_params.getLong("charge_fee"),m_params.getLong("redeem2usr_fee"),m_params.getLong("redeem_total_fee"));
		throw CException(ERR_REDEM_FEE_UNCONSISTENT, szErrMsg, __FILE__, __LINE__);
	}
	return true;
}

/**
  *  ��װ��ص�
  */
void AbstractRedeemSpAck::BuildRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
        strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
        stRecord.Fuid = m_stTradeBuy.Fuid;
        strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
		stRecord.Fpur_type = m_stTradeBuy.Fpur_type;

        stRecord.Fspe_tag = 0;
        stRecord.Fstate = REDEM_SUC;
        strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
		stRecord.Freal_redem_amt = m_params.getLong("redeem2usr_fee");
		stRecord.Fcharge_fee = m_params.getLong("charge_fee");		
		if(strcmp(m_stTradeBuy.Fcoding,"")==0)
		{
        	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
		}
}

/**
  *  �������ȷ������
  */
void AbstractRedeemSpAck::RecordRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
    UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
  *  ͬ������ɹ��Ĳ�����ȫ�ֱ���
  */
void AbstractRedeemSpAck::SyncRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
	//���׼�¼��MQ,��װ�仯�Ĳ���
	m_stTradeBuy.Fstate= stRecord.Fstate;
	strncpy(m_stTradeBuy.Fcoding, stRecord.Fcoding, sizeof(m_stTradeBuy.Fcoding) - 1);
	strncpy(m_stTradeBuy.Fmemo, stRecord.Fmemo, sizeof(m_stTradeBuy.Fmemo) - 1);
	m_stTradeBuy.Fpurpose = stRecord.Fpurpose;
	m_stTradeBuy.Fspe_tag = stRecord.Fspe_tag;
	m_stTradeBuy.Freal_redem_amt = stRecord.Freal_redem_amt;
	m_stTradeBuy.Fcharge_fee = stRecord.Fcharge_fee;	
}

/**
*	������˾��سɹ�����
*      13 ��5״̬����
*	�������˻����
*/
void AbstractRedeemSpAck::UpdateRedemTradeForAckSuc() throw (CException)
{
	// ������	
	if(!CheckRedemTradeForAckSuc())
	{
		return;
	}

	//ȷ�ϳɹ������˻�
	doDrawResult(SUBACC_FETCH_RESULT_OK);
	
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	// ��װ���׵�
    BuildRedemTradeForAckSuc(stRecord);
	// ��¼���׵�
	RecordRedemTradeForAckSuc(stRecord);
  
    // �����仯
    m_balanceChanged = true;

	//���׼�¼��MQ,��װ�仯�Ĳ���
	SyncRedemTradeForSuc(stRecord);
	
	// ����MQ��Ϣ
	sendFundBuy2MqMsg(m_stTradeBuy);

}

bool AbstractRedeemSpAck::CheckRedemTradeForAckFail() throw (CException)
{
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		throw CException(ERR_REDEM_DRAW_REFUSE, "ʵʱ�ݶ�ȷ�����Ͳ����ڻ���˾֪ͨ", __FILE__, __LINE__);
	}
	// ���ݶ�һ����
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_BAD_PARAM, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// �������
	if(m_stTradeBuy.Fstate==REDEM_FAIL)
	{
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record state has update ", __FILE__, __LINE__);
	}
	// ���״̬�Ƿ���Ը���:ֻ����13��6״̬�Ľ��׵�
	if( REDEEM_INFO_SUC!= m_stTradeBuy.Fstate)
	{
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
    }
	return true;
}

void AbstractRedeemSpAck::BuildRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	stRecord.Fstate = REDEM_FAIL;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
    strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
}

void AbstractRedeemSpAck::RecordRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	// ��������
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}


/**
*	���ȷ��ʧ�ܴ���
*/
void AbstractRedeemSpAck::UpdateRedemTradeForAckFail() throw (CException)
{
	// ������
	if(!CheckRedemTradeForAckFail())
	{
		return;
	}
	// ȷ��ʧ�ܽⶳ�ݶ�	
	doDrawResult(SUBACC_FETCH_RESULT_FAIL);
	
	// �޸���ص�״̬Ϊʧ��
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	BuildRedemTradeForAckFail(stRecord);
	RecordRedemTradeForAckFail(stRecord);
}


void AbstractRedeemSpAck::checkSpLoaning() throw (CException)
{
    //�������Ϊ���ֲ��ۼ���ض��
    //������¼ʱĬ��Fpurpose = PURPOSE_DRAW_T1,��سɹ��ž���ʹ����������
    //��ʱ��������T+1�����⣬������ض��ߵ���
    /*
    if(m_stTradeBuy.Fpurpose != PURPOSE_DRAW_T1 && m_stTradeBuy.Fpurpose != PURPOSE_DRAW_T0)
    {
    	return;
    }
    */

    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));

    strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
    {
    	//��Ӧ�÷���
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }

    if(m_stTradeBuy.Floading_type == DRAW_USE_LOADING) // ��Ҫ����
    {
        checkRedemOverLoading(m_pFundCon, fundSpConfig, m_params.getLong("total_fee"),true);
    }

    //��������ۼƶ��,t+1������ز��ۼƣ�����������ͨ��ء����ѡ�t+0��ض�Ҫ�ۼ�
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    updateFundSpRedomTotal(m_pFundCon, fundSpConfig, m_stTradeBuy.Ftotal_fee,m_stTradeBuy.Floading_type,m_stTradeBuy.Facc_time);
	
}


/**
*��� ֪ͨ��ʱ����
*/
void AbstractRedeemSpAck::UpdateRedemTradeForInfoTimeout() throw (CException)
{
	//�������þ�����س�ʱ���ɹ�����ʧ�ܣ�������ɹ�������Ҫ�����������ܳ���Գ�ʱ��ص����в���
	if("true" == gPtrConfig->m_AppCfg.redem_timeout_conf)
	{
		UpdateRedemTradeForSpInfoSuc();
	}
	else
	{
		UpdateRedemTradeForInfoFail();
	}
}
void AbstractRedeemSpAck::CheckRedemTradeForInfoFail() throw (CException)
{
	
	if(REDEM_FAIL == m_stTradeBuy.Fstate)
	{
		//�������
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_ININ != m_stTradeBuy.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
}

void AbstractRedeemSpAck::BuildRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���		
	}
	stRecord.Fstate = REDEM_FAIL;		
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
    //����trade_id,���½��׼�¼ʱ��Ҫʹ��
    strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
}

void AbstractRedeemSpAck::RecordRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	// ��������
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
*	���֪ͨʧ�ܴ���
*/
void AbstractRedeemSpAck::UpdateRedemTradeForInfoFail() throw (CException)
{
	// ������
	CheckRedemTradeForInfoFail();
	
	// ȷ��ʧ���Ƚⶳ�ݶ�	
	doDrawResult(SUBACC_FETCH_RESULT_FAIL);

	// ��������
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	BuildRedemTradeForInfoFail(stRecord);
	RecordRedemTradeForInfoFail(stRecord);
}


void AbstractRedeemSpAck::BuildRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException)
{
	// ��ص����ֱ�Ӹ��µ���ʱ��͵���״̬
	if(m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA||m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA_T1)
	{
		strncpy(stRecord.Ffetch_arrival_time,m_params.getString("systime").c_str(),sizeof(stRecord.Ffetch_arrival_time));
		stRecord.Ffetch_result=FETCH_RESULT_BALANCE_SUCCESS;
	}

	stRecord.Fstate = REDEM_FINISH;

	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fspe_tag = m_stTradeBuy.Fspe_tag; //�����޸ĳ�ʱ���
	stRecord.Fuid = m_stTradeBuy.Fuid;
    //����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
}

void AbstractRedeemSpAck::RecordRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException)
{
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

void AbstractRedeemSpAck::UpdateRedemTradeForFinish() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	if(REDEM_FINISH == m_stTradeBuy.Fstate)
	{
		//�������
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_SUC != m_stTradeBuy.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	BuildRedemTradeForFinish(stRecord);
	RecordRedemTradeForFinish(stRecord);
}

/**
  * 5״̬�ĳ�ʱ����
  */
void AbstractRedeemSpAck::UpdateRedemTradeForBudan() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	if(REDEM_SUC != m_stTradeBuy.Fstate && REDEM_FINISH != m_stTradeBuy.Fstate && REDEEM_INFO_SUC!=m_stTradeBuy.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	if(TRADE_RECORD_TIMEOUT != m_stTradeBuy.Fspe_tag || INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		//�ǳ�ʱ�������������,���߳�ʱ�����룬ֱ�ӷ���
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}
	
	stRecord.Fspe_tag = 0;
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;    
    //����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

bool AbstractRedeemSpAck::payNotifyOvertime(string pay_suc_time)
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


/**
 * ���˻���������
 * ���˻�ʧ���˲��ܲ�������û����������ٷ�����أ�������������ػ���ɲƸ�ͨ��ʧ
 * д������ģ���ص������Ի���˾�ɹ�Ϊ׼�����������˻�Ϊ׼��һֱ�����ۣ�ǰ�����۶������ķ������������������˾����ó���˵�¡�����
 * ����漰���ϵͳ ����˾+�����ֵϵͳ+���˻�+�Ƹ�ͨ +��������ز�����Ƶ�ϵͳ
 * һ�����ԭ���Ǽ�Ǯ�ɹ��ӻ����׵��ɹ�����Ϊ�ɹ���������ʧ�ܶ�ͨ����������ɡ�
 * ��Ǯ�漰������ϵͳ������˾ϵͳ���ݶ���˻���Ǯ���������ĸ�Ϊ׼? ��˭Ϊ׼��������:
 * ������Ϊ�����˻��ɹ�Ϊ׼��Ϊ���ף�����: 1)���˻��ǲƸ�ͨ�ɿ��Ƶģ��ɹ�����ɻ���˾������2)���˻��쳣�����ǿ���ͨ��ʵʱ���˷��֣�����ʱ������
 * ���ڵ�����:���˻�ʧ��δ��Ǯ������˾�Ѿ����ݶ�û��޷��ٴ���أ�����ͨ���ڶ��������˾�Ķ��˰��û��ʽ��ϣ����������Ż��������൥ʵʱ֪ͨ����˾���ʧ�ܣ�
 *
 */
void AbstractRedeemSpAck::doDraw() throw (CException)
{
	gPtrAppLog->debug("doDraw, listid[%s]  ", m_stTradeBuy.Fsub_trans_id);
	string subAccControlList; //��Լ���ⶳ���� ���˻��ܿص���

   if (PURPOSE_UNFREEZE_FOR_FETCH == m_stTradeBuy.Fpurpose)
   {
       //��Լ���ⶳ�ۿ��Ҫ�Ȳ�ѯ�����˻����ܿص���
       ST_UNFREEZE_FUND unFreezedata;
       memset(&unFreezedata,0,sizeof(ST_UNFREEZE_FUND));
       strncpy(unFreezedata.Funfreeze_id,m_stTradeBuy.Fcft_trans_id,sizeof(unFreezedata.Funfreeze_id)-1);
       if (false==queryFundUnFreezeByUnfreezeid(m_pFundCon,unFreezedata,false))
       {
            throw CException(ERR_QUERY_UNFREEZE_BILL, "query unfreeze bill fail ", __FILE__, __LINE__);
       }
       subAccControlList = unFreezedata.Fsub_acc_control_no;
   }


	try
	{
	    SubaccDraw(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeBuy.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeBuy.Facc_time,subAccControlList);
           m_subAccDrawOk = true;
	}
	
	catch(CException& e)
	{

		//���Ҫ��������
		//���������˻���Ǯ����10����û�ɹ��ĸ澯���ڲ����������ǲ���������ⲿ���ܲ�����10����û�ɹ����쳣ʱ���ᴥ���澯
		if(payNotifyOvertime(m_stTradeBuy.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.��ز�������10�������˻���δ�ɹ�");		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// ����Ϣ�����
		callErrorRpc(m_request, gPtrSysLog);

		throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�
		
	}
	
}

void AbstractRedeemSpAck::doDrawReq() throw (CException)
{    
	gPtrAppLog->debug("doDrawReq, listid[%s]  ", m_stTradeBuy.Fsub_trans_id);
    try
    {
        SubaccFetchReq(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeBuy.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeBuy.Facc_time,m_fund_sp_config.Fcurtype);
           m_subAccDrawOk = true;
    }
    catch(CException &e)
    {
		// ������˻���ض��ᳬ��10����û�ɹ��ĸ澯���ٲ�����
		// �����ǲ���������ⲿ���ܲ�����10����û�ɹ����쳣ʱ���ᴥ���澯
		if(payNotifyOvertime(m_stTradeBuy.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.��ز�������10�������˻���δ����ɹ�[%s]",m_stTradeBuy.Flistid);		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// ����Ϣ�����
		callErrorRpc(m_request, gPtrSysLog);

		throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�
    }
}

void AbstractRedeemSpAck::doDrawResult(int result) throw (CException)
{
	gPtrAppLog->debug("doDrawResult, listid[%s][%d]  ", m_stTradeBuy.Fsub_trans_id,result);

	string subaccTime;
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)	
	{	
		// ֱ��ȷ�Ͻ��ʹ��acc_time���в����
		subaccTime = m_stTradeBuy.Facc_time;
	}else{
		// ��ʱȷ������ʹ�õ�ǰʱ��ȷ������,�ȴ���������
		subaccTime = m_params.getString("systime");
	}

	try
	{
	    SubaccFetchResult(gPtrSubaccRpc, m_fund_sp_config.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeBuy.Fsub_trans_id, m_stTradeBuy.Ftotal_fee, subaccTime ,result,m_fund_sp_config.Fcurtype);
           m_subAccDrawOk = true;
	}
	catch(CException& e)
	{
		// �ӳ�ȷ�����Ͳ������ֱ�Ӓ����쳣
		if(m_fund_sp_config.Fbuy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)	
		{
			throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�
		}

		// ���Ҫ��������
		// ���������˻���Ǯ����10����û�ɹ��ĸ澯���ڲ����������ǲ���������ⲿ���ܲ�����10����û�ɹ����쳣ʱ���ᴥ���澯
		if(payNotifyOvertime(subaccTime))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.���ȷ�ϲ�������10�������˻���δ����ɹ�[%s]",m_stTradeBuy.Flistid);		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// ����Ϣ�����
		callErrorRpc(m_request, gPtrSysLog);

		throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�
		
	}
	
}

void AbstractRedeemSpAck::updateCkvs()
{
	//�����û�����CKV, ��ظ���ʧ�ܲ����쳣
	if(m_balanceChanged)
	{
		//setFundTotalaccToKV(m_fundUserTotalAcc);
		// updateUserAcc(m_stTradeBuy); ��ҳ�ֲ�ɾ��,�������������
	}
}

void AbstractRedeemSpAck::updateExauAuthLimitNoExcp()
{
	// ���û�仯,�����и���
	if(!m_balanceChanged)
	{
		return;
	}
	// T+0  ������ز��޶�
	if (m_stTradeBuy.Floading_type == 0)
	{
		return;
	}

	//�ۼ�exau ���������⣬���ⳬʱ���µ�����ع�
	//��������ʱ����exau���ƣ��ۼ�ʱʧ�ܲ������������ۼӣ�����ص��޷�����ɹ�������
	try
	{
		//�ۼ��û�����޶�
		int redem_type = (m_stTradeBuy.Floading_type == 0?DRAW_ARRIVE_TYPE_T1:DRAW_ARRIVE_TYPE_T0);
		updateExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_params.getLong("total_fee"),m_fund_bind.Fcre_id,redem_type);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
	}
}


void AbstractRedeemSpAck::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	char szMsg[MAX_MSG_LEN + 1] = {0};

    // ��װ�ؼ�����
    CUrlAnalyze::setParam(szMsg, "Flistid", fundTradeBuy.Flistid, true);
    CUrlAnalyze::setParam(szMsg, "Fspid", fundTradeBuy.Fspid);
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
void AbstractRedeemSpAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());

	int loading_type = m_stTradeBuy.Floading_type;
    if (m_params.getInt("fetch_type") != 4) // t1 ���طſ�ǰ��֮ǰ����ʱt1�������ظñ�Ǹ�itg�ӳ�����
    {
        if (m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA)
        {
            CUrlAnalyze::setParam(rqst->odata, "fetch_type", 5); //��ص����ͨ���
        }
        else
        {
            CUrlAnalyze::setParam(rqst->odata, "fetch_type", (loading_type==DRAW_USE_LOADING?DRAW_ARRIVE_TYPE_T0:DRAW_ARRIVE_TYPE_T1));
        }
    }
    else
    {
        CUrlAnalyze::setParam(rqst->odata, "fetch_type",4);
    }
    CUrlAnalyze::setParam(rqst->odata, "loading_type", loading_type);

    rqst->olen = strlen(rqst->odata);
    return;
}


