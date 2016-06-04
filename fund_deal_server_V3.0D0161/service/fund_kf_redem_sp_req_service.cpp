/**
  * FileName: fund_redem_sp_req_service.cpp
  * Author: rajeszhou
  * Version :1.0
  * Date: 2014-8-19
  * Description: �����׷��� ����ͷ�ǿ��������� Դ�ļ�
  */

#include "fund_commfunc.h"
#include "fund_kf_redem_sp_req_service.h"

FundKfRedemSpReq::FundKfRedemSpReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fund_close, 0, sizeof(FundCloseTrans));

	m_bBuyTradeExist = false;

}

/**
  * service step 1: �����������
  */
void FundKfRedemSpReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	m_request = rqst;
    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_kf_sp_req_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_fetch_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_charge_ctrl_id", 0, 32);
	m_params.readStrParam(szMsg, "sp_fetch_id", 0, 32);
	//m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
     m_params.readIntParam(szMsg, "fetch_type", 0,DRAW_ARRIVE_TYPE_T1);
	//m_params.readStrParam(szMsg, "transfer_id", 1, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
    //m_params.readStrParam(szMsg, "fund_name", 0, 64);
    m_params.readStrParam(szMsg, "fund_code", 1, 64);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "purpose", 0,4);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "end_date", 1, 16);
	m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	if (!IsTimeLimt(SetTodayTime(gPtrConfig->m_AppCfg.kf_start_time), SetTodayTime(gPtrConfig->m_AppCfg.kf_end_time)))
	{
		throw EXCEPTION(ERR_KF_ONLY_TINE, "�ͷ�����ֻ�����ڹ̶�ʱ���ڷ���"); 
	}

	if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T0)
	{
	   throw CException(ERR_KF_ONLY_T0_REDEM, "�ͷ�����ֻ��T+0.", __FILE__, __LINE__);
	}


}

/*
 * ���ɻ���ע����token
 */
string FundKfRedemSpReq::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����uid|cft_bank_billno|spid|total_fee|key
    // ��������ԭ��
    ss << m_params["uid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundKfRedemSpReq::CheckToken() throw (CException)
{
	// ����token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}


/**
  * ����������ȡ�ڲ�����
  */
void FundKfRedemSpReq::CheckParams() throw (CException)
{
    // ��֤token
    CheckToken();

	//���spid ��fund_code �Ƿ���Ч
	strncpy(m_fund_sp_config.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_sp_config.Fspid) - 1);
	strncpy(m_fund_sp_config.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_sp_config.Ffund_code) - 1);
	checkFundSpAndFundcode(m_pFundCon,m_fund_sp_config, false);//��ǿ�Ʊ�������Ч����˾���û��ѿ�ͨ�˸û���˾����ʹ�ô����ƵĻ���˾

	//����Ƿ�֧����ص��Ƹ�ͨ���
	if(m_params.getInt("purpose") == PURPOSE_DEFAULT && !(gPtrConfig->m_AppCfg.support_redem_to_cft == 1))
	{
		throw CException(ERR_CANNOT_REDEM_TO_CFT, "not support redeem to cft balance.", __FILE__, __LINE__);
	}

    if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1)
    {
        m_params.setParam("fetch_type", DRAW_ARRIVE_TYPE_T0);
    }
    /*if (m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T0 && 0 == m_params.getInt("bank_type"))
    {
        throw CException(ERR_BAD_PARAM, "ȱ�ٲ��� bank_type.", __FILE__, __LINE__);
    }*/

	/*if(m_fund_sp_config.Fclose_flag != CLOSE_FLAG_NORMAL)
	{	
		//���ڲ�Ʒ��ʱ�������
		throw EXCEPTION(ERR_BAD_PARAM, "Do not support the redemption of the Fund's");    
	}*/
}


/**
  * ִ���깺����
  */
void FundKfRedemSpReq::excute() throw (CException)
{
	/* ����1: ��������������ص� */
	if ( CreatTradeInfo() )
	{
		/* ����2:  ������ز���*/
		ProcessTradeInfo();
	}
}

