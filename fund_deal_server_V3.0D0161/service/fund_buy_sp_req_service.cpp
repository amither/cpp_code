/**
  * FileName: fund_reg_user_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-17
  * Description: �����׷��� �����깺 Դ�ļ�
  */

#include "fund_commfunc.h"
#include "fund_buy_sp_req_service.h"

FundBuySpReq::FundBuySpReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));

    m_bRepeatEntry = false;
    m_bBuyTradeExist = false;
}

/**
  * service step 1: �����������
  */
void FundBuySpReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_buy_sp_req_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "uin", 1, 64);
	m_params.readStrParam(szMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
    //m_params.readStrParam(szMsg, "fund_name", 0, 64);
    m_params.readStrParam(szMsg, "fund_code", 0, 64);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "purpose", 0, 101);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

    //�깺����֧��ȯ���ѣ�����������ڼ�¼��ֵȯ��������¼��Fstandby4��varchar64 �Ǳ���
    m_params.readStrParam(szMsg, "coupon_id", 0, 32);

}

/*
 * ���ɻ���ע����token
 */
string FundBuySpReq::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����uid|fund_trans_id|spid|sp_billno|total_fee|key
    // ��������ԭ��
    ss << m_params["uid"] << "|" ;
    ss << m_params["fund_trans_id"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["sp_billno"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundBuySpReq::CheckToken() throw (CException)
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
void FundBuySpReq::CheckParams() throw (CException)
{
    // ��֤token
    CheckToken();

	if(m_params.getString("fund_trans_id").substr(0,10) != m_params.getString("spid"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input fund_trans_id error"); 
	}

	//���spid ��fund_code �Ƿ���Ч
	strncpy(m_fund_sp_config.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_sp_config.Fspid) - 1);
	strncpy(m_fund_sp_config.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_sp_config.Ffund_code) - 1);
	checkFundSpAndFundcode(m_pFundCon,m_fund_sp_config, false);//��ǿ�Ʊ�������Ч����˾���û��ѿ�ͨ�˸û���˾����ʹ�ô����ƵĻ���˾
	m_params.setParam("fund_code",m_fund_sp_config.Ffund_code);//����û����fund_code��ѡ��һ��

    //��鼰����channel_id
    CheckChannelId();
}

/**
  * ִ���깺����
  */
void FundBuySpReq::excute() throw (CException)
{
    try
    {
        CheckParams();

        /* �������˻���¼ */
        CheckFundBind();

		/* �������˻��󶨻���˾�����˻���¼ */
		CheckFundBindSpAcc();

		//����û����зݶ�
		checkUserTotalShare();

		/* �������Ƿ���Խ����깺 */
		checkFundcodePurchaseValid(m_fund_sp_config.Fbuy_valid);

         /* �������� */
        m_pFundCon->Begin();
		 
		/* ��ѯ�����׼�¼ */
        CheckFundTrade();

        /* ����޶� �����ڲ�ѯ�����׵�֮����Ҫ�����Ƿ���ڻ����׵��Լ�uidΪ0�ж��Ƿ���
        ȫ���û�֧����Ŀ���������ǣ�������Ҫ�Ź�*/
        if ((m_bBuyTradeExist==false ||  m_stTradeBuy.Fuid != 0) && m_params.getInt("purpose") != 101) //�����깺������޶�
        {
        	checkFundcodeToScopeUpperLimit(m_fund_bind.Fuid,m_params["systime"],m_params.getLong("total_fee"), m_fund_sp_config, true);
        }

		/* ����û��Ƿ���Թ�������Ʋ�Ʒ */
		checkPermissionBuyCloseFund(m_fund_bind.Ftrade_id, m_fund_sp_config, m_params.getString("systime"), true);

        /* ��¼�����׼�¼ */
        RecordFundTrade();

        /* �ύ���� */
        m_pFundCon->Commit();

		/* ��¼�ɹ������û���״̬�жϷ���ֵ*/
		checkUserTempFail();
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

/*
 * ��ѯ�����˻��Ƿ����
 */
void FundBuySpReq::CheckFundBind() throw (CException)
{
	bool bind_exist;
	//�������û��״��깺��Ȩ�ɹ���֧��ʧ��(��Ȳ��㣬����������)���û�������Ϣ����û��uid
	if(!m_params.getString("uin").empty())
	{
		bind_exist = QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false);	
	}else
	{
		bind_exist = QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false);
	}
	
	if(!bind_exist)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
    else
    {
        if(m_fund_bind.Flstate == LSTATE_FREEZE)
        {
            throw CException(ERR_ALREADY_FREEZE, "the user be frozen! ", __FILE__, __LINE__);
        }
    }

	// ��¼���ڣ�������¼�е�trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
}

/*
*����Ƿ�󶨻���˾�ʺţ����ҿɽ���
*/
void FundBuySpReq::CheckFundBindSpAcc() throw (CException)
{
	
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	bool existsBindSp = queryFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false);
	if(!existsBindSp){		
		TRACE_ERROR("the fund bind sp account record not exist.spid:%s", m_params.getString("spid").c_str());
        throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not exist");
	}

	if(LSTATE_FREEZE == m_fund_bind_sp_acc.Flstate){
		//�˻������᲻�������
		TRACE_ERROR("the fund bind sp account record has been frozen.");
        throw EXCEPTION(ERR_SP_ACC_FREEZE, "the fund bind sp account record has been frozen.");
	}

	if(BIND_SPACC_SUC != m_fund_bind_sp_acc.Fstate&&BIND_SPACC_TEMP_FAIL != m_fund_bind_sp_acc.Fstate){
		//�˻�δ�󶨲�������:�˴��������ʱʧ�ܵ��û����ɽ��׵�,֧���ص�ʱ������˿�
		TRACE_ERROR("the fund bind sp account record not allowed buy.");
        throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record state not allowed buy.");
	}
		
}


