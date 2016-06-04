/**
  * FileName: fund_reg_profit_rate_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-29
  * Description: 基金交易服务 登记基金收益率 源文件
  */

#include "fund_commfunc.h"
#include "fund_reg_profit_rate_service.h"

FundRegProfitRate::FundRegProfitRate(CMySQL* mysql,int para)
{
    m_pFundCon = mysql;
    memset(&m_fundSpConfig, 0, sizeof(FundSpConfig));
	memset(&m_fundProfitRate, 0, sizeof(FundProfitRate));
	m_optype=para;
}

/**
  * service step 1: 解析输入参数
  */
void FundRegProfitRate::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 要保留请求数据，抛差错使用
    m_request = rqst;

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_reg_profit_rate_service] receives: %s", szMsg);

    m_params.readStrParam(szMsg, "spid", 1, 15);
	m_params.readStrParam(szMsg, "fund_code", 1, 64);
    m_params.readStrParam(szMsg, "date", 1, 20);
	m_params.readSignedLongParam(szMsg, "seven_day_profit_rate", MIN_LONG, MAX_LONG);//指数复用为日涨幅
	m_params.readLongParam(szMsg, "day_profit_rate", 0, MAX_LONG);//指数复用为净值

    m_params.readStrParam(szMsg, "client_ip", 0, 16);

    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	/*	F1week_rise_rate '近一个星期涨跌幅 货基;共用区间收益率' a_week_rise_rate,
		F1month_rise_rate '近一月涨跌幅 货基;共用一月区间收益率' a_month_rise_rate,
		F3month_rise_rate '近三月涨跌幅 货基;共用三月区间收益率' three_month_rise_rate,
		F6month_rise_rate '近半年涨跌幅 货基;共用六月区间收益率' six_month_rise_rate,
		F1year_rise_rate '近一年涨跌幅 货基;共用一年区间收益率' a_year_rise_rate,
		Fcumulative_net '累计净值'，

		F1year_track_error '近一年跟踪误差; 共用年化收益率'a_year_annual_profit, a_year_track_error
		F1month_annual_profit '近一月年化收益率 ' a_month_annual_profit,
		F3month_annual_profit '近三月年化收益率 ' three_month_annual_profit,
		F6month_annual_profit '近半年年化收益率 ' six_month_annual_profit;
	*/
	m_params.readSignedLongParam(szMsg, "a_week_rise_rate", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "a_month_rise_rate", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "three_month_rise_rate", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "six_month_rise_rate", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "a_year_rise_rate", MIN_LONG, MAX_LONG);
	m_params.readLongParam(szMsg, "cumulative_net", 0, MAX_LONG);
	
	m_params.readSignedLongParam(szMsg, "a_month_annual_profit", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "three_month_annual_profit", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "six_month_annual_profit", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "a_year_annual_profit", MIN_LONG, MAX_LONG);
	m_params.readSignedLongParam(szMsg, "a_year_track_error", MIN_LONG, MAX_LONG);

	

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

void FundRegProfitRate::checkFundSpConfig()
{
	strncpy(m_fundSpConfig.Fspid, m_params.getString("spid").c_str(), sizeof(m_fundSpConfig.Fspid) - 1);
    strncpy(m_fundSpConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fundSpConfig.Ffund_code) - 1);
    
    if(!queryFundSpAndFundcodeConfig(gPtrFundDB, m_fundSpConfig, false))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
}

/*
 * 生成基金注册用token
 */
string FundRegProfitRate::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照spid|fund_code|date|key
    // 规则生成原串
    ss << m_params["spid"] << "|" ;
    ss << m_params["fund_code"] << "|" ;
    ss << m_params["date"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

	TRACE_DEBUG("fund authen token  input=%s", 
	                ss.str().c_str());

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundRegProfitRate::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}


/**
  * 检查参数，获取内部参数
  */