bool FundKfRedemSpReq::CreatTradeInfo()
{
	try
    {
        CheckParams();
		
        /* ������󶨼�¼ */
        CheckFundBind();

		/* �������˻��󶨻���˾�����˻���¼ */
		CheckFundBindSpAcc();

         /* �������� */
        m_pFundCon->Begin();

		/** ��ѯ��ؽ��׼�¼ 
		* Ϊ��֧����ز������룬�����Ȳ�ѯ���Ƿ����
		*/
        CheckFundTrade();

		/* ��ص������ڣ������������ */
		if (!m_bBuyTradeExist)
		{
			//��cgi �����ʱ��飬�������Ҳ�޷���ȫ��������
			//�޷ݶ����˾�����ʧ�ܣ���ʱ����˾�������������ȷ��ʱ�����˻����Ҳ��ʧ��
			CheckFundBalance();

			/* �����ص����˻���ȣ������µ��ʶ��(��Ϊ��û��ʼ�������) */
			checkSpLoaning();

			/*������ص�����  */
			GenerFundTrade();		

			/* ��¼��ؽ��׼�¼ */
			RecordFundTrade();

			UpdateFundCloseListid();
		}

        /* �ύ���� */
        m_pFundCon->Commit();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }

		/* �����ʱ����ִ������2 */
		return false;
    }

	return true;
}


void FundKfRedemSpReq::ProcessTradeInfo()
{
	try
    {        
        /* ������󶨼�¼ */
        CheckFundBind();

         /* �������� */
        m_pFundCon->Begin();

		/* ������� */
        CheckFundTrade();

		if(!m_bBuyTradeExist)
	{
		throw CException(ERR_SYSTEM_UNKNOW_ERROR, "fund redem record not exist,error! ", __FILE__, __LINE__);
	}

		QueryFundCloseId();

		/* �����ض�ȣ���������ض�� */
		checkSpLoaningPlus();

		/* �����˻���������״̬ */
		RecordRedemTradeForSuc();

        /* �ύ���� */
        m_pFundCon->Commit();

		/* ����ckv */
		updateCkvs();

		/* ������ؽ������ */
		updateExauAuthLimitNoExcp();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }
    }
}

void FundKfRedemSpReq::updateCkvs()
{
	//���¶��ڽ��׼�¼��ckv
	setFundCloseTransToKV(m_fund_bind.Ftrade_id, m_params.getString("fund_code"));
	//���½��׼�¼��ckv
	// �˴�ckv���±�������UpdateFundTrade��
	//setTradeRecordsToKV(m_pFundCon, m_stRecord);

}

void FundKfRedemSpReq::updateExauAuthLimitNoExcp()
{
	//�ۼ�exau ���������⣬���ⳬʱ���µ�����ع�
	//��������ʱ����exau���ƣ��ۼ�ʱʧ�ܲ������������ۼӣ�����ص��޷�����ɹ�������
	try
	{
		//�ۼ��û�����޶�
		int redem_type = DRAW_ARRIVE_TYPE_T0;
		updateExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_real_fee,m_fund_bind.Fcre_id,redem_type);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
	}
}

static bool payNotifyOvertime(string pay_suc_time)
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

void FundKfRedemSpReq::doDraw() throw (CException)
{
	gPtrAppLog->debug("doDraw, listid[%s]  ", m_stTradeBuy.Fsub_trans_id);

	try
	{
		TRACE_DEBUG("[SubaccDraw][spid:%s][Fqqid:%s][client_ip:%s][Fsub_trans_id:%s][total_fee:%ld][acc_time:%s]",
				m_params.getString("spid").c_str(), m_fund_bind.Fqqid, m_params.getString("client_ip").c_str(),
				m_stTradeBuy.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeBuy.Facc_time);
		//TODO: �ǵ�ȥ��ע��Not enough balance on account!
	    SubaccDraw(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
		m_stTradeBuy.Fsub_trans_id, m_total_fee, m_stTradeBuy.Facc_time);
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


/**
 * �������״̬
 * @return 
 */
void FundKfRedemSpReq::UpdateState()
{
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	stRecord.Fstate = REDEM_SUC;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
    stRecord.Fpurpose = m_stTradeBuy.Fpurpose;
    stRecord.Floading_type = m_stTradeBuy.Floading_type;
    SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
	UpdateFundCloseState();
}

/**
*	������˾��سɹ�����
*	�������˻����
*/
void FundKfRedemSpReq::RecordRedemTradeForSuc() throw (CException)
{
	UpdateState();
	
	doDraw();

	//���׼�¼��MQ,��װ�仯�Ĳ���	
	sendFundBuy2MqMsg(m_stTradeBuy);
}

void FundKfRedemSpReq::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
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


/*
 * ��ѯ�����˻��Ƿ���ڣ��Լ���֤������һ����
 */
void FundKfRedemSpReq::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }

	// ��¼���ڣ�������¼�е�trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
}