/**
  * �������׼�¼�Ƿ��Ѿ�����
  */
void FundBuySpReq::CheckFundTrade() throw (CException)
{
    // û�й����¼��������һ��
    m_bBuyTradeExist = QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), 
		PURTYPE_BUY, &m_stTradeBuy, true);

    gPtrAppLog->debug("fund buy req trade record exist : %d", m_bBuyTradeExist);

    if(!m_bBuyTradeExist)
        return;

    // ���ؼ�����
    if( (0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_stTradeBuy.Ftrade_id, "") 
		&& 0 != strcmp(m_stTradeBuy.Ftrade_id, m_params.getString("trade_id").c_str()))
    {
        gPtrAppLog->error("fund trade exists, trade_id is different! trade_id in db[%s], trade_id input[%s] ", 
			m_stTradeBuy.Ftrade_id, m_params.getString("trade_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, trade_id is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str()))
    {
        gPtrAppLog->error("fund trade exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fund_code is different!", __FILE__, __LINE__);
    }

    if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, total_fee is different!", __FILE__, __LINE__);
    }

    // ��¼���ڣ�����״̬��Ч������
    if(LSTATE_INVALID == m_stTradeBuy.Flstate)
    {
        gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			m_stTradeBuy.Flistid, m_stTradeBuy.Ftrade_id, m_stTradeBuy.Fpur_type);
        throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
    }

    // ��¼�Ѿ�������������
    if (m_stTradeBuy.Fstandby1 != 0)
    {
        throw EXCEPTION(ERR_BUYPAY_LOCK, "the trade exists and locked");
    }

	// ��¼�������Ѿ�������˾Ԥ�깺�ɹ�
    if(PAY_INIT <= m_stTradeBuy.Fstate && m_stTradeBuy.Fstate <= PURCHASE_SUC)
    {
        throw CException(ERR_BUY_REQ_SP_OK, "the state in db is newer. ", __FILE__, __LINE__);
    }

	// �ǳɹ�״̬�Ҳ�Ϊ��ʼ״̬�Ķ����������봴��
	if(CREATE_INIT != m_stTradeBuy.Fstate)
    {
        throw CException(ERR_BUY_RECORD_INVALID, "fund trade record state invalid. ", __FILE__, __LINE__);
    }

    m_bRepeatEntry = true;
	// �����������û�״̬����ֵ
	checkUserTempFail();
}

