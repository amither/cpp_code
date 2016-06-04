#define TIXML_USE_STL
#include "globalconfig.h"
#include "error.h"
#include "tinyxml.h"

/**
 * 取子节点
 */
TiXmlNode*  NODE_CHILD(TiXmlNode* pNode, const char* child=NULL)
{
    if(!pNode)
    {
        throw CException(ERR_NULL_XML_NODE, string("Get node failed:") + string(child), __FILE__, __LINE__);
    }

    TiXmlNode* pChild = child ?  pNode->FirstChild(child) : pNode->FirstChild();
    if (!pChild)
    {
        throw CException(ERR_NULL_XML_NODE, string("Get child node failed:") + string(child), __FILE__, __LINE__);
    }

    return pChild;
}

/**
 * 取相邻节点
 */
TiXmlNode* NODE_NEXT(TiXmlNode* pNode)
{
    return pNode->NextSibling();
}

/**
 * 取结点名称
 */
string NODE_NAME(TiXmlNode* pNode)
{
    return pNode->Value() ? pNode->Value() : "";
}

/**
 * 取结点值
 */
string NODE_STR_VALUE(TiXmlNode* pNode)
{
    return pNode->ToElement()->GetText() ? pNode->ToElement()->GetText() : "";
}

/**
 * 取结点值
 */
int NODE_INT_VALUE(TiXmlNode* pNode)
{
    return pNode->ToElement()->GetText() ? atoi(pNode->ToElement()->GetText()) : 0;
}

/**
 * 取结点值
 */
LONG NODE_LONG_VALUE(TiXmlNode* pNode)
{
    return pNode->ToElement()->GetText() ? atoll(pNode->ToElement()->GetText()) : 0;
}

/**
 * 取结点属性
 */
string NODE_ATTR(TiXmlNode* pNode, const char* attribute)
{
    return pNode->ToElement()->Attribute(attribute) ? pNode->ToElement()->Attribute(attribute) : "";
}

/**
 * 读取配置文件
 * @input       strCfgFile      配置文件
 */