/*
*����Ƿ�󶨻���˾�ʺ�
*/
void FundKfRedemSpReq::CheckFundBindSpAcc() throw (CException)
{
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	//��ز������Ʊ������������ʺţ�ֻ�����깺
	//queryValidMasterSpAcc(m_pFundCon, m_fund_bind_sp_acc,m_params.getString("spid"), false);
	queryValidFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false);

}

/**
  * �������׼�¼�Ƿ��Ѿ�����
  */
void FundKfRedemSpReq::CheckFundTrade() throw (CException)
{
    // û����ؼ�¼��������һ��
    m_bBuyTradeExist = QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		0, &m_stTradeBuy, true);

    gPtrAppLog->debug("fund buy req trade record exist : %d", m_bBuyTradeExist);

	if(!m_bBuyTradeExist)
	{
		return;
	}

	m_params.setParam("fund_trans_id", m_stTradeBuy.Flistid);
	
	// ���ؼ�����
	if( (0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str())))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}


	if(0 != strcmp(m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str()))
	{
		gPtrAppLog->error("fund trade exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fund_code is different!", __FILE__, __LINE__);
	}

	//�����total_fee������صı���
	if(m_stTradeBuy.Freal_redem_amt!= m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, total_fee is different!", __FILE__, __LINE__);
	}

	if(0 != strcmp(m_stTradeBuy.Fcft_trans_id, m_params.getString("cft_trans_id").c_str()))
	{
		gPtrAppLog->error("fund trade exists, cft_trans_id is different! cft_trans_id in db[%s], cft_trans_id input[%s] ", 
			m_stTradeBuy.Fcft_trans_id, m_params.getString("cft_trans_id").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, cft_trans_id is different!", __FILE__, __LINE__);
	}
    
    if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T0 || m_stTradeBuy.Floading_type != DRAW_USE_LOADING)
    {
            gPtrAppLog->error("fund trade exists, Floading_type is confict! Floading_type in db[%d], fetch_type input[%s] ", 
    			m_stTradeBuy.Floading_type, m_params.getString("fetch_type").c_str());
            throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fetch_type is confict!", __FILE__, __LINE__);
    }


	if(m_params.getInt("uid") != 0 && m_stTradeBuy.Fuid!=0 && m_params.getInt("uid") != m_stTradeBuy.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_stTradeBuy.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}
    
	// ��¼���ڣ�����״̬��Ч������
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			m_stTradeBuy.Flistid, m_stTradeBuy.Ftrade_id, m_stTradeBuy.Fpur_type);
		throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
	}

    if(m_stTradeBuy.Fstate == REDEM_ININ )
    {
		//�������룬�����쳣������ִ��
		//���ܴ��ڿ����˻�ʧ��
		return;
    }
	else if(m_stTradeBuy.Fstate == REDEM_SUC)
	{
		//����
		throw CException(ERR_REPEAT_ENTRY, "fund redem from sp success. ", __FILE__, __LINE__);
	}
	else
    {
		//����״̬����ز���������
		throw CException(ERR_REDEM_REPEAT_ENTRY, "fund redem trade exist. ", __FILE__, __LINE__);
    }
}

void FundKfRedemSpReq::QueryFundCloseId()
{
	strncpy(m_fund_close.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_close.Ftrade_id) - 1);
	strncpy(m_fund_close.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fund_close.Ftrade_id) - 1);
	strncpy(m_fund_close.Fend_date, m_params.getString("end_date").c_str(), sizeof(m_fund_close.Fend_date) - 1);

	bool ret = queryFundCloseTransByEndDate(m_pFundCon, m_fund_close, true);

	if (ret) 
	{
		//���bank_billno
		if(strlen(m_fund_close.Fsell_listid) != 0 && 
			strncmp(m_stTradeBuy.Flistid, m_fund_close.Fsell_listid, sizeof(m_fund_close.Fsell_listid)) != 0)
		{
			throw CException(ERR_FUND_CLOSE_REQ_FAIL, "sell_listid already exist.", __FILE__, __LINE__);		
		}
		
		m_real_fee = m_fund_close.Fstart_total_fee;
		m_total_fee = m_fund_close.Fcurrent_total_fee;
		m_params.setParam("fund_close_fid", m_fund_close.Fid);
	}
	else 
	{
		throw CException(ERR_FUND_CLOSE_REQ_FAIL, "fund close query error.", __FILE__, __LINE__);
	}

	if(m_real_fee != m_params.getLong("total_fee"))
	{
		TRACE_DEBUG("[���ڱ���:%ld][������:%ld]",m_real_fee, m_params.getLong("total_fee"))
		throw CException(ERR_FUND_CLOSE_BALANCE_NOT_EQ, "��������ڱ���", __FILE__, __LINE__);
	}
}

