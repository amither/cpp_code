/**
  * FileName: fund_deal_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金交易服务
  */


#include <sys/time.h>
#include <unistd.h>
#include "fund_common.h"
#include "fund_commfunc.h"
//#include "serruninfo.h"

#include "fund_deal_service.h"

#include "fundacc_bind_sp_service.h"
#include "fund_buy_sp_req_service.h"
#include "fund_buy_sp_ack_service.h"
#include "fund_redem_sp_req_service.h"
#include "fund_redem_sp_ack_service.h"
#include "fund_spaccount_freeze_service.h"
#include "fund_reg_daily_profit_service.h"
#include "fund_reg_close_profit_service.h"
#include "fund_prepay_req_service.h"
#include "fund_prepay_ack_service.h"
#include "fund_query_bind_sp_service.h"
#include "fund_query_total_profit_service.h"
#include "fund_update_kvcache_service.h"
#include "fund_query_recommend_sp_service.h"
#include "fund_reg_profit_rate_service.h"
#include "fund_query_balance_service.h"
#include "fund_update_pay_card_service.h"
#include "fund_subacc_budan_service.h"
#include "fund_nopass_reset_paycard_service.h"
#include "fundacc_unbind_service.h"
#include "fund_update_bank_config_service.h"
#include "fund_update_end_type.h"
#include "passconf_api.h"
#include "relay_client.h"
#include "fund_transfer_ack_service.h"
#include "fund_transfer_req_service.h"
#include "user_classify.h"
#include "fund_balance_unfreeze_service.h"
#include "fund_balance_freeze_service.h"
#include "fund_balance_pre_freeze_service.h"
#include "fund_comm_qry_chklgi.h"
#include "fund_balance_pre_freeze_service.h"
#include "fund_fetch_ack_service.h"
#include "fund_fetch_req_service.h"
#include "fund_charge_ack_service.h"
#include "fund_charge_req_service.h"
#include "fund_kf_redem_sp_req_service.h"
#include "fund_close_end_redem_service.h"
#include "fund_close_roll_service.h"
#include "fund_redem_close_req_service.h"
#include "fund_redem_close_ack_service.h"
#include "fund_query_balance_order_service.h"
#include "fund_balance_recover_service.h"
#include "fund_manage_config_service.h"
#include "fund_check_ckv_service.h"
#include "fund_pre_record_user_acc_service.h"
#include "fund_risk_assess_service.h"
#include "fund_fetch_notify_service.h"
#include "fund_refund_service.h"
#include "factory_buy_sp_req_service.h"
#include "factory_buy_sp_ack_service.h"
#include "factory_redeem_sp_req_service.h"
#include "factory_redeem_sp_ack_service.h"
#include "fund_reg_exp_profit_service.h"
#include "fund_redeem_fetch_ack_service.h"
#include "fund_update_bind.h"
#include "fund_account_freeze_service.h"
#include "fund_insert_infomation_service.h"
#include "fund_update_infomation_service.h"
#include "fund_insert_trans_date.h"
#include "fund_update_trans_date.h"
#include "fund_deal_test.h"

//middle全局连接句柄

CRpcWrapper* gPtrSubaccRpc = NULL; //子账户服务连接
CRpcWrapper* gPtrExauRpc = NULL; //exau_check_server服务连接
CRpcWrapper* gPtrBindQueryRpc = NULL; //绑定银行卡查询服服务连接
CftLog* gPtrAppLog = NULL; // 应用日志句柄
CftLog* gPtrCkvErrorLog = NULL; // 更新ckv出错日志
CftLog* gPtrCkvSlaveErrorLog = NULL; // 更新备ckv出错日志
CftLog* gPtrSysLog = NULL; // 系统日志句柄
CftLog* gPtrCheckLog = NULL; // 审计日志句柄
GlobalConfig* gPtrConfig = NULL; // 全局配置句柄
CMySQL  *gPtrFundDB = NULL; //数据库连接句柄
CMySQL  *gPtrFundSlaveDB = NULL; //基金数据库备机连接句柄
CkvSvrOperator *gCkvSvrOperator = NULL; //ckv cache 访问句柄
CkvSvrOperator *gSubAccCkvSvrOperator = NULL; //子账户 ckv cache 访问句柄
map<string, CMySQL*> subaccDB;//子账户数据库句柄
CRpcWrapper* gPtrFundFetchRpc = NULL; //fund_fetch_server 服务连接
CSessionApi* gPtrSession = NULL; // session服务指针
tenpaymq::CRelayClient* gPtrRelayClient = NULL;//全局relayclient