/**
  * ���ɻ������¼��״̬: �ȴ�����
  */
void FundBuySpReq::RecordFundTrade()
{
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

    if (m_bRepeatEntry)
    {
		if(0 == strcmp("", m_stTradeBuy.Ftrade_id))
		{
			//�����깺��û��trade_id�ģ�Ҫ�ڴ˴�����
			//�����깺�����ڵĳ���:��ʵ����֤�û�֧���м�Ȩ���������ʱ�깺���ȴ������������˻���û�д���
			UpdateFundTradeForReq();
			return;//��������׳��쳣���ᵼ���������±��ع�
		}
		else
		{
	        // ��¼�Ѵ���
	        throw EXCEPTION(ERR_REPEAT_ENTRY, "the trade exists");
		}
    }

    strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(stRecord.Fspid)-1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    stRecord.Fuid = m_fund_bind.Fuid;
    strncpy(stRecord.Ffund_name, m_fund_sp_config.Ffund_name, sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(stRecord.Ffund_code)-1);
    stRecord.Fbank_type = m_params.getInt("bank_type");
    strncpy(stRecord.Fcard_no, m_params.getString("card_no").c_str(), sizeof(stRecord.Fcard_no)-1);
    
	//�����깺
	if(m_params.getInt("purpose") ==100)
	{
		//stRecord.Fpur_type = PURTYPE_REWARD_PROFIT;
		throw EXCEPTION(ERR_BAD_PARAM, "input purpose error");   //�Ȳ�֧����������
	}else if(m_params.getInt("purpose") ==101)
	{
		stRecord.Fpur_type = PURTYPE_REWARD_SHARE;
		
	}
       else if (m_params.getInt("purpose") ==PURPOSE_BALANCE_BUY)
       {
           stRecord.Fpur_type = PURTYPE_TRANSFER_PURCHASE;
       }
	else
	{
		stRecord.Fpur_type = PURTYPE_PURCHASE;
	}
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = CREATE_INIT;
    stRecord.Flstate = LSTATE_VALID;
    strncpy(stRecord.Ftrade_date, m_params.getString("trade_date").c_str(), sizeof(stRecord.Ftrade_date)-1);
    strncpy(stRecord.Ffund_value, m_params.getString("fund_value").c_str(), sizeof(stRecord.Ffund_value)-1);
    strncpy(stRecord.Ffund_vdate, m_params.getString("fund_vdate").c_str(), sizeof(stRecord.Ffund_vdate)-1);
    strncpy(stRecord.Ffund_type, m_params.getString("fund_type").c_str(), sizeof(stRecord.Ffund_type)-1);
    strncpy(stRecord.Fnotify_url, m_params.getString("notify_url").c_str(), sizeof(stRecord.Fnotify_url)-1);
    strncpy(stRecord.Frela_listid, "", sizeof(stRecord.Frela_listid)-1);
    //strncpy(stRecord.Fdrawid, "buy_no_drawid", sizeof(stRecord.Fdrawid)-1);
    strncpy(stRecord.Ffetchid, "buy_no_fetchid", sizeof(stRecord.Ffetchid)-1);
    stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    //�Ƹ�ͨ�����Ƿ�֧������? stRecord.Fstandby1 = 1; // ������¼
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid")); // ��������
    /*if(m_params.getInt("purpose") ==1)
    {
	//�ݶ�ת���ļ�¼�깺��;
    	stRecord.Fpurpose= PURPOSE_CHANGE_SP;
    }*/

    stRecord.Fpurpose = m_params.getInt("purpose");

	strncpy(stRecord.Fchannel_id, m_channelId.c_str(), sizeof(stRecord.Fchannel_id)-1);
    stRecord.Fpay_channel = m_payChannel;
	
	//ȯid
	strncpy(stRecord.Fcoupon_id, m_params.getString("coupon_id").c_str(), sizeof(stRecord.Fcoupon_id)-1);

    InsertTradeFund(m_pFundCon, &stRecord);
	if(m_fund_bind.Fuid >= 10000)
	{
    	InsertTradeUserFund(m_pFundCon, &stRecord);
	}
}