void FundRegProfitRate::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
	
	// 检查商户信息
	checkFundSpConfig();
	
	 // 来自外部数据,只更新明细收益率
	if(m_optype==OP_TYPE_REG_RATE_FROM_OUTER)
	{
		m_updateBaseRate=false;
		m_updateDetailRate=true;
		
	//来自合作商数据,  更新基础数据。详细数据灰度
	}else
	{
		m_updateBaseRate=true;
		m_updateDetailRate=(m_params.getLong("a_month_annual_profit")>0);
	}

	// 字段转换( 1年年化收益率重用1年跟踪误差)
	if(m_fundSpConfig.Ftype != SPCONFIG_TYPE_ETF)
	{
		m_params.setParam("a_year_track_error",m_params.getLong("a_year_annual_profit"));
	}

	// 更新货币基金的基础数据才检查检查配置收益数值
	if(m_updateBaseRate==false||m_fundSpConfig.Ftype == SPCONFIG_TYPE_ETF)
	{
		return;
	}

	//检查收益数值
	if(m_params.getLong("seven_day_profit_rate") > gPtrConfig->m_AppCfg.seven_day_profit_rate_max 
		|| m_params.getLong("seven_day_profit_rate") < gPtrConfig->m_AppCfg.seven_day_profit_rate_min)
	{
		throw EXCEPTION(ERR_BAD_PARAM, "seven_day_profit_rate invalid");    
	}

}

/**
  * 执行收益入账
  */
void FundRegProfitRate::excute() throw (CException)
{
    try
    {
        CheckParams();

		checkProfitRate();

        regProfitRate();
		
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

void FundRegProfitRate::checkProfitRate() throw (CException)
{
    strncpy(m_fundProfitRate.Fspid, m_params.getString("spid").c_str(), sizeof(m_fundProfitRate.Fspid)-1);
    strncpy(m_fundProfitRate.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fundProfitRate.Ffund_code)-1);
    strncpy(m_fundProfitRate.Fdate, m_params.getString("date").c_str(), sizeof(m_fundProfitRate.Fdate)-1);

	bool hasProfitRate = queryFundProfitRate(m_pFundCon, m_fundProfitRate, true);
	
	if(!hasProfitRate)
	{
		
		// 检查必须先更新基金公司收益率再更新外部收益率
		if(!m_updateBaseRate)
		{
			char errMsg[128]={0};
			snprintf(errMsg, sizeof(errMsg), "只更新明细收益率,必须在基本收益率更新成功后再更新[%s][%s]",m_fundProfitRate.Fspid,m_fundProfitRate.Fdate); 
			throw CException(ERR_REG_OUTER_PROFIT_RATE, errMsg, __FILE__, __LINE__);
		}
		return;
	}

	// 检查基本信息重入
	// 未输入基本收益率,可以不检查
	if(m_fundProfitRate.F1day_profit_rate!= m_params.getLong("day_profit_rate")&&m_params.getLong("day_profit_rate")!=0)
    {
        gPtrAppLog->error("profit record exist, one_day_profit_rate is different! one_day_profit_rate in db[%lld], one_day_profit_rate input[%lld] ", 
			m_fundProfitRate.F1day_profit_rate, m_params.getLong("day_profit_rate"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "profit record exist, one_day_profit_rate is different!", __FILE__, __LINE__);
    }

	if(m_fundProfitRate.F7day_profit_rate!= m_params.getLong("seven_day_profit_rate")&&m_params.getLong("seven_day_profit_rate")!=0)
    {
        gPtrAppLog->error("profit record exist, seven_day_profit_rate is different! seven_day_profit_rate in db[%lld], seven_day_profit_rate input[%lld] ", 
			m_fundProfitRate.F7day_profit_rate, m_params.getLong("seven_day_profit_rate"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "profit record exist, seven_day_profit_rate is different!", __FILE__, __LINE__);
    }

	if(!m_updateDetailRate)
	{
		//不更新明细数据情况:检查内容一致后抛出重入错误
		throw CException(ERR_REPEAT_ENTRY, "profit record exist! ", __FILE__, __LINE__);
	}
} 


/**
* 登记收益率信息
*/
void FundRegProfitRate::regProfitRate()
{
	FundProfitRate  fundProfitRate;
    memset(&fundProfitRate, 0, sizeof(FundProfitRate));

    strncpy(fundProfitRate.Fspid, m_params.getString("spid").c_str(), sizeof(fundProfitRate.Fspid)-1);
    strncpy(fundProfitRate.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundProfitRate.Ffund_code)-1);
    strncpy(fundProfitRate.Fdate, m_params.getString("date").c_str(), sizeof(fundProfitRate.Fdate)-1);

	if(m_updateDetailRate)
	{
		fundProfitRate.F1week_rise_rate= m_params.getLong("a_week_rise_rate");
		fundProfitRate.F1month_rise_rate= m_params.getLong("a_month_rise_rate");
		fundProfitRate.F3month_rise_rate= m_params.getLong("three_month_rise_rate");
		fundProfitRate.F6month_rise_rate= m_params.getLong("six_month_rise_rate");
		fundProfitRate.F1year_rise_rate= m_params.getLong("a_year_rise_rate");
		fundProfitRate.Fcumulative_net= m_params.getLong("cumulative_net");

		fundProfitRate.F1month_annual_profit= m_params.getLong("a_month_annual_profit");
		fundProfitRate.F3month_annual_profit= m_params.getLong("three_month_annual_profit");
		fundProfitRate.F6month_annual_profit= m_params.getLong("six_month_annual_profit");
		fundProfitRate.F1year_track_error= m_params.getLong("a_year_track_error");
	}
	strncpy(fundProfitRate.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundProfitRate.Fmodify_time)-1);

	// 插入基础收益率数据
	if(m_updateBaseRate&&m_fundProfitRate.Fimt_id==0)
	{
    	fundProfitRate.F1day_profit_rate= m_params.getLong("day_profit_rate");
		fundProfitRate.F7day_profit_rate= m_params.getLong("seven_day_profit_rate");
		strncpy(fundProfitRate.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundProfitRate.Fcreate_time)-1);
		insertFundProfitRate(m_pFundCon, fundProfitRate);
	// 更新外部收益率数据
	}else if(m_fundProfitRate.Fimt_id>0)
	{
		fundProfitRate.Fimt_id = m_fundProfitRate.Fimt_id;
		updateFundProfitRate(m_pFundCon,fundProfitRate);
	}else
	{
		// 不该出现的情况
		char errMsg[128]={0};
		snprintf(errMsg, sizeof(errMsg), "基础数据未更新不能先更新其他数据[%s][%s][%d]",m_fundProfitRate.Fspid,m_fundProfitRate.Fdate,m_updateBaseRate); 
		throw CException(ERR_REG_OUTER_PROFIT_RATE, errMsg, __FILE__, __LINE__);		
	}

	//更新缓存
	updateProfitRateCache();
}