UserClassify *g_user_classify = NULL; //配置中心的用户白名单句柄


/**
 * 打印调试信息
 * @input:  szLog 日志信息
 */
void DEBUG(const char* szLog)
{
    gPtrAppLog->debug("%s", szLog);
}

/**
 * 打印调试信息
 * @input:  szLog 日志信息
 */
void ERROR(const char* szLog)
{
    gPtrAppLog->error("%s", szLog);
}



/**
 * 打包输出消息
 */
void packMessage(int iRetcode,  const char* pResult, TRPC_SVCINFO* rqst)
{
    if( NULL == rqst->odata )
        return;

    if (strstr(pResult, "result=") == NULL)
    {
        rqst->olen = snprintf(rqst->odata, MAX_TRPC_DATA_LEN, "result=%d&res_info=%s", iRetcode, pResult);
    }
    else
    {
        rqst->olen = snprintf(rqst->odata, MAX_TRPC_DATA_LEN, "%s", pResult);
    }
}

void freeSubaccDb()
{
	for(map<string, CMySQL*>::iterator it = subaccDB.begin(); it!= subaccDB.end(); ++it)
	{
		delete it->second;
	}

}

/**
 * Description: 初始化数据库
 */
CMySQL  * initFunddealDb(GlobalConfig::PassconfDB& passconfDB)
{
	char szIdata[10 * 1024] = {0};
    char szOdata[10 * 1024] = {0};
    size_t oLen = sizeof(szOdata);
	// 设置参数
    CUrlAnalyze::setParam(szIdata, "server_name", "fund_deal_server", true);
    CUrlAnalyze::setParam(szIdata, "conf_sign", SERVER_VERSION_SIGN);
    CUrlAnalyze::setParam(szIdata, "conf_num", 1);
	CUrlAnalyze::setParam(szIdata, "conf_key0", passconfDB.db_conf_key.c_str());
    CUrlAnalyze::setParam(szIdata, "conf_role0", passconfDB.db_conf_role.c_str());

	trpc_debug_log("idata:%s", szIdata);

    if (getPassConf(szIdata, strlen(szIdata), szOdata, &oLen) != 0)
    {
        throw CException(ERR_DB_INITIAL, szOdata);
    }

	trpc_debug_log("odata:%s", szOdata);

	char    szHost[15 + 1];
	char    szUser[64 + 1];
	char    szPswd[64 + 1];
	int 	iPort;
	int     iOvertime;
	char	szCharset[32 + 1];
	CUrlAnalyze::getParam(szOdata, "ip0",szHost,sizeof(szHost)-1);
	CUrlAnalyze::getParam(szOdata, "user0", szUser, sizeof(szUser)-1);
	CUrlAnalyze::getParam(szOdata, "passwd0", szPswd, sizeof(szPswd)-1);
	CUrlAnalyze::getParam(szOdata, "overtime0", &iOvertime);
	CUrlAnalyze::getParam(szOdata, "port0", &iPort);
	CUrlAnalyze::getParam(szOdata, "charset0", szCharset, sizeof(szCharset)-1);

    TRACE_DEBUG("db_idle_time:%d", gPtrConfig->m_SysCfg.db_idle_time);
	// 初始化数据库连接
    return new CMySQL(szHost, szUser, szPswd, iPort, iOvertime, szCharset, "fund_db", gPtrConfig->m_SysCfg.db_idle_time);


	/*
    CDbConfig dbConfig("fund_deal_server", SERVER_VERSION_SIGN);

	TRACE_NORMAL("initFunddealDb. subacc_db_index[%s] role[%s] ", gPtrConfig->m_fund_deal_db.db_conf_key.c_str(),
				gPtrConfig->m_fund_deal_db.db_conf_role.c_str());

    //添加fund_db conf_key
    dbConfig.addDbName(gPtrConfig->m_fund_deal_db.db_conf_key, gPtrConfig->m_fund_deal_db.db_conf_role);
	TRACE_NORMAL("1");

    //获取数据库配置
    dbConfig.executeBatchDbQuery();
	TRACE_NORMAL("2");

    //初始化ocall_db
    DbHostCfg dbConfigItem = dbConfig.getDbConfigure(gPtrConfig->m_fund_deal_db.db_conf_key, gPtrConfig->m_fund_deal_db.db_conf_role);

    gPtrSysLog->debug("zwdb conf[%s|%s][%s]", dbConfigItem.szConfKey, dbConfigItem.szConfRole, dbConfigItem.szHost);


	// 初始化数据库连接
    gPtrFundDB = new CMySQL(dbConfigItem.szHost, dbConfigItem.szUser,
                                 dbConfigItem.szPswd,dbConfigItem.iPort,
                                 dbConfigItem.iOvertime, dbConfigItem.szCharset, "fund_db");
    */

}