GlobalConfig::GlobalConfig(const string& strCfgFile) throw(CException)
{
    // 解析配置文件
    TiXmlDocument  Doc(strCfgFile.c_str());
    if(!Doc.LoadFile())
    {
        throw CException(ERR_LOAD_XML_CFG, "Load xml config file error:" + strCfgFile);
    }


    /**
     * 获取DB配置信息
     */
    TiXmlNode* pDbNode = Doc.FirstChild("root")->FirstChild("fund_deal_db_conf");
    m_fund_deal_db.db_conf_key = NODE_STR_VALUE(NODE_CHILD(pDbNode, "db_conf_key"));
    m_fund_deal_db.db_conf_role= NODE_STR_VALUE(NODE_CHILD(pDbNode, "db_conf_role"));

	TiXmlNode* pDbSlaveNode = Doc.FirstChild("root")->FirstChild("fund_deal_slave_db_conf");
    m_fund_deal_slave_db.db_conf_key = NODE_STR_VALUE(NODE_CHILD(pDbSlaveNode, "db_conf_key"));
    m_fund_deal_slave_db.db_conf_role= NODE_STR_VALUE(NODE_CHILD(pDbSlaveNode, "db_conf_role"));

	/**
	* 读取子账户相关配置，包括用户分段及ttc、数据库以及子账户服务等配置信息
	*/
	TiXmlNode* pSubaccNode = Doc.FirstChild("root")->FirstChild("subacc");

	/**
	* 子账户用户分段
	*/
	TiXmlNode* pSubaccUserSegNode = NODE_CHILD(pSubaccNode, "subaccuserseg")->FirstChild("item");
    for(; pSubaccUserSegNode; pSubaccUserSegNode = NODE_NEXT(pSubaccUserSegNode))
    {
        SubaccUserSeg subaccUserSeg;
        subaccUserSeg.start_uid= atoi(NODE_ATTR(pSubaccUserSegNode, "start_uid").c_str());
        subaccUserSeg.end_uid= atoi(NODE_ATTR(pSubaccUserSegNode, "end_uid").c_str());
        subaccUserSeg.subacc_db= NODE_ATTR(pSubaccUserSegNode, "subacc_db");
        subaccUserSeg.subacc_ttc= NODE_ATTR(pSubaccUserSegNode, "subacc_ttc");

        m_subaccUserSegVec.push_back(subaccUserSeg);

		/**
	     * 获取子账户DB配置信息
	     */
	    TiXmlNode* psubaccDbNode = NODE_CHILD(pSubaccNode, subaccUserSeg.subacc_db.c_str());
		PassconfDB subaccDBCfg;
		subaccDBCfg.db_conf_key = NODE_STR_VALUE(NODE_CHILD(psubaccDbNode, "db_conf_key"));
		subaccDBCfg.db_conf_role = NODE_STR_VALUE(NODE_CHILD(psubaccDbNode, "db_conf_role"));

		m_subaccDbCfgVec[subaccUserSeg.subacc_db] = subaccDBCfg;

		/**
		* 子账户ttc-cache 配置信息
		*/
		TiXmlNode* pTtcconfigNode = NODE_CHILD(pSubaccNode, subaccUserSeg.subacc_ttc.c_str());
		HostCfg subaccTtcCfg;
		subaccTtcCfg.host = NODE_STR_VALUE(NODE_CHILD(pTtcconfigNode, "host"));
		subaccTtcCfg.port = NODE_INT_VALUE(NODE_CHILD(pTtcconfigNode, "port"));
		subaccTtcCfg.overtime= NODE_INT_VALUE(NODE_CHILD(pTtcconfigNode, "overtime"));
		subaccTtcCfg.user = NODE_STR_VALUE(NODE_CHILD(pTtcconfigNode, "user"));
	    subaccTtcCfg.pswd = NODE_STR_VALUE(NODE_CHILD(pTtcconfigNode, "pswd"));
	    subaccTtcCfg.charset = NODE_STR_VALUE(NODE_CHILD(pTtcconfigNode, "charset"));

		m_subaccTtcCfgVec[subaccUserSeg.subacc_ttc] = subaccTtcCfg;
    }

	/**
     * 获取子账户服务配置信息
     */
    TiXmlNode* pSubaccServerNode = NODE_CHILD(pSubaccNode, "subaccserver");
    m_SubaccCfg.host = NODE_STR_VALUE(NODE_CHILD(pSubaccServerNode, "host"));
    m_SubaccCfg.user = NODE_STR_VALUE(NODE_CHILD(pSubaccServerNode, "user"));
    m_SubaccCfg.pswd = NODE_STR_VALUE(NODE_CHILD(pSubaccServerNode, "pswd"));
    m_SubaccCfg.port = NODE_INT_VALUE(NODE_CHILD(pSubaccServerNode, "port"));
    m_SubaccCfg.overtime = NODE_INT_VALUE(NODE_CHILD(pSubaccServerNode, "overtime"));
    m_SubaccCfg.charset = NODE_STR_VALUE(NODE_CHILD(pSubaccServerNode, "charset"));

	/**
     * 获取exau配置信息
     */
    TiXmlNode* pExauNode = Doc.FirstChild("root")->FirstChild("exau");
    m_ExauCfg.host = NODE_STR_VALUE(NODE_CHILD(pExauNode, "host"));
    m_ExauCfg.user = NODE_STR_VALUE(NODE_CHILD(pExauNode, "user"));
    m_ExauCfg.pswd = NODE_STR_VALUE(NODE_CHILD(pExauNode, "pswd"));
    m_ExauCfg.port = NODE_INT_VALUE(NODE_CHILD(pExauNode, "port"));
    m_ExauCfg.overtime = NODE_INT_VALUE(NODE_CHILD(pExauNode, "overtime"));
    m_ExauCfg.charset = NODE_STR_VALUE(NODE_CHILD(pExauNode, "charset"));


    TiXmlNode* pFundFetchNode = Doc.FirstChild("root")->FirstChild("fundfetchserver");
    m_FundFetchCfg.host = NODE_STR_VALUE(NODE_CHILD(pFundFetchNode, "host"));
    m_FundFetchCfg.user = NODE_STR_VALUE(NODE_CHILD(pFundFetchNode, "user"));
    m_FundFetchCfg.pswd = NODE_STR_VALUE(NODE_CHILD(pFundFetchNode, "pswd"));
    m_FundFetchCfg.port = NODE_INT_VALUE(NODE_CHILD(pFundFetchNode, "port"));
    m_FundFetchCfg.overtime = NODE_INT_VALUE(NODE_CHILD(pFundFetchNode, "overtime"));

	/**
     * 获取绑定查询配置信息
     */
    TiXmlNode* pBindQueryNode = Doc.FirstChild("root")->FirstChild("bindquery");
    m_BindQueryCfg.host = NODE_STR_VALUE(NODE_CHILD(pBindQueryNode, "host"));
    m_BindQueryCfg.user = NODE_STR_VALUE(NODE_CHILD(pBindQueryNode, "user"));
    m_BindQueryCfg.pswd = NODE_STR_VALUE(NODE_CHILD(pBindQueryNode, "pswd"));
    m_BindQueryCfg.port = NODE_INT_VALUE(NODE_CHILD(pBindQueryNode, "port"));
    m_BindQueryCfg.overtime = NODE_INT_VALUE(NODE_CHILD(pBindQueryNode, "overtime"));
    m_BindQueryCfg.charset = NODE_STR_VALUE(NODE_CHILD(pBindQueryNode, "charset"));

	/**
	 * billno配置信息
	 */
	TiXmlNode* pBillnoNode = Doc.FirstChild("root")->FirstChild("billno");
	m_BillnoCfg.host = NODE_STR_VALUE(NODE_CHILD(pBillnoNode, "host"));
	m_BillnoCfg.port = NODE_INT_VALUE(NODE_CHILD(pBillnoNode, "port"));
	m_BillnoCfg.appid= NODE_INT_VALUE(NODE_CHILD(pBillnoNode, "appid"));

	/**
	 *主ckv配置信息
	 */
	TiXmlNode* pCkvconfigNode = Doc.FirstChild("root")->FirstChild("ckvconfig");
	m_CkvCfg.m_use_l5 = NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "m_use_l5"));
	m_CkvCfg.l5_modid = NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "l5_modid"));
	m_CkvCfg.l5_cmdid= NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "l5_cmdid"));
	m_CkvCfg.bid= NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "bid"));
	m_CkvCfg.passwd= NODE_STR_VALUE(NODE_CHILD(pCkvconfigNode, "passwd"));
	m_CkvCfg.timeout= NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "timeout"));
	m_CkvCfg.host_vec = split(NODE_STR_VALUE(NODE_CHILD(pCkvconfigNode, "host_list")),",");
	m_CkvCfg.port = NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "port"));

    	/**
	 *备份ckv配置信息
	 */
	pCkvconfigNode = Doc.FirstChild("root")->FirstChild("ckvconfig_slave");
       if (NULL != pCkvconfigNode)
       {
        	m_CkvCfg_slave.m_use_l5 = NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "m_use_l5"));
        	m_CkvCfg_slave.l5_modid = NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "l5_modid"));
        	m_CkvCfg_slave.l5_cmdid= NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "l5_cmdid"));
        	m_CkvCfg_slave.bid= NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "bid"));
        	m_CkvCfg_slave.passwd= NODE_STR_VALUE(NODE_CHILD(pCkvconfigNode, "passwd"));
        	m_CkvCfg_slave.timeout= NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "timeout"));
        	m_CkvCfg_slave.host_vec = split(NODE_STR_VALUE(NODE_CHILD(pCkvconfigNode, "host_list")),",");
        	m_CkvCfg_slave.port = NODE_INT_VALUE(NODE_CHILD(pCkvconfigNode, "port"));
       }

    /**
	 * 子账户ckv配置信息
	 */
	TiXmlNode* pSubAccCkvconfigNode = Doc.FirstChild("root")->FirstChild("subacc_ckvconfig");
	m_SubAccCkvCfg.m_use_l5 = NODE_INT_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "m_use_l5"));
	m_SubAccCkvCfg.l5_modid = NODE_INT_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "l5_modid"));
	m_SubAccCkvCfg.l5_cmdid= NODE_INT_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "l5_cmdid"));
	m_SubAccCkvCfg.bid= NODE_INT_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "bid"));
	m_SubAccCkvCfg.passwd= NODE_STR_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "passwd"));
	m_SubAccCkvCfg.timeout= NODE_INT_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "timeout"));
	m_SubAccCkvCfg.host_vec = split(NODE_STR_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "host_list")),",");
	m_SubAccCkvCfg.port = NODE_INT_VALUE(NODE_CHILD(pSubAccCkvconfigNode, "port"));

    /**
     * 获取业务配置信息
     */
    TiXmlNode* pAppNode = Doc.FirstChild("root")->FirstChild("app");
    m_AppCfg.pre_regkey     = NODE_STR_VALUE(NODE_CHILD(pAppNode, "pre_regkey"));
    m_AppCfg.nopass_reset_paycard_key     = NODE_STR_VALUE(NODE_CHILD(pAppNode, "nopass_reset_paycard_key"));
	m_AppCfg.subacc_cur_type     = NODE_INT_VALUE(NODE_CHILD(pAppNode, "subacc_cur_type"));
	m_AppCfg.redem_timeout_conf     = NODE_STR_VALUE(NODE_CHILD(pAppNode, "redem_timeout_conf"));
	m_AppCfg.trans_recon_type     = NODE_INT_VALUE(NODE_CHILD(pAppNode, "trans_recon_type"));
	if(m_AppCfg.trans_recon_type != 1 && m_AppCfg.trans_recon_type != 2)
	{
		throw CException(ERR_CONFIG_ERROR, "config trans_recon_type error:" + m_AppCfg.trans_recon_type);
	}
	m_AppCfg.change_sp_stop_time     = NODE_STR_VALUE(NODE_CHILD(pAppNode, "change_sp_stop_time"));
	m_AppCfg.paycb_overtime_inteval     = NODE_INT_VALUE(NODE_CHILD(pAppNode, "paycb_overtime_inteval"));
	m_AppCfg.sp_loading_credit= NODE_LONG_VALUE(NODE_CHILD(pAppNode, "sp_loading_credit"));
	m_AppCfg.total_loading_credit= NODE_LONG_VALUE(NODE_CHILD(pAppNode, "total_loading_credit"));
	m_AppCfg.sp_loading_warning_limit = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "sp_loading_warning_limit"));
	m_AppCfg.sp_loading_stop_redem_limit = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "sp_loading_stop_redem_limit"));
	m_AppCfg.over_loading_draw_type= NODE_INT_VALUE(NODE_CHILD(pAppNode, "over_loading_draw_type"));
	m_AppCfg.over_loading_use_common = NODE_INT_VALUE(NODE_CHILD(pAppNode, "over_loading_use_common"));
	m_AppCfg.support_redem_to_cft= NODE_INT_VALUE(NODE_CHILD(pAppNode, "support_redem_to_cft"));
	m_AppCfg.reward_profit_rate= NODE_LONG_VALUE(NODE_CHILD(pAppNode, "reward_profit_rate"));
	m_AppCfg.ui_ttc_max_err= NODE_INT_VALUE(NODE_CHILD(pAppNode, "ui_ttc_max_err"));
	m_AppCfg.ui_ttc_stop_time= NODE_INT_VALUE(NODE_CHILD(pAppNode, "ui_ttc_stop_time"));
	m_AppCfg.exau_sp_id = NODE_STR_VALUE(NODE_CHILD(pAppNode, "exau_sp_id"));
	m_AppCfg.user_max_share = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "user_max_share"));
	m_AppCfg.outer_vip_max_share = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "outer_vip_max_share"));
	m_AppCfg.tencent_vip_max_share = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "tencent_vip_max_share"));
	m_AppCfg.rebuy_buf = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "rebuy_buf"));
	m_AppCfg.seven_day_profit_rate_max= NODE_LONG_VALUE(NODE_CHILD(pAppNode, "seven_day_profit_rate_max"));
	m_AppCfg.seven_day_profit_rate_min= NODE_LONG_VALUE(NODE_CHILD(pAppNode, "seven_day_profit_rate_min"));
	m_AppCfg.sentMqMsg= NODE_INT_VALUE(NODE_CHILD(pAppNode, "sentMqMsg"));
	m_AppCfg.update_pay_card_limit = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "update_pay_card_limit"));
	m_AppCfg.check_same_pay_card= NODE_INT_VALUE(NODE_CHILD(pAppNode, "check_same_pay_card"));
	m_AppCfg.payCardWhiteVec = split(NODE_STR_VALUE(NODE_CHILD(pAppNode, "pay_card_white_list_sp")),"|");

	m_AppCfg.stop_fetch_when_overloading = NODE_INT_VALUE(NODE_CHILD(pAppNode, "stop_fetch_when_overloading"));
       m_AppCfg.check_tpulsbalance_for_reg_profit = NODE_INT_VALUE(NODE_CHILD(pAppNode, "check_tpulsbalance_for_reg_profit"));
       m_AppCfg.sp_loading_last_allowed_redem_limit=NODE_LONG_VALUE(NODE_CHILD(pAppNode, "sp_loading_last_allowed_redem_limit"));
	m_AppCfg.multi_sp_config = NODE_INT_VALUE(NODE_CHILD(pAppNode, "multi_sp_config"));
	m_AppCfg.default_sp= NODE_STR_VALUE(NODE_CHILD(pAppNode, "default_sp"));
      m_AppCfg.tplus_redem_switch = NODE_INT_VALUE(NODE_CHILD(pAppNode, "tplus_redem_switch"));
      m_AppCfg.tplus_redem_sps_white_list = NODE_STR_VALUE(NODE_CHILD(pAppNode, "tplus_redem_sps_white_list"));
      m_AppCfg.key_unbind = NODE_STR_VALUE(NODE_CHILD(pAppNode, "key_unbind"));
	m_AppCfg.subacc_split_by_last3= NODE_INT_VALUE(NODE_CHILD(pAppNode, "subacc_split_by_last3"));
      m_AppCfg.unbind_no_trans_days = NODE_INT_VALUE(NODE_CHILD(pAppNode, "unbind_no_trans_days"));
      m_AppCfg.unbindwhitelist = NODE_STR_VALUE(NODE_CHILD(pAppNode, "unbindwhitelist"));

      m_AppCfg.coupon_key = NODE_STR_VALUE(NODE_CHILD(pAppNode, "coupon_key"));
	  m_AppCfg.transfer_ackkey= NODE_STR_VALUE(NODE_CHILD(pAppNode, "transfer_ackkey"));
      m_AppCfg.transfer_reqkey= NODE_STR_VALUE(NODE_CHILD(pAppNode, "transfer_reqkey"));
      m_AppCfg.transfer_redem_result_key= NODE_STR_VALUE(NODE_CHILD(pAppNode, "transfer_redem_result_key"));
      m_AppCfg.max_transfer_times_oneday = NODE_INT_VALUE(NODE_CHILD(pAppNode, "max_transfer_times_oneday"));
      m_AppCfg.sp_redem_rate = NODE_INT_VALUE(NODE_CHILD(pAppNode, "sp_redem_rate"));
      m_AppCfg.transfer_limit_white_list = NODE_STR_VALUE(NODE_CHILD(pAppNode, "transfer_limit_white_list"));
      m_AppCfg.sp_redem_tranfer_limit_offset = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "sp_redem_tranfer_limit_offset"));
      m_AppCfg.freeze_service_key = NODE_STR_VALUE(NODE_CHILD(pAppNode, "freeze_service_key"));
      m_AppCfg.max_transfer_fee_one_time = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "max_transfer_fee_one_time"));
	  m_AppCfg.close_buy_date_upper_limit = NODE_INT_VALUE(NODE_CHILD(pAppNode, "close_buy_date_upper_limit"));

	  m_AppCfg.switch_conf_center = NODE_INT_VALUE(NODE_CHILD(pAppNode, "switch_conf_center"));
	  m_AppCfg.outer_vip_list= NODE_STR_VALUE(NODE_CHILD(pAppNode, "outer_vip_list"));
	  m_AppCfg.tencent_vip_list= NODE_STR_VALUE(NODE_CHILD(pAppNode, "tencent_vip_list"));
      //更新收益ckv的控制开关，1-直接使用CKV中的数据进行更新，其它-查询DB数据更新CKV
      m_AppCfg.update_profit_ckv_switch = NODE_INT_VALUE(NODE_CHILD(pAppNode, "update_profit_ckv_switch"));
	  m_AppCfg.kf_start_time= NODE_STR_VALUE(NODE_CHILD(pAppNode, "kf_start_time"));
	  m_AppCfg.kf_end_time= NODE_STR_VALUE(NODE_CHILD(pAppNode, "kf_end_time"));
	  //到期赎回设置开关
	  m_AppCfg.check_end_redem_time=NODE_INT_VALUE(NODE_CHILD(pAppNode, "check_end_redem_time")); //0:不检查; 1:检查
	  m_AppCfg.change_end_redem_time=NODE_STR_VALUE(NODE_CHILD(pAppNode, "change_end_redem_time"));//HHmmdd 格式
	  m_AppCfg.close_end_change_sp_config=NODE_INT_VALUE(NODE_CHILD(pAppNode, "close_end_change_sp_config"));//0:不记入余额; 1:记录余额
	  m_AppCfg.close_redem_req_stop_time=NODE_STR_VALUE(NODE_CHILD(pAppNode, "close_redem_req_stop_time"));//定期赎回请求停止时间hhmmdd
	  m_AppCfg.close_redem_ack_stop_time=NODE_STR_VALUE(NODE_CHILD(pAppNode, "close_redem_ack_stop_time"));//定期赎回确认停止时间hhmmdd

     m_AppCfg.charge_service_req_key = NODE_STR_VALUE(NODE_CHILD(pAppNode, "charge_service_req_key"));
     m_AppCfg.charge_service_cnf_key = NODE_STR_VALUE(NODE_CHILD(pAppNode, "charge_service_cnf_key"));
     m_AppCfg.fetch_service_key = NODE_STR_VALUE(NODE_CHILD(pAppNode, "fetch_service_key"));
     m_AppCfg.refund_service_key = NODE_STR_VALUE(NODE_CHILD(pAppNode,"refund_service_key"));
	 m_AppCfg.update_fund_bind_key = NODE_STR_VALUE(NODE_CHILD(pAppNode,"update_fund_bind_key"));
      m_AppCfg.assert_limit_level1_chargefee = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "assert_limit_level1_chargefee"));
      m_AppCfg.sp_loading_enough_check_befor_subacc = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "sp_loading_enough_check_befor_subacc"));
      m_AppCfg.consum_fund_spid =NODE_STR_VALUE(NODE_CHILD(pAppNode, "consum_fund_spid"));
      m_AppCfg.wx_wfj_spid =NODE_STR_VALUE(NODE_CHILD(pAppNode, "wx_wfj_spid"));
      m_AppCfg.check_exau_auth_limit= NODE_INT_VALUE(NODE_CHILD(pAppNode, "check_exau_auth_limit"));
      m_AppCfg.multycardbuy_allow_all_switch=NODE_INT_VALUE(NODE_CHILD(pAppNode, "multycardbuy_allow_all_switch"));
      m_AppCfg.usr_rst_paycard_fee_limit = NODE_LONG_VALUE(NODE_CHILD(pAppNode, "usr_rst_paycard_fee_limit"));
      m_AppCfg.undone_trans_timespan = NODE_INT_VALUE(NODE_CHILD(pAppNode, "undone_trans_timespan"));
      m_AppCfg.risk_assess_key=NODE_STR_VALUE(NODE_CHILD(pAppNode, "risk_assess_key"));
	m_AppCfg.insert_trans_date_key=NODE_STR_VALUE(NODE_CHILD(pAppNode, "insert_trans_date_key"));
	m_AppCfg.update_trans_date_key=NODE_STR_VALUE(NODE_CHILD(pAppNode, "update_trans_date_key"));

	  
      vector<string> assessRiskVec =split(NODE_STR_VALUE(NODE_CHILD(pAppNode, "assess_risk_type_conf")),"|");
	m_AppCfg.AssessRiskTypeVec.clear();
	for(vector<string>::iterator iter = assessRiskVec.begin(); iter != assessRiskVec.end();++iter)
	{
	         string riskType= *iter;
                m_AppCfg.AssessRiskTypeVec.push_back(atoi(riskType.c_str()));
	}
      m_AppCfg.refund_for_check_bind_serailno_diff=NODE_INT_VALUE(NODE_CHILD(pAppNode, "refund_for_check_bind_serailno_diff"));
	
	m_AppCfg.account_freeze_key = NODE_STR_VALUE(NODE_CHILD(pAppNode, "account_freeze_key"));
      m_AppCfg.account_freeze_white_ip = NODE_STR_VALUE(NODE_CHILD(pAppNode, "account_freeze_white_ip"));
	  m_AppCfg.fund_index_trans_grey = NODE_INT_VALUE(NODE_CHILD(pAppNode, "fund_index_trans_grey"));

    m_AppCfg.db_encode_switch=NODE_INT_VALUE(NODE_CHILD(pAppNode, "db_encode_switch")); //0:不检查; 1:检查
		  
	/**
	 * 获取微信推荐商户配置信息
	 */
	TiXmlNode* pWxRecommendSpNode = Doc.FirstChild("root")->FirstChild("wx_recommend_sp");

	//推荐商户白名单配置
	vector<string> wxRecSpWhiteVec = split(NODE_STR_VALUE(NODE_CHILD(pWxRecommendSpNode, "rec_sp_white_list")),"|");
	m_WxRecommendSpCfg.rec_sp_white_list.clear();
	for(vector<string>::iterator iter = wxRecSpWhiteVec.begin(); iter != wxRecSpWhiteVec.end();++iter)
	{
		string spWhiteConfig = *iter;
		size_t spPos = spWhiteConfig.find(':');
		if(spPos != string::npos)
		{
			string spid = spWhiteConfig.substr(0, spPos);
			string whiteListStr = spWhiteConfig.substr(spPos + 1);

			vector<string> whiteUserVec = split(whiteListStr,",");

			for(vector<string>::iterator iter = whiteUserVec.begin(); iter != whiteUserVec.end();++iter)
			{
				m_WxRecommendSpCfg.rec_sp_white_list[*iter] = spid;
			}
		}
	}

	//推荐商户按用户尾号配置
	vector<string> wxRecSpUinTailStr = split(NODE_STR_VALUE(NODE_CHILD(pWxRecommendSpNode, "rec_sp_uin_tail_config")),"|");
	m_WxRecommendSpCfg.rec_sp_uin_tail_config.clear();
	for(vector<string>::iterator iter = wxRecSpUinTailStr.begin(); iter != wxRecSpUinTailStr.end();++iter)
	{
		string spUserTailConfig = *iter;
		size_t spPos = spUserTailConfig.find(':');
		if(spPos != string::npos)
		{
			string spid = spUserTailConfig.substr(0, spPos);
			string uinTailListStr = spUserTailConfig.substr(spPos + 1);

			vector<string> uinTailListVec = split(uinTailListStr,",");

			for(vector<string>::iterator iter = uinTailListVec.begin(); iter != uinTailListVec.end();++iter)
			{
				m_WxRecommendSpCfg.rec_sp_uin_tail_config[*iter] = spid;
			}
		}
	}

	/**
	 * 获取手Q推荐商户配置信息
	 */
	TiXmlNode* pCftqqRecommendSpNode = Doc.FirstChild("root")->FirstChild("cftqq_recommend_sp");

	//推荐商户白名单配置
	vector<string> cftqqRecSpWhiteVec = split(NODE_STR_VALUE(NODE_CHILD(pCftqqRecommendSpNode, "rec_sp_white_list")),"|");
	m_CftqqRecommendSpCfg.rec_sp_white_list.clear();
	for(vector<string>::iterator iter = cftqqRecSpWhiteVec.begin(); iter != cftqqRecSpWhiteVec.end();++iter)
	{
		string spWhiteConfig = *iter;
		size_t spPos = spWhiteConfig.find(':');
		if(spPos != string::npos)
		{
			string spid = spWhiteConfig.substr(0, spPos);
			string whiteListStr = spWhiteConfig.substr(spPos + 1);

			vector<string> whiteUserVec = split(whiteListStr,",");

			for(vector<string>::iterator iter = whiteUserVec.begin(); iter != whiteUserVec.end();++iter)
			{
				m_CftqqRecommendSpCfg.rec_sp_white_list[*iter] = spid;
			}
		}
	}

	//推荐商户按用户尾号配置
	vector<string> cftqqRecSpUinTailStr = split(NODE_STR_VALUE(NODE_CHILD(pCftqqRecommendSpNode, "rec_sp_uin_tail_config")),"|");
	m_CftqqRecommendSpCfg.rec_sp_uin_tail_config.clear();
	for(vector<string>::iterator iter = cftqqRecSpUinTailStr.begin(); iter != cftqqRecSpUinTailStr.end();++iter)
	{
		string spUserTailConfig = *iter;
		size_t spPos = spUserTailConfig.find(':');
		if(spPos != string::npos)
		{
			string spid = spUserTailConfig.substr(0, spPos);
			string uinTailListStr = spUserTailConfig.substr(spPos + 1);

			vector<string> uinTailListVec = split(uinTailListStr,",");

			for(vector<string>::iterator iter = uinTailListVec.begin(); iter != uinTailListVec.end();++iter)
			{
				m_CftqqRecommendSpCfg.rec_sp_uin_tail_config[*iter] = spid;
			}
		}
	}

    /**
     * 获取系统配置信息
     */
    TiXmlNode* pSysNode = Doc.FirstChild("root")->FirstChild("system");
    m_SysCfg.mid_no = NODE_INT_VALUE(NODE_CHILD(pSysNode, "mid_no"));
    m_SysCfg.log_path = NODE_STR_VALUE(NODE_CHILD(pSysNode, "log_path"));
    m_SysCfg.log_num = NODE_INT_VALUE(NODE_CHILD(pSysNode, "log_num"));
    m_SysCfg.log_size = NODE_INT_VALUE(NODE_CHILD(pSysNode, "log_size"));
    m_SysCfg.encrypt_sp_id = NODE_STR_VALUE(NODE_CHILD(pSysNode, "encrypt_sp_id"));
    m_SysCfg.balance_sp_id  = NODE_STR_VALUE(NODE_CHILD(pSysNode, "balance_sp_id"));
    m_SysCfg.db_idle_time = NODE_INT_VALUE(NODE_CHILD(pSysNode, "db_idle_time"));
    //如果db中idle_time < 600 s，则强制设置成600s
    if(m_SysCfg.db_idle_time < 600)
        m_SysCfg.db_idle_time = 600;

	/**
     * 安全域内mq配置
     */
    TiXmlNode* pMqNode = Doc.FirstChild("root")->FirstChild("mqconf");
    m_MqCfg.host = NODE_STR_VALUE(NODE_CHILD(pMqNode, "host"));
    m_MqCfg.port = NODE_INT_VALUE(NODE_CHILD(pMqNode, "port"));
    m_MqCfg.overtime = NODE_INT_VALUE(NODE_CHILD(pMqNode, "overtime"));
    m_MqCfg.concurrent = NODE_INT_VALUE(NODE_CHILD(pMqNode, "concurrent"));
    TiXmlNode* pSptransferRate = Doc.FirstChild("root")->FirstChild("sp_transfer_rate");
    if (pSptransferRate != NULL)
    {
        for (TiXmlNode* pitem=pSptransferRate->FirstChild("item");pitem != NULL;pitem=NODE_NEXT(pitem))
        {
            string spid = NODE_ATTR(pitem, "spid");
            int rate =  atoi(NODE_ATTR(pitem, "transfer_rate").c_str());
            m_spTransferRate[spid] = rate;
        }
    }
    TiXmlNode* pSpDianziCfg = Doc.FirstChild("root")->FirstChild("sp_dianzi_info");
    if (pSpDianziCfg != NULL)
    {
        for (TiXmlNode* pitem=pSpDianziCfg->FirstChild("item");pitem != NULL;pitem=NODE_NEXT(pitem))
        {
            string spid = NODE_ATTR(pitem, "spid");
            LONG wan_balance =  atoll(NODE_ATTR(pitem, "warn_balance").c_str());
            LONG allowlastfastredem_balance =  atoll(NODE_ATTR(pitem, "last_redem_allowed_balance").c_str());
            m_spLastAllowFastRedem_Dianzi[spid] = allowlastfastredem_balance;
            m_spWarn_Dianzi[spid] = wan_balance;
        }
    }


    /*
     * session服务器配置
     */
    if(Doc.FirstChild("root")->FirstChild("session_cfg") == NULL)
    {
        throw CException(ERR_CONFIG_ERROR, "config session_cfg empty");
    }
    {
        TiXmlNode* pSession = Doc.FirstChild("root")->FirstChild("session_cfg");
        m_SessionSvrCfg.bid = NODE_INT_VALUE(NODE_CHILD(pSession, "bid"));
        m_SessionSvrCfg.shm_id = NODE_INT_VALUE(NODE_CHILD(pSession, "shm_id"));
        m_SessionSvrCfg.conn_timeout = NODE_INT_VALUE(NODE_CHILD(pSession, "conn_timeout"));
        m_SessionSvrCfg.rw_timeout = NODE_INT_VALUE(NODE_CHILD(pSession, "rw_timeout"));
        m_SessionSvrCfg.ischk_shm = NODE_INT_VALUE(NODE_CHILD(pSession, "ischk_shm"));
        m_SessionSvrCfg.chk_shm_num = NODE_INT_VALUE(NODE_CHILD(pSession, "chk_shm_num"));

        TiXmlNode* pSessionHost = pSession->FirstChild("node");
        for(;pSessionHost; pSessionHost=NODE_NEXT(pSessionHost))
        {
            SessionHostInfo stSessCfg;
            stSessCfg.strHost = NODE_ATTR(pSessionHost, "host");
            stSessCfg.iPort = atoi(NODE_ATTR(pSessionHost, "port").c_str());
            stSessCfg.iTimeout = atoi(NODE_ATTR(pSessionHost, "timeout").c_str());

            m_SessionSvrCfg.sess_hosts.push_back(stSessCfg);
        }
    }

    //用户限额等级 同 登记对应的限额值 配置关系
    TiXmlNode* pAssetLevelConf = Doc.FirstChild("root")->FirstChild("asset_level_conf");

    if(pAssetLevelConf == NULL)
    {
        throw CException(ERR_CONFIG_ERROR, "config asset_level_conf empty");
    }
    for (TiXmlNode* pAssetLevelItem=pAssetLevelConf->FirstChild("item"); pAssetLevelItem; pAssetLevelItem = NODE_NEXT(pAssetLevelItem))
    {
        m_assetLimitLev2Value[atoi(NODE_ATTR(pAssetLevelItem, "level").c_str())] = atoll(NODE_ATTR(pAssetLevelItem, "limit").c_str());
    }
}

/**
 * 打印配置内容
 */
ostream& GlobalConfig::dump(ostream& os) const
{

	// 打印billno配置信息
		os<<"Billno configuration:"<<std::endl;
		os<<"\t\t"<<m_BillnoCfg.host<<":"<<m_BillnoCfg.port
			<<":"<<m_BillnoCfg.appid
			<<std::endl;

    // 打印系统配置信息
    os<<"System configuration:"<<std::endl
        <<"\t\tmid_no="<<m_SysCfg.mid_no
        <<"\t\tlog_path="<<m_SysCfg.log_path
        <<"\t\tlog_num="<<m_SysCfg.log_num
        <<"\t\tlog_size="<<m_SysCfg.log_size
        <<std::endl;

    return os;
}

/**
 * 打印配置内容
 */
ostream& operator<<(ostream& os, const GlobalConfig& Cfg)
{
    return Cfg.dump(os);
}