/**
* ��ѯ���ͨ�˻����
* ��ѯ���ڻ��𱾽��Ƿ��total feeһ��
*/
void FundKfRedemSpReq::CheckFundBalance()
{
	//TODO ���������ĸ��ֶ�,���˻�ֻ�������ˣ���ʱ�����ڶ��Ჿ��
	//TODO �ж����ʽ������ݶ�ת��	
	strncpy(m_fund_close.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_close.Ffund_code) - 1);
	strncpy(m_fund_close.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fund_close.Ftrade_id) - 1);
	strncpy(m_fund_close.Fend_date, m_params.getString("end_date").c_str(), sizeof(m_fund_close.Fend_date) - 1);

	bool ret = queryFundCloseTransByEndDate(m_pFundCon, m_fund_close, true);

	if (ret)
	{
		//���bank_billno
		if(strlen(m_fund_close.Fsell_listid) != 0)//ԭ����������ص�����Ҫ�ж������뻹�����·����,��������·���ľ;ܾ�
		{
			ST_TRADE_FUND m_stTrade;
			if(!QueryTradeFund(m_pFundCon, m_fund_close.Fsell_listid, PURTYPE_REDEEM, &m_stTrade, true))//֮ǰ��¼����ص�������
			{
				gPtrAppLog->error("buy record not exist, listid[%s]  ", m_fund_close.Fsell_listid);
				throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
			}

			if(strncmp(m_stTrade.Fcft_bank_billno, m_params.getString("cft_bank_billno").c_str(), sizeof(m_stTrade.Fcft_bank_billno)) != 0)
			{
				throw CException(ERR_FUND_CLOSE_REQ_FAIL, "sell_listid already exist.", __FILE__, __LINE__);		
			}
		}
		m_real_fee = m_fund_close.Fstart_total_fee;
		m_total_fee = m_fund_close.Fcurrent_total_fee;

		m_params.setParam("fund_close_fid", m_fund_close.Fid);
	}
	else 
	{
		throw CException(ERR_FUND_CLOSE_REQ_FAIL, "fund close query error.", __FILE__, __LINE__);
	}
	if (m_fund_close.Fstate != CLOSE_FUND_STATE_PENDING)
	{
		throw CException(ERR_FUND_CLOSE_REDEM_SUC, "�û���״̬���ܱ�ΥԼ���", __FILE__, __LINE__);
	}
	
	if(m_real_fee != m_params.getLong("total_fee"))
	{
		TRACE_DEBUG("[���ڱ���:%ld][������:%ld]",m_real_fee, m_params.getLong("total_fee"))
		throw CException(ERR_FUND_CLOSE_BALANCE_NOT_EQ, "��������ڱ���", __FILE__, __LINE__);
	}

	string max_trade_date=getCacheCloseMaxTradeDate(gPtrFundSlaveDB, string(m_fund_close.Ftrans_date), m_params.getString("fund_code"));
	
	if (atoi(max_trade_date.c_str())>=GetDateToday())
	{
		throw CException(ERR_FUND_CLOSE_FOCRE_STATE, "���ڻ���ݶ�δȷ������,����ǿ�����", __FILE__, __LINE__);
	}
	
	if (atoi(m_fund_close.Fbook_stop_date)<=GetDateToday())
	{
		throw CException(ERR_FUND_CLOSE_FOCRE_TOO_LATE, "���ڻ������Ͻ���,����ǿ�����", __FILE__, __LINE__);
	}	

}