void initSubaccConf()
{
	for(unsigned int i= 0; i< gPtrConfig->m_subaccUserSegVec.size(); ++i)
	{
		string subacc_db_index = gPtrConfig->m_subaccUserSegVec[i].subacc_db;

		char szIdata[10 * 1024] = {0};
	    char szOdata[10 * 1024] = {0};
	    size_t oLen = sizeof(szOdata);
		// 设置参数
	    CUrlAnalyze::setParam(szIdata, "server_name", "fund_deal_server", true);
	    CUrlAnalyze::setParam(szIdata, "conf_sign", SERVER_VERSION_SIGN);
	    CUrlAnalyze::setParam(szIdata, "conf_num", 1);
		CUrlAnalyze::setParam(szIdata, "conf_key0", gPtrConfig->m_subaccDbCfgVec[subacc_db_index].db_conf_key.c_str());
	    CUrlAnalyze::setParam(szIdata, "conf_role0", gPtrConfig->m_subaccDbCfgVec[subacc_db_index].db_conf_role.c_str());

		trpc_debug_log("idata:%s", szIdata);

	    if (getPassConf(szIdata, strlen(szIdata), szOdata, &oLen) != 0)
	    {
	        throw CException(ERR_DB_INITIAL, szOdata);
	    }

		trpc_debug_log("odata:%s", szOdata);

		char    szHost[15 + 1];
		char    szUser[64 + 1];
		char    szPswd[64 + 1];
		int 	iPort;
		int     iOvertime;
		char	szCharset[32 + 1];
		CUrlAnalyze::getParam(szOdata, "ip0",szHost,sizeof(szHost)-1);
		CUrlAnalyze::getParam(szOdata, "user0", szUser, sizeof(szUser)-1);
		CUrlAnalyze::getParam(szOdata, "passwd0", szPswd, sizeof(szPswd)-1);
		CUrlAnalyze::getParam(szOdata, "overtime0", &iOvertime);
		CUrlAnalyze::getParam(szOdata, "port0", &iPort);
		CUrlAnalyze::getParam(szOdata, "charset0", szCharset, sizeof(szCharset)-1);

        TRACE_DEBUG("db_idle_time:%d", gPtrConfig->m_SysCfg.db_idle_time);
		// 初始化数据库连接
	    CMySQL * gPtrFundDB = new CMySQL(szHost, szUser, szPswd, iPort, iOvertime, szCharset, "subacc_db", gPtrConfig->m_SysCfg.db_idle_time);

		subaccDB[subacc_db_index] = gPtrFundDB;

		TRACE_NORMAL("m_subaccDbCfgVec. subacc_db_index[%s] host[%s] port[%d]", subacc_db_index.c_str(),
				szHost, iPort);
	}
}