void FundRegProfitRate::updateProfitRateCache()
{
		FundProfitRate data;
		memset(&data, 0,sizeof(FundProfitRate));

		strncpy(data.Fspid,m_params.getString("spid").c_str(), sizeof(data.Fspid) - 1);
		strncpy(data.Ffund_code,m_params.getString("fund_code").c_str(), sizeof(data.Ffund_code) - 1);
		strncpy(data.Fdate,m_params.getString("date").c_str(), sizeof(data.Fdate) - 1);

		if(!setSpProfitRateToKV(m_pFundCon,data))
		{
			TRACE_ERROR("[ckv更新失败][setSpProfitRateToKV][spid:%s]",data.Fspid);
		}

		if(!setMultiSpProfitRateToKV(m_pFundCon,data))
		{
			TRACE_ERROR("[ckv更新失败][setMultiSpProfitRateToKV][spid:%s]",data.Fspid);
		}
		
    	if(!setAllLastSpProfitRateToKV(m_pFundCon))
    	{
    		TRACE_ERROR("[ckv更新失败][setAllLastSpProfitRateToKV][spid:%s]",data.Fspid);
    	}
		
/*  最高收益率CKV不再使用，并且更新数据源不一致,删除
		//非指数的更新最高收益ckv
		if (is_indexFund)
			return;
		
		//更新最高收益率记录信息	
		if(!setHighestProfitRateSpToKV(m_pFundCon,data))
		{
			TRACE_ERROR("[ckv更新失败][setHighestProfitRateSpToKV][spid:%s]",data.Fspid);
		}
*/
}

/**
  * 打包输出参数
  */
void FundRegProfitRate::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
  
    rqst->olen = strlen(rqst->odata);
    return;
}