/**
* �����ص����˻����
*/
void FundKfRedemSpReq::checkSpLoaning() throw (CException)
{
	//�������Ϊ���ֲ��ۼ���ض��

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));
	
	strncpy(fundSpConfig.Fspid, m_params.getString("spid").c_str(), sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//��Ӧ�÷���
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}


    if ((fundSpConfig.Fredem_valid&0x07) ==2) // ֹͣ���
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
    
    //�����˻���ȳ��޵Ĵ���
    checkRedemOverLoading(m_pFundCon, fundSpConfig, m_params.getLong("total_fee"),true);

}

/**
* �����ص����˻����,���ۼӶ��
*/
void FundKfRedemSpReq::checkSpLoaningPlus() throw (CException)
{
	//�������Ϊ���ֲ��ۼ���ض��

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));
	
	strncpy(fundSpConfig.Fspid, m_params.getString("spid").c_str(), sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//��Ӧ�÷���
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}


    if ((fundSpConfig.Fredem_valid&0x07) ==2) // ֹͣ���
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
    
    //�����˻���ȳ��޵Ĵ���
    checkRedemOverLoading(m_pFundCon, fundSpConfig, m_params.getLong("total_fee"),true);

	//��������ۼƶ��,t+1������ز��ۼƣ�����������ͨ��ء����ѡ�t+0��ض�Ҫ�ۼ�
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    LONG tmp_total_fee = m_params.getLong("total_fee");
    updateFundSpRedomTotal(m_pFundCon, fundSpConfig, tmp_total_fee,m_stTradeBuy.Floading_type,m_stTradeBuy.Facc_time);
	
}

void FundKfRedemSpReq::GenerFundTrade()
{
	memset(&m_stRecord, 0, sizeof(ST_TRADE_FUND));

	string drawid = genSubaccDrawid();
	string cft_bank_billno = m_params.getString("cft_bank_billno");
	//��ص����ڲ�����  10�̻���+8λ����+10���к�+cft_bank_billno��3λ����֤cft_bank_billno��listid�ķֿ�ֱ����һ�£������ڲ�ֱ�
	string listid =  m_params.getString("spid") + drawid + cft_bank_billno.substr(cft_bank_billno.size()-3);
	m_params.setParam("fund_trans_id", listid);

    strncpy(m_stRecord.Flistid, listid.c_str(), sizeof(m_stRecord.Flistid)-1);
    strncpy(m_stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(m_stRecord.Fspid)-1);
    strncpy(m_stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(m_stRecord.Fcoding)-1);
    strncpy(m_stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_stRecord.Ftrade_id)-1);
    m_stRecord.Fuid = m_params.getInt("uid");
    strncpy(m_stRecord.Ffund_name, m_fund_sp_config.Ffund_name, sizeof(m_stRecord.Ffund_name)-1);
    strncpy(m_stRecord.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_stRecord.Ffund_code)-1);
    m_stRecord.Fbank_type = 0;
    strncpy(m_stRecord.Fcard_no, m_params.getString("card_no").c_str(), sizeof(m_stRecord.Fcard_no)-1);
    m_stRecord.Fpur_type = PURTYPE_REDEEM;
	//total fee = ȡ�����ڴεı���+����
	//Freal_redem_amt = ȡ�����ڴεı���
    m_stRecord.Ftotal_fee = m_total_fee;
	m_stRecord.Freal_redem_amt = m_real_fee;
	m_stRecord.Fspe_tag = 0;//��ʱ�����ɹ���Ҫ����ʱ״̬�޸ģ������²�ͣ����
    m_stRecord.Fstate = REDEM_ININ;
    m_stRecord.Flstate = LSTATE_VALID;
	m_stRecord.Fopt_type = KF_REDEM;
	m_stRecord.Fclose_listid = m_params.getLong("fund_close_fid");
	strncpy(m_stRecord.Fend_date, m_params.getString("end_date").c_str(), sizeof(m_stRecord.Fend_date) - 1);
	
    strncpy(m_stRecord.Ffetchid, m_params.getString("cft_fetch_id").c_str(), sizeof(m_stRecord.Ffetchid)-1);
    m_stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(m_stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Fcreate_time)-1);
    strncpy(m_stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Fmodify_time)-1);
    //�Ƹ�ͨ�����Ƿ�֧������? m_stRecord.Fstandby1 = 1; // ������¼
    m_stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));; // ��������
    strncpy(m_stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Facc_time)-1);
	strncpy(m_stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(m_stRecord.Fcft_trans_id)-1);
	strncpy(m_stRecord.Fcft_charge_ctrl_id, m_params.getString("cft_charge_ctrl_id").c_str(), sizeof(m_stRecord.Fcft_charge_ctrl_id)-1);
	strncpy(m_stRecord.Fsp_fetch_id, m_params.getString("sp_fetch_id").c_str(), sizeof(m_stRecord.Fsp_fetch_id)-1);
	strncpy(m_stRecord.Fcft_bank_billno, m_params.getString("cft_bank_billno").c_str(), sizeof(m_stRecord.Fcft_bank_billno)-1);
	strncpy(m_stRecord.Fsub_trans_id, drawid.c_str(), sizeof(m_stRecord.Fsub_trans_id)-1);
	m_stRecord.Fpurpose = m_params.getInt("purpose");
	strncpy(m_stRecord.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(m_stRecord.Fchannel_id)-1);

    m_stRecord.Floading_type = DRAW_USE_LOADING;
}