DECLARE_SO_INIT ()
{
    try
    {
		trpc_debug_log(">>> begin load config");
		// 初始化配置文件
        gPtrConfig = new GlobalConfig("/usr/local/middle/fund_deal_server/conf/fund_deal_server.xml");
		trpc_debug_log(">>> end load config");

		// 初始化日志指针
        gPtrAppLog = new CftLog((gPtrConfig->m_SysCfg.log_path + "/fund_deal").c_str(),
                                gPtrConfig->m_SysCfg.log_size * 1024 * 1024, gPtrConfig->m_SysCfg.log_num,
                                CftLog::_DATE_MODE);
        gPtrAppLog->setSuffix("_app");

		//更新ckv异常的数据写入
		gPtrCkvErrorLog = new CftLog((gPtrConfig->m_SysCfg.log_path + "/fund_deal").c_str(),
                                gPtrConfig->m_SysCfg.log_size * 1024 * 1024, gPtrConfig->m_SysCfg.log_num,
                                CftLog::_DATE_MODE);
        gPtrCkvErrorLog->setSuffix("_ckverror");
        
        gPtrCkvSlaveErrorLog = new CftLog((gPtrConfig->m_SysCfg.log_path + "/fund_deal").c_str(),
                                gPtrConfig->m_SysCfg.log_size * 1024 * 1024, gPtrConfig->m_SysCfg.log_num,
                                CftLog::_DATE_MODE);
        gPtrCkvSlaveErrorLog->setSuffix("_slaveckverror");
        

        gPtrSysLog = new CftLog((gPtrConfig->m_SysCfg.log_path + "/fund_deal_sys").c_str(),
                                gPtrConfig->m_SysCfg.log_size * 1024 * 1024, gPtrConfig->m_SysCfg.log_num);


        gPtrCheckLog = new CftLog((gPtrConfig->m_SysCfg.log_path + "/fund_deal_server_check").c_str(),
                                  gPtrConfig->m_SysCfg.log_size * 1024 * 1024, gPtrConfig->m_SysCfg.log_num);

		// 打印启动日志
		gPtrSysLog->normal("fund_deal_server start ...");

        // 初始化数据库
		gPtrFundDB = initFunddealDb(gPtrConfig->m_fund_deal_db);
		gPtrFundSlaveDB = initFunddealDb(gPtrConfig->m_fund_deal_slave_db);

		// 创建子账户连接
        gPtrSubaccRpc = new CRpcWrapper(gPtrConfig->m_SubaccCfg.host, gPtrConfig->m_SubaccCfg.port,
                                       gPtrConfig->m_SubaccCfg.user, gPtrConfig->m_SubaccCfg.pswd,
                                       gPtrConfig->m_SubaccCfg.overtime);

		// 创建exau连接
		gPtrExauRpc = new CRpcWrapper(gPtrConfig->m_ExauCfg.host, gPtrConfig->m_ExauCfg.port,
                                       gPtrConfig->m_ExauCfg.user, gPtrConfig->m_ExauCfg.pswd,
                                       gPtrConfig->m_ExauCfg.overtime);
		// 绑定查询服务连接
		gPtrBindQueryRpc = new CRpcWrapper(gPtrConfig->m_BindQueryCfg.host, gPtrConfig->m_BindQueryCfg.port,
                                       gPtrConfig->m_BindQueryCfg.user, gPtrConfig->m_BindQueryCfg.pswd,
                                       gPtrConfig->m_BindQueryCfg.overtime);
       
         // 创建fund_fetch连接
         gPtrFundFetchRpc  = new CRpcWrapper(gPtrConfig->m_FundFetchCfg.host, gPtrConfig->m_FundFetchCfg.port,
                                   gPtrConfig->m_FundFetchCfg.user, gPtrConfig->m_FundFetchCfg.pswd,
                                   gPtrConfig->m_FundFetchCfg.overtime);

         //初始化session服务
         gPtrSession = new CSessionApi();

		// 初始化子账户数据库连接
		initSubaccConf();

		// 初始化ckv cache 连接
		gCkvSvrOperator = new CkvSvrOperator();
		gCkvSvrOperator->init();
        
        // 初始化子账户ckv cache 连接
        gSubAccCkvSvrOperator = new CkvSvrOperator();
        gSubAccCkvSvrOperator->init_sub_acc_ckv();

		//初始化配置中心白名单
		g_user_classify = new UserClassify();
		g_user_classify->init("fund_deal_server", gPtrConfig->m_AppCfg.switch_conf_center);

		//初始化relayclient
		gPtrRelayClient = new tenpaymq::CRelayClient(gPtrConfig->m_MqCfg.host.c_str(), gPtrConfig->m_MqCfg.port, gPtrConfig->m_MqCfg.concurrent, gPtrConfig->m_MqCfg.overtime);
		//gPtrRelayClient = new tenpaymq::CRelayClient("10.12.196.175", 22000, 10, 1000);
    }
    catch (CException& e)
    {
        if (gPtrSysLog) {
            gPtrSysLog->error("[%s][%d]DECLARE_SO_INIT: %d, %s", e.file(), e.line(), e.error(), e.what());
        } else {
            trpc_error_log("Failed to start fund_deal_server:[%s][%d] %d, %s!\n",  e.file(), e.line(), e.error(), e.what());
        }

        throw;
    }

    // 打印启动日志
    gPtrSysLog->normal("fund_deal_server start ok ...");
}