/**
* ��ʵ����֤�û��ڿ���ǰ���ȴ���������¼����ȱ��trade_id����ʵ����֤ͨ������ͨ����˻�����trade_id
*/
void FundBuySpReq::UpdateFundTradeForReq()
{
	
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fstate = m_stTradeBuy.Fstate;
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fbank_type = m_params.getInt("bank_type");
	/*
	* �ӳٵ�֧���ɹ��ڼ�¼
	if(m_fund_bind.Fuid != 0)
	{
		stRecord.Fuid = m_fund_bind.Fuid;
		if(0 == m_stTradeBuy.Fuid)
		{
			//uid ������0Ҫ�����û���
			m_stTradeBuy.Fuid = m_fund_bind.Fuid;
			InsertTradeUserFund(m_pFundCon, &m_stTradeBuy);
		}
	}
	*/
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"), (m_fund_bind.Fuid == 0 || m_stTradeBuy.Fuid == 0) ? false : true);
}

void FundBuySpReq::checkUserTempFail() throw (CException)
{
	// ��¼�����ɹ��󷵻ش���
	if(BIND_SPACC_TEMP_FAIL == m_fund_bind_sp_acc.Fstate){
		
    	throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record state not allowed buy.");
	}
}

void FundBuySpReq::checkUserTotalShare() throw (CException)
{
		//����깺�ͻ���Ͷ�������޶�
    if (m_params.getInt("purpose") == PURPOSE_BALANCE_BUY || m_params.getInt("purpose") == PURPOSE_ACTION_BUY)
    {
        return;
    }
      

    LONG currentTotalAsset = queryUserTotalAsset(m_fund_bind.Fuid,m_fund_bind.Ftrade_id);
    if (true == isUserAssetOverLimit(m_fund_bind.Fasset_limit_lev,currentTotalAsset, m_params.getLong("total_fee")))
    {
        //����ʧ�������깺���޶�
        if (true == checkInnerBalancePayForReBuy(gPtrFundDB, m_params.getString("fund_trans_id"), m_params.getString("total_fee"),""))
        {
            return;
        }
        
        TRACE_ERROR("user total Asset Over Limit!");
        throw CException(ERR_FUND_ASSET_OVER_LIMIT, "user total Asset Over Limit! ", __FILE__, __LINE__);
    }
}


/**
  * ����������
  */
void FundBuySpReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
	CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_params.getString("fund_code").c_str());

    rqst->olen = strlen(rqst->odata);
    return;
}

//���������ֶ��е�channel_id������������channel_id��pay_channel(0�����п�   1�����ͨ��� 2: ���� )
//�磬 0_68|fm_3_unknown,  channel_idΪ68|fm_3_unknown, pay_channelΪ0
void FundBuySpReq::CheckChannelId() throw (CException)
{
    const string channelIdOrig = m_params.getString("channel_id");
    m_channelId = channelIdOrig;
    if (m_params.getInt("purpose") == PURPOSE_BALANCE_BUY)
    {
        m_payChannel = PAY_TYPE_BALANCE;
        return;
    }

    if (channelIdOrig.empty())
    {
        m_payChannel = PAY_TYPE_CARD;
        return;
    }

    string str1 = channelIdOrig.substr(0, channelIdOrig.find('|'));
    size_t it = str1.find('_');
    if (it != string::npos)
    {
        m_payChannel = atoi(str1.substr(0, it).c_str());
        if (m_payChannel < PAY_TYPE_CARD || m_payChannel >= PAY_TYPE_END)
        {
            TRACE_ERROR("input channel_id %s , payChannel:%d error", channelIdOrig.c_str(), m_payChannel);
            throw EXCEPTION(ERR_BAD_PARAM, "input channel_id error");   
        }
        m_channelId = channelIdOrig.substr(it+1);
    }
    else
    {
        m_payChannel = PAY_TYPE_CARD;
    }

    TRACE_DEBUG("channelIdOrig: %s, m_channelId: %s, m_payChannel: %d", channelIdOrig.c_str(), m_channelId.c_str(), m_payChannel);
}