/**
  * ���ɻ�����ؼ�¼��״̬: ��ʼ���״̬
  */
void FundKfRedemSpReq::RecordFundTrade()
{
    InsertTradeFund(m_pFundCon, &m_stRecord);
    InsertTradeUserFund(m_pFundCon, &m_stRecord);

	//��¼�����ʺ����ֵ��ͻ����׵���Ӧ��ϵ
	if(!m_params.getString("sp_fetch_id").empty())
	{
		FundFetch fundFetch;
		memset(&fundFetch, 0, sizeof(FundFetch));
		
		strncpy(fundFetch.Ffetchid, m_params.getString("sp_fetch_id").c_str(), sizeof(fundFetch.Ffetchid)-1);
		strncpy(fundFetch.Ffund_trans_id, m_stRecord.Flistid, sizeof(fundFetch.Ffund_trans_id)-1);
		strncpy(fundFetch.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fcreate_time)-1);
    	strncpy(fundFetch.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fmodify_time)-1);
		

		insertFundFetch(m_pFundCon, fundFetch);
	}

	
}


/**
 * ���¶���״̬Ϊ�ɹ�
 * @return 
 */
void FundKfRedemSpReq::UpdateFundCloseState()
{
	//���¶��ڲ�Ʒ״̬
	FundCloseTrans fund_close_set;
	fund_close_set.Fid = m_params.getLong("fund_close_fid");
	fund_close_set.Fcurrent_total_fee = 0;
	fund_close_set.Fstate = CLOSE_FUND_STATE_SUC;
	fund_close_set.Fuser_end_type = KF_REDEM;

	//strncpy(fund_close_set.Fsell_listid, m_params.getString("fund_trans_id").c_str(), sizeof(fund_close_set.Fsell_listid)-1);
	strncpy(fund_close_set.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fund_close_set.Fmodify_time)-1);
	strncpy(fund_close_set.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(fund_close_set.Ftrade_id)-1);
	
	saveFundCloseTrans(fund_close_set,m_fund_close,m_params.getString("fund_trans_id").c_str(),PURTYPE_REDEEM);
}

void FundKfRedemSpReq::UpdateFundCloseListid()
{
	FundCloseTrans closeTrans;

	closeTrans.Fid=m_params.getLong("fund_close_fid");
	strncpy(closeTrans.Fsell_listid, m_params.getString("fund_trans_id").c_str(), sizeof(closeTrans.Fsell_listid)-1);
	strncpy(closeTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(closeTrans.Fmodify_time)-1);
	strncpy(closeTrans.Ftrade_id, m_stRecord.Ftrade_id, sizeof(closeTrans.Ftrade_id)-1);
	
	updateFundCloseTransById(gPtrFundDB,closeTrans);
}


/**
  * ����������
  */
void FundKfRedemSpReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "acc_time", m_bBuyTradeExist ? m_stTradeBuy.Facc_time : m_params.getString("systime").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
	CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
	CUrlAnalyze::setParam(rqst->odata, "fund_trans_id", m_params.getString("fund_trans_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "spid", m_params.getString("spid").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_name", m_fund_sp_config.Fsp_name);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_params.getString("fund_code").c_str());
	CUrlAnalyze::setParam(rqst->odata, "fund_name", m_fund_sp_config.Ffund_name);
	CUrlAnalyze::setParam(rqst->odata, "cur_total_fee", m_total_fee);

    rqst->olen = strlen(rqst->odata);
    return;
}