DECLARE_SO_FINI()
{
    // 打印关闭日志
    gPtrSysLog->normal("fund_deal_server stop!");

    // 关闭数据库连接
    delete gPtrFundDB;

	delete gPtrFundSlaveDB;

	freeSubaccDb();

    // 删除配置
    delete gPtrConfig;

	delete gPtrSubaccRpc;

	delete gPtrExauRpc;

	// 删除ckv
    delete gCkvSvrOperator;
    delete gSubAccCkvSvrOperator;

    delete gPtrFundFetchRpc;

    delete gPtrSession;

	// 关闭日志
    delete gPtrAppLog;
    delete gPtrSysLog;
	delete gPtrCkvErrorLog;
    delete gPtrCheckLog;


	//关闭配置中心连接
	delete g_user_classify;

}


/**
 * 模板函数
 * @input: rqst   middle消息结构
 *             pRpc 远程连接句柄
 *             pTrans 事务管理器连接句柄
 */
template<class T>
void decalare_service(TRPC_SVCINFO * rqst)
{
    // 运行信息记录对象
    SerRunInfo SRI(rqst);

    try
    {

        // 定义服务实例
        T objService(gPtrFundDB);

        // 解析输入参数
        objService.parseInputMsg(rqst);

        // 执行业务逻辑
        objService.excute();

        // 打包返回结果
        objService.packReturnMsg(rqst);
    }
    catch(CException& e)
    {
        /* 记录错误日志*/
        gPtrAppLog->error("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        /* 打包返回失败结果 */
        packMessage(e.error(), e.what(), rqst);
    }

    /* 返回消息*/
    trpc_return(rqst, 0);
}

template<class T>
void decalare_service_with_para(TRPC_SVCINFO * rqst,int para)
{
    // 运行信息记录对象
    SerRunInfo SRI(rqst);

    try
    {

        // 定义服务实例
        T objService(gPtrFundDB,para);

        // 解析输入参数
        objService.parseInputMsg(rqst);

        // 执行业务逻辑
        objService.excute();

        // 打包返回结果
        objService.packReturnMsg(rqst);
    }
    catch(CException& e)
    {
        /* 记录错误日志*/
        gPtrAppLog->error("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        /* 打包返回失败结果 */
        packMessage(e.error(), e.what(), rqst);
    }

    /* 返回消息*/
    trpc_return(rqst, 0);
}

/**
  * Description: 基金开户接口
  */
DECLARE_SERVICE(fundacc_bind_sp_service)
{
	decalare_service<FundBindSpAcc>(rqst);
}

/**
  * Description: 申购请求
  */
DECLARE_SERVICE(fund_buy_sp_req_service)
{
	decalare_service<FundBuySpReq>(rqst);
}

/**
  * Description: 申购确认
  */
DECLARE_SERVICE(fund_buy_sp_ack_service)
{
	decalare_service<FundBuySpAck>(rqst);
}

/**
  * Description: 赎回请求
  */
DECLARE_SERVICE(fund_redem_sp_req_service)
{
	decalare_service<FundRedemSpReq>(rqst);
}

/**
  * Description: 赎回确认
  */
DECLARE_SERVICE(fund_redem_sp_ack_service)
{
	decalare_service<FundRedemSpAck>(rqst);
}

/**
  * Description: 申购请求
  */
DECLARE_SERVICE(fund_buy_req_service)
{
	decalare_service<FactoryBuySpReq>(rqst);
}

/**
  * Description: 申购确认
  */
DECLARE_SERVICE(fund_buy_ack_service)
{
	decalare_service<FactoryBuySpAck>(rqst);
}
/**
  * Description: 申购确认
  */
DECLARE_SERVICE(fund_buy_ack_batch_service)
{
	decalare_service<FactoryBuySpAck>(rqst);
}

/**
  * Description: 赎回请求
  */
DECLARE_SERVICE(fund_redeem_req_service)
{
	decalare_service<FactoryRedeemSpReq>(rqst);
}

/**
  * Description: 赎回确认
  */
DECLARE_SERVICE(fund_redeem_ack_service)
{
	decalare_service<FactoryRedeemSpAck>(rqst);
}

/**
  * Description: 赎回确认
  */
DECLARE_SERVICE(fund_redeem_ack_batch_service)
{
	decalare_service<FactoryRedeemSpAck>(rqst);
}

/**
  * Description: 客服強赎
  */
DECLARE_SERVICE(fund_kf_redem_sp_req_service)
{
	decalare_service<FundKfRedemSpReq>(rqst);
}


/**
  * Description: 基金公司账户冻结解冻
  */
DECLARE_SERVICE(fund_spaccount_freeze_service)
{
	decalare_service<FundSpAccFreeze>(rqst);
}

/**
  * Description: 基金收益入账
  */
DECLARE_SERVICE(fund_reg_daily_profit_service)
{
	decalare_service<FundRegProfit>(rqst);
}

/**
  * Description: 非实名认证用户开户前预支付请求接口
  */
DECLARE_SERVICE(fund_prepay_req_service)
{
	decalare_service<FundPrepayReq>(rqst);
}

/**
  * Description: 非实名认证用户开户前预支付确认接口
  */
DECLARE_SERVICE(fund_prepay_ack_service)
{
	decalare_service<FundPrepayAck>(rqst);
}

/**
  * Description: 查询余额增值账户绑定信息
  */
DECLARE_SERVICE(fund_query_bind_sp_service)
{
	decalare_service<FundQueryBindSp>(rqst);
}


/**
  * Description: 查询累计收益
  */
DECLARE_SERVICE(fund_query_total_profit_service)
{
	decalare_service<FundQueryTotalProfit>(rqst);
}

/**
  * Description: 查询累计收益
  */
DECLARE_SERVICE(fund_qry_profit_chklgi_service)
{
	decalare_service_with_para<FundQueryTotalProfit>(rqst,CHECK_LOGIN);
}

/**
  * Description: 查询余额增值账户余额
  */
DECLARE_SERVICE(fund_query_balance_service)
{
	decalare_service<FundQueryBalance>(rqst);
}

/**
  * Description: 查询余额增值账户余额
  */
DECLARE_SERVICE(fund_qry_balance_chklgi_service)
{
	decalare_service_with_para<FundQueryBalance>(rqst,CHECK_LOGIN);
}

/**
  * Description: 查询是否可进行份额转换

DECLARE_SERVICE(fund_query_changesp_limit_service)
{
	decalare_service<FundQueryChangespLimit>(rqst);
}
*/

/**
  * Description: 查询推荐基金公司
  */
DECLARE_SERVICE(fund_query_recommend_sp_service)
{
	decalare_service<FundQueryRecommendSp>(rqst);
}


/**
  * Description: 更新kv-cache
  */
DECLARE_SERVICE(fund_update_kvcache_service)
{
	decalare_service<FundUpdateKvcache>(rqst);
}

/**
  * Description: 更新kv-cache
  */
DECLARE_SERVICE(fund_update_kvcache_chk_service)
{
	decalare_service_with_para<FundUpdateKvcache>(rqst,CHECK_LOGIN);
}

/**
  * Description: 更新kv-cache(新增request_type)
  */
DECLARE_SERVICE(fund_deal_update_kvcache_c)
{
	decalare_service<FundUpdateKvcache>(rqst);
}


/**
  * Description: 更新基金公司收益率
  */
DECLARE_SERVICE(fund_reg_profit_rate_service)
{
	decalare_service_with_para<FundRegProfitRate>(rqst,OP_TYPE_REG_RATE_FROM_PARTNER);
}

/**
  * Description: 更新外部接口收益率
  */
DECLARE_SERVICE(fund_reg_outer_rate_service)
{
	decalare_service_with_para<FundRegProfitRate>(rqst,OP_TYPE_REG_RATE_FROM_OUTER);
}

/**
  * Description: 更新用户支付卡
  */
DECLARE_SERVICE(fund_update_pay_card_service)
{
	decalare_service<FundUpdatePayCard>(rqst);
}

/**
  * Description: 子账户补单
  */
DECLARE_SERVICE(fund_subacc_budan_service)
{
	decalare_service<FundSubaccBudan>(rqst);
}


/**
  * Description: 重置用户支付卡
  */
DECLARE_SERVICE(fund_nopass_reset_paycard_service)
{
	decalare_service<FundNopassResetPayCard>(rqst);
}

/**
  * Description: 更新银行配置信息
  */
DECLARE_SERVICE(fund_update_bank_config_service)
{
	decalare_service<FundUpdateBankConfig>(rqst);
}


/**
  * Description: 用户注销校验
  */
DECLARE_SERVICE(fundacc_unbind_check_service)
{
	decalare_service_with_para<FundUnbindAcc>(rqst,1);
}


/**
  * Description: 用户注销校验
  */
DECLARE_SERVICE(fundacc_unbind_service)
{
	decalare_service_with_para<FundUnbindAcc>(rqst,2);
}

/**
  * Description: 定期基金收益入账
  */
DECLARE_SERVICE(fund_reg_close_profit_service)
{
	decalare_service<FundRegCloseProfit>(rqst);
}

/**
  * Description: 定期基金变更预约收回信息
  */
DECLARE_SERVICE(fund_update_end_type)
{
	decalare_service<FundUpdateEndType>(rqst);
}

/**
  * Description: 份额转换请求
  */
DECLARE_SERVICE(fund_transfer_req_service)
{
	decalare_service<FundTransferReq>(rqst);
}

/**
  * Description: 份额转换确认
  */
DECLARE_SERVICE(fund_transfer_ack_service)
{
	decalare_service<FundTransferAck>(rqst);
}

/**
  * Description: 份额预冻结请求
  */
DECLARE_SERVICE(fund_balance_pre_freeze_service)
{
	decalare_service<FundBalancePreFreeze>(rqst);
}

/**
  * Description: 份额冻结请求
  */
DECLARE_SERVICE(fund_balance_freeze_service)
{
	decalare_service<FundBalanceFreeze>(rqst);
}

/**
  * Description: 份额解冻请求
  */
DECLARE_SERVICE(fund_balance_unfreeze_service)
{
	decalare_service<FundBalanceUnFreeze>(rqst);
}


/**
  * Description: 查询
  */
DECLARE_SERVICE(fund_commquery_chklgi_service)
{
       decalare_service_with_para<FundComQryChkLgi>(rqst,CHECK_LOGIN);
}

/**
  * Description: 查询不验证登录，不能通过relay调用,只能查询单笔
  */
DECLARE_SERVICE(fund_commquery_simple_service)
{
       decalare_service<FundComQryChkLgi>(rqst);
}


/**
  * Description: 定期到期赎回
  */
DECLARE_SERVICE(fund_close_end_redem_service)
{
	decalare_service<FundCloseEndRedem>(rqst);
}


/**
  * Description: 定期期末赎回文件入账

DECLARE_SERVICE(fund_reg_close_end_redem_service)
{
	decalare_service<FundRegCloseEndRedem>(rqst);
}
*/


/**
  * Description: 定期期末滚动
  */
DECLARE_SERVICE(fund_close_roll_service)
{
	decalare_service<FundCloseRoll>(rqst);
}


/**
  * Description: 定期实时赎回申请
  */
DECLARE_SERVICE(fund_redem_close_req_service)
{
	decalare_service<FundRedemCloseReq>(rqst);
}


/**
  * Description: 定期实时赎回申请
  */
DECLARE_SERVICE(fund_redem_close_ack_service)
{
	decalare_service<FundRedemCloseAck>(rqst);
}

/**
  * Description: 理财通余额提现请求
  */
DECLARE_SERVICE(fund_ba_fetch_req_service)
{
	decalare_service<FundFetchReq>(rqst);
}

/**
  * Description: 理财通余额提现确认
  */
DECLARE_SERVICE(fund_ba_fetch_ack_service)
{
	decalare_service<FundFetchAck>(rqst);
}

/**
  * Description: 理财通余额充值请求
  */
DECLARE_SERVICE(fund_ba_charge_req_service)
{
	decalare_service<FundChargeReq>(rqst);
}

/**
  * Description: 理财通余额充值确认
  */
DECLARE_SERVICE(fund_ba_charge_ack_service)
{
	decalare_service<FundChargeAck>(rqst);
}

/**
  * Description: 查询余额流水单
  */
DECLARE_SERVICE(fund_qry_balance_roll_service)
{
	decalare_service<FundQryBalanceOrder>(rqst);
}

/**
  * Description: 余额回补
  */
DECLARE_SERVICE(fund_balance_recover_service)
{
	decalare_service<FundBalanceRecover>(rqst);
}

/**
  * Description: 理财通后台管理系统配置管理
  */
DECLARE_SERVICE(fund_manage_config_service)
{
	decalare_service<FundManageConfig>(rqst);
}

/**
  * Description: 定期周期注册

DECLARE_SERVICE(fund_reg_close_cycle_service)
{
	decalare_service<FundRegCloseCycle>(rqst);
}
*/

/**
  * Description: CKV对账
  */
DECLARE_SERVICE(fund_check_ckv_service)
{
	decalare_service<FundCheckCkv>(rqst);
}

/**
  * Description: 保存前置理财通账户信息
  */
DECLARE_SERVICE(fund_pre_record_useracc_service)
{
	decalare_service<FundPreRecordUserAcc>(rqst);
}

/**
  * Description: 风险评测接口
  */
DECLARE_SERVICE(fund_risk_assess_service)
{
       decalare_service<FundRiskAssess>(rqst);
}

/**
  * Description: 余额提现冻结通知确认接口
  */
DECLARE_SERVICE(fund_ba_fetch_notify_service)
{
       decalare_service<FundFetchNotify>(rqst);
}

/**
  * Description: 退款接口
  */
DECLARE_SERVICE(fund_refund_service)
{
       decalare_service<FundRefund>(rqst);
}
/**
  * Description: 指数基金入账
  */
DECLARE_SERVICE(fund_reg_exp_profit_service)
{
	decalare_service<FundRegExpProfit>(rqst);
}

/**
  * Description: 赎回到账确认
  */
DECLARE_SERVICE(fund_redeem_fetch_ack_service)
{
	decalare_service<FundRedeemFetchAck>(rqst);
}

/**
  * Description: 更新用户信息
  */
DECLARE_SERVICE(fund_update_bind_service)
{
	decalare_service<FundUpdateBind>(rqst);
}
/**
  * Description: 冻结理财通账户
  */
DECLARE_SERVICE(fund_account_freeze_service)
{
	decalare_service<FundAccountFreeze>(rqst);
}

/**
  * Description: 插入资讯信息
  */
DECLARE_SERVICE(fund_insert_infomation_service)
{
	decalare_service<FundInsertInfomation>(rqst);
}

/**
  * Description: 插入资讯信息
  */
DECLARE_SERVICE(fund_update_infomation_service)
{
	decalare_service<FundUpdateInfomation>(rqst);
}

/**
  * Description: 插入交易日信息
  */
DECLARE_SERVICE(fund_deal_insert_trans_date_c)
{
	decalare_service<FundInsertTransDate>(rqst);
}


/**
  * Description: 更新交易日信息
  */
DECLARE_SERVICE(fund_deal_update_trans_date_c)
{
	decalare_service<FundUpdateTransDate>(rqst);
}
/**
  * Description: 单元测试
  */
DECLARE_SERVICE(fund_deal_test)
{
	decalare_service<FundDealTest>(rqst);
}

