/**
  * FileName: fund_deal_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-16
  * Description: �����׷���
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

//middleȫ�����Ӿ��

CRpcWrapper* gPtrSubaccRpc = NULL; //���˻���������
CRpcWrapper* gPtrExauRpc = NULL; //exau_check_server��������
CRpcWrapper* gPtrBindQueryRpc = NULL; //�����п���ѯ����������
CftLog* gPtrAppLog = NULL; // Ӧ����־���
CftLog* gPtrCkvErrorLog = NULL; // ����ckv������־
CftLog* gPtrCkvSlaveErrorLog = NULL; // ���±�ckv������־
CftLog* gPtrSysLog = NULL; // ϵͳ��־���
CftLog* gPtrCheckLog = NULL; // �����־���
GlobalConfig* gPtrConfig = NULL; // ȫ�����þ��
CMySQL  *gPtrFundDB = NULL; //���ݿ����Ӿ��
CMySQL  *gPtrFundSlaveDB = NULL; //�������ݿⱸ�����Ӿ��
CkvSvrOperator *gCkvSvrOperator = NULL; //ckv cache ���ʾ��
CkvSvrOperator *gSubAccCkvSvrOperator = NULL; //���˻� ckv cache ���ʾ��
map<string, CMySQL*> subaccDB;//���˻����ݿ���
CRpcWrapper* gPtrFundFetchRpc = NULL; //fund_fetch_server ��������
CSessionApi* gPtrSession = NULL; // session����ָ��
tenpaymq::CRelayClient* gPtrRelayClient = NULL;//ȫ��relayclient

UserClassify *g_user_classify = NULL; //�������ĵ��û����������


/**
 * ��ӡ������Ϣ
 * @input:  szLog ��־��Ϣ
 */
void DEBUG(const char* szLog)
{
    gPtrAppLog->debug("%s", szLog);
}

/**
 * ��ӡ������Ϣ
 * @input:  szLog ��־��Ϣ
 */
void ERROR(const char* szLog)
{
    gPtrAppLog->error("%s", szLog);
}



/**
 * ��������Ϣ
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
 * Description: ��ʼ�����ݿ�
 */
CMySQL  * initFunddealDb(GlobalConfig::PassconfDB& passconfDB)
{
	char szIdata[10 * 1024] = {0};
    char szOdata[10 * 1024] = {0};
    size_t oLen = sizeof(szOdata);
	// ���ò���
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
	// ��ʼ�����ݿ�����
    return new CMySQL(szHost, szUser, szPswd, iPort, iOvertime, szCharset, "fund_db", gPtrConfig->m_SysCfg.db_idle_time);


	/*
    CDbConfig dbConfig("fund_deal_server", SERVER_VERSION_SIGN);

	TRACE_NORMAL("initFunddealDb. subacc_db_index[%s] role[%s] ", gPtrConfig->m_fund_deal_db.db_conf_key.c_str(),
				gPtrConfig->m_fund_deal_db.db_conf_role.c_str());

    //���fund_db conf_key
    dbConfig.addDbName(gPtrConfig->m_fund_deal_db.db_conf_key, gPtrConfig->m_fund_deal_db.db_conf_role);
	TRACE_NORMAL("1");

    //��ȡ���ݿ�����
    dbConfig.executeBatchDbQuery();
	TRACE_NORMAL("2");

    //��ʼ��ocall_db
    DbHostCfg dbConfigItem = dbConfig.getDbConfigure(gPtrConfig->m_fund_deal_db.db_conf_key, gPtrConfig->m_fund_deal_db.db_conf_role);

    gPtrSysLog->debug("zwdb conf[%s|%s][%s]", dbConfigItem.szConfKey, dbConfigItem.szConfRole, dbConfigItem.szHost);


	// ��ʼ�����ݿ�����
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
		// ���ò���
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
		// ��ʼ�����ݿ�����
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
		// ��ʼ�������ļ�
        gPtrConfig = new GlobalConfig("/usr/local/middle/fund_deal_server/conf/fund_deal_server.xml");
		trpc_debug_log(">>> end load config");

		// ��ʼ����־ָ��
        gPtrAppLog = new CftLog((gPtrConfig->m_SysCfg.log_path + "/fund_deal").c_str(),
                                gPtrConfig->m_SysCfg.log_size * 1024 * 1024, gPtrConfig->m_SysCfg.log_num,
                                CftLog::_DATE_MODE);
        gPtrAppLog->setSuffix("_app");

		//����ckv�쳣������д��
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

		// ��ӡ������־
		gPtrSysLog->normal("fund_deal_server start ...");

        // ��ʼ�����ݿ�
		gPtrFundDB = initFunddealDb(gPtrConfig->m_fund_deal_db);
		gPtrFundSlaveDB = initFunddealDb(gPtrConfig->m_fund_deal_slave_db);

		// �������˻�����
        gPtrSubaccRpc = new CRpcWrapper(gPtrConfig->m_SubaccCfg.host, gPtrConfig->m_SubaccCfg.port,
                                       gPtrConfig->m_SubaccCfg.user, gPtrConfig->m_SubaccCfg.pswd,
                                       gPtrConfig->m_SubaccCfg.overtime);

		// ����exau����
		gPtrExauRpc = new CRpcWrapper(gPtrConfig->m_ExauCfg.host, gPtrConfig->m_ExauCfg.port,
                                       gPtrConfig->m_ExauCfg.user, gPtrConfig->m_ExauCfg.pswd,
                                       gPtrConfig->m_ExauCfg.overtime);
		// �󶨲�ѯ��������
		gPtrBindQueryRpc = new CRpcWrapper(gPtrConfig->m_BindQueryCfg.host, gPtrConfig->m_BindQueryCfg.port,
                                       gPtrConfig->m_BindQueryCfg.user, gPtrConfig->m_BindQueryCfg.pswd,
                                       gPtrConfig->m_BindQueryCfg.overtime);
       
         // ����fund_fetch����
         gPtrFundFetchRpc  = new CRpcWrapper(gPtrConfig->m_FundFetchCfg.host, gPtrConfig->m_FundFetchCfg.port,
                                   gPtrConfig->m_FundFetchCfg.user, gPtrConfig->m_FundFetchCfg.pswd,
                                   gPtrConfig->m_FundFetchCfg.overtime);

         //��ʼ��session����
         gPtrSession = new CSessionApi();

		// ��ʼ�����˻����ݿ�����
		initSubaccConf();

		// ��ʼ��ckv cache ����
		gCkvSvrOperator = new CkvSvrOperator();
		gCkvSvrOperator->init();
        
        // ��ʼ�����˻�ckv cache ����
        gSubAccCkvSvrOperator = new CkvSvrOperator();
        gSubAccCkvSvrOperator->init_sub_acc_ckv();

		//��ʼ���������İ�����
		g_user_classify = new UserClassify();
		g_user_classify->init("fund_deal_server", gPtrConfig->m_AppCfg.switch_conf_center);

		//��ʼ��relayclient
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

    // ��ӡ������־
    gPtrSysLog->normal("fund_deal_server start ok ...");
}


DECLARE_SO_FINI()
{
    // ��ӡ�ر���־
    gPtrSysLog->normal("fund_deal_server stop!");

    // �ر����ݿ�����
    delete gPtrFundDB;

	delete gPtrFundSlaveDB;

	freeSubaccDb();

    // ɾ������
    delete gPtrConfig;

	delete gPtrSubaccRpc;

	delete gPtrExauRpc;

	// ɾ��ckv
    delete gCkvSvrOperator;
    delete gSubAccCkvSvrOperator;

    delete gPtrFundFetchRpc;

    delete gPtrSession;

	// �ر���־
    delete gPtrAppLog;
    delete gPtrSysLog;
	delete gPtrCkvErrorLog;
    delete gPtrCheckLog;


	//�ر�������������
	delete g_user_classify;

}


/**
 * ģ�庯��
 * @input: rqst   middle��Ϣ�ṹ
 *             pRpc Զ�����Ӿ��
 *             pTrans ������������Ӿ��
 */
template<class T>
void decalare_service(TRPC_SVCINFO * rqst)
{
    // ������Ϣ��¼����
    SerRunInfo SRI(rqst);

    try
    {

        // �������ʵ��
        T objService(gPtrFundDB);

        // �����������
        objService.parseInputMsg(rqst);

        // ִ��ҵ���߼�
        objService.excute();

        // ������ؽ��
        objService.packReturnMsg(rqst);
    }
    catch(CException& e)
    {
        /* ��¼������־*/
        gPtrAppLog->error("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        /* �������ʧ�ܽ�� */
        packMessage(e.error(), e.what(), rqst);
    }

    /* ������Ϣ*/
    trpc_return(rqst, 0);
}

template<class T>
void decalare_service_with_para(TRPC_SVCINFO * rqst,int para)
{
    // ������Ϣ��¼����
    SerRunInfo SRI(rqst);

    try
    {

        // �������ʵ��
        T objService(gPtrFundDB,para);

        // �����������
        objService.parseInputMsg(rqst);

        // ִ��ҵ���߼�
        objService.excute();

        // ������ؽ��
        objService.packReturnMsg(rqst);
    }
    catch(CException& e)
    {
        /* ��¼������־*/
        gPtrAppLog->error("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        /* �������ʧ�ܽ�� */
        packMessage(e.error(), e.what(), rqst);
    }

    /* ������Ϣ*/
    trpc_return(rqst, 0);
}

/**
  * Description: ���𿪻��ӿ�
  */
DECLARE_SERVICE(fundacc_bind_sp_service)
{
	decalare_service<FundBindSpAcc>(rqst);
}

/**
  * Description: �깺����
  */
DECLARE_SERVICE(fund_buy_sp_req_service)
{
	decalare_service<FundBuySpReq>(rqst);
}

/**
  * Description: �깺ȷ��
  */
DECLARE_SERVICE(fund_buy_sp_ack_service)
{
	decalare_service<FundBuySpAck>(rqst);
}

/**
  * Description: �������
  */
DECLARE_SERVICE(fund_redem_sp_req_service)
{
	decalare_service<FundRedemSpReq>(rqst);
}

/**
  * Description: ���ȷ��
  */
DECLARE_SERVICE(fund_redem_sp_ack_service)
{
	decalare_service<FundRedemSpAck>(rqst);
}

/**
  * Description: �깺����
  */
DECLARE_SERVICE(fund_buy_req_service)
{
	decalare_service<FactoryBuySpReq>(rqst);
}

/**
  * Description: �깺ȷ��
  */
DECLARE_SERVICE(fund_buy_ack_service)
{
	decalare_service<FactoryBuySpAck>(rqst);
}
/**
  * Description: �깺ȷ��
  */
DECLARE_SERVICE(fund_buy_ack_batch_service)
{
	decalare_service<FactoryBuySpAck>(rqst);
}

/**
  * Description: �������
  */
DECLARE_SERVICE(fund_redeem_req_service)
{
	decalare_service<FactoryRedeemSpReq>(rqst);
}

/**
  * Description: ���ȷ��
  */
DECLARE_SERVICE(fund_redeem_ack_service)
{
	decalare_service<FactoryRedeemSpAck>(rqst);
}

/**
  * Description: ���ȷ��
  */
DECLARE_SERVICE(fund_redeem_ack_batch_service)
{
	decalare_service<FactoryRedeemSpAck>(rqst);
}

/**
  * Description: �ͷ�����
  */
DECLARE_SERVICE(fund_kf_redem_sp_req_service)
{
	decalare_service<FundKfRedemSpReq>(rqst);
}


/**
  * Description: ����˾�˻�����ⶳ
  */
DECLARE_SERVICE(fund_spaccount_freeze_service)
{
	decalare_service<FundSpAccFreeze>(rqst);
}

/**
  * Description: ������������
  */
DECLARE_SERVICE(fund_reg_daily_profit_service)
{
	decalare_service<FundRegProfit>(rqst);
}

/**
  * Description: ��ʵ����֤�û�����ǰԤ֧������ӿ�
  */
DECLARE_SERVICE(fund_prepay_req_service)
{
	decalare_service<FundPrepayReq>(rqst);
}

/**
  * Description: ��ʵ����֤�û�����ǰԤ֧��ȷ�Ͻӿ�
  */
DECLARE_SERVICE(fund_prepay_ack_service)
{
	decalare_service<FundPrepayAck>(rqst);
}

/**
  * Description: ��ѯ�����ֵ�˻�����Ϣ
  */
DECLARE_SERVICE(fund_query_bind_sp_service)
{
	decalare_service<FundQueryBindSp>(rqst);
}


/**
  * Description: ��ѯ�ۼ�����
  */
DECLARE_SERVICE(fund_query_total_profit_service)
{
	decalare_service<FundQueryTotalProfit>(rqst);
}

/**
  * Description: ��ѯ�ۼ�����
  */
DECLARE_SERVICE(fund_qry_profit_chklgi_service)
{
	decalare_service_with_para<FundQueryTotalProfit>(rqst,CHECK_LOGIN);
}

/**
  * Description: ��ѯ�����ֵ�˻����
  */
DECLARE_SERVICE(fund_query_balance_service)
{
	decalare_service<FundQueryBalance>(rqst);
}

/**
  * Description: ��ѯ�����ֵ�˻����
  */
DECLARE_SERVICE(fund_qry_balance_chklgi_service)
{
	decalare_service_with_para<FundQueryBalance>(rqst,CHECK_LOGIN);
}

/**
  * Description: ��ѯ�Ƿ�ɽ��зݶ�ת��

DECLARE_SERVICE(fund_query_changesp_limit_service)
{
	decalare_service<FundQueryChangespLimit>(rqst);
}
*/

/**
  * Description: ��ѯ�Ƽ�����˾
  */
DECLARE_SERVICE(fund_query_recommend_sp_service)
{
	decalare_service<FundQueryRecommendSp>(rqst);
}


/**
  * Description: ����kv-cache
  */
DECLARE_SERVICE(fund_update_kvcache_service)
{
	decalare_service<FundUpdateKvcache>(rqst);
}

/**
  * Description: ����kv-cache
  */
DECLARE_SERVICE(fund_update_kvcache_chk_service)
{
	decalare_service_with_para<FundUpdateKvcache>(rqst,CHECK_LOGIN);
}

/**
  * Description: ����kv-cache(����request_type)
  */
DECLARE_SERVICE(fund_deal_update_kvcache_c)
{
	decalare_service<FundUpdateKvcache>(rqst);
}


/**
  * Description: ���»���˾������
  */
DECLARE_SERVICE(fund_reg_profit_rate_service)
{
	decalare_service_with_para<FundRegProfitRate>(rqst,OP_TYPE_REG_RATE_FROM_PARTNER);
}

/**
  * Description: �����ⲿ�ӿ�������
  */
DECLARE_SERVICE(fund_reg_outer_rate_service)
{
	decalare_service_with_para<FundRegProfitRate>(rqst,OP_TYPE_REG_RATE_FROM_OUTER);
}

/**
  * Description: �����û�֧����
  */
DECLARE_SERVICE(fund_update_pay_card_service)
{
	decalare_service<FundUpdatePayCard>(rqst);
}

/**
  * Description: ���˻�����
  */
DECLARE_SERVICE(fund_subacc_budan_service)
{
	decalare_service<FundSubaccBudan>(rqst);
}


/**
  * Description: �����û�֧����
  */
DECLARE_SERVICE(fund_nopass_reset_paycard_service)
{
	decalare_service<FundNopassResetPayCard>(rqst);
}

/**
  * Description: ��������������Ϣ
  */
DECLARE_SERVICE(fund_update_bank_config_service)
{
	decalare_service<FundUpdateBankConfig>(rqst);
}


/**
  * Description: �û�ע��У��
  */
DECLARE_SERVICE(fundacc_unbind_check_service)
{
	decalare_service_with_para<FundUnbindAcc>(rqst,1);
}


/**
  * Description: �û�ע��У��
  */
DECLARE_SERVICE(fundacc_unbind_service)
{
	decalare_service_with_para<FundUnbindAcc>(rqst,2);
}

/**
  * Description: ���ڻ�����������
  */
DECLARE_SERVICE(fund_reg_close_profit_service)
{
	decalare_service<FundRegCloseProfit>(rqst);
}

/**
  * Description: ���ڻ�����ԤԼ�ջ���Ϣ
  */
DECLARE_SERVICE(fund_update_end_type)
{
	decalare_service<FundUpdateEndType>(rqst);
}

/**
  * Description: �ݶ�ת������
  */
DECLARE_SERVICE(fund_transfer_req_service)
{
	decalare_service<FundTransferReq>(rqst);
}

/**
  * Description: �ݶ�ת��ȷ��
  */
DECLARE_SERVICE(fund_transfer_ack_service)
{
	decalare_service<FundTransferAck>(rqst);
}

/**
  * Description: �ݶ�Ԥ��������
  */
DECLARE_SERVICE(fund_balance_pre_freeze_service)
{
	decalare_service<FundBalancePreFreeze>(rqst);
}

/**
  * Description: �ݶ������
  */
DECLARE_SERVICE(fund_balance_freeze_service)
{
	decalare_service<FundBalanceFreeze>(rqst);
}

/**
  * Description: �ݶ�ⶳ����
  */
DECLARE_SERVICE(fund_balance_unfreeze_service)
{
	decalare_service<FundBalanceUnFreeze>(rqst);
}


/**
  * Description: ��ѯ
  */
DECLARE_SERVICE(fund_commquery_chklgi_service)
{
       decalare_service_with_para<FundComQryChkLgi>(rqst,CHECK_LOGIN);
}

/**
  * Description: ��ѯ����֤��¼������ͨ��relay����,ֻ�ܲ�ѯ����
  */
DECLARE_SERVICE(fund_commquery_simple_service)
{
       decalare_service<FundComQryChkLgi>(rqst);
}


/**
  * Description: ���ڵ������
  */
DECLARE_SERVICE(fund_close_end_redem_service)
{
	decalare_service<FundCloseEndRedem>(rqst);
}


/**
  * Description: ������ĩ����ļ�����

DECLARE_SERVICE(fund_reg_close_end_redem_service)
{
	decalare_service<FundRegCloseEndRedem>(rqst);
}
*/


/**
  * Description: ������ĩ����
  */
DECLARE_SERVICE(fund_close_roll_service)
{
	decalare_service<FundCloseRoll>(rqst);
}


/**
  * Description: ����ʵʱ�������
  */
DECLARE_SERVICE(fund_redem_close_req_service)
{
	decalare_service<FundRedemCloseReq>(rqst);
}


/**
  * Description: ����ʵʱ�������
  */
DECLARE_SERVICE(fund_redem_close_ack_service)
{
	decalare_service<FundRedemCloseAck>(rqst);
}

/**
  * Description: ���ͨ�����������
  */
DECLARE_SERVICE(fund_ba_fetch_req_service)
{
	decalare_service<FundFetchReq>(rqst);
}

/**
  * Description: ���ͨ�������ȷ��
  */
DECLARE_SERVICE(fund_ba_fetch_ack_service)
{
	decalare_service<FundFetchAck>(rqst);
}

/**
  * Description: ���ͨ����ֵ����
  */
DECLARE_SERVICE(fund_ba_charge_req_service)
{
	decalare_service<FundChargeReq>(rqst);
}

/**
  * Description: ���ͨ����ֵȷ��
  */
DECLARE_SERVICE(fund_ba_charge_ack_service)
{
	decalare_service<FundChargeAck>(rqst);
}

/**
  * Description: ��ѯ�����ˮ��
  */
DECLARE_SERVICE(fund_qry_balance_roll_service)
{
	decalare_service<FundQryBalanceOrder>(rqst);
}

/**
  * Description: ���ز�
  */
DECLARE_SERVICE(fund_balance_recover_service)
{
	decalare_service<FundBalanceRecover>(rqst);
}

/**
  * Description: ���ͨ��̨����ϵͳ���ù���
  */
DECLARE_SERVICE(fund_manage_config_service)
{
	decalare_service<FundManageConfig>(rqst);
}

/**
  * Description: ��������ע��

DECLARE_SERVICE(fund_reg_close_cycle_service)
{
	decalare_service<FundRegCloseCycle>(rqst);
}
*/

/**
  * Description: CKV����
  */
DECLARE_SERVICE(fund_check_ckv_service)
{
	decalare_service<FundCheckCkv>(rqst);
}

/**
  * Description: ����ǰ�����ͨ�˻���Ϣ
  */
DECLARE_SERVICE(fund_pre_record_useracc_service)
{
	decalare_service<FundPreRecordUserAcc>(rqst);
}

/**
  * Description: ��������ӿ�
  */
DECLARE_SERVICE(fund_risk_assess_service)
{
       decalare_service<FundRiskAssess>(rqst);
}

/**
  * Description: ������ֶ���֪ͨȷ�Ͻӿ�
  */
DECLARE_SERVICE(fund_ba_fetch_notify_service)
{
       decalare_service<FundFetchNotify>(rqst);
}

/**
  * Description: �˿�ӿ�
  */
DECLARE_SERVICE(fund_refund_service)
{
       decalare_service<FundRefund>(rqst);
}
/**
  * Description: ָ����������
  */
DECLARE_SERVICE(fund_reg_exp_profit_service)
{
	decalare_service<FundRegExpProfit>(rqst);
}

/**
  * Description: ��ص���ȷ��
  */
DECLARE_SERVICE(fund_redeem_fetch_ack_service)
{
	decalare_service<FundRedeemFetchAck>(rqst);
}

/**
  * Description: �����û���Ϣ
  */
DECLARE_SERVICE(fund_update_bind_service)
{
	decalare_service<FundUpdateBind>(rqst);
}
/**
  * Description: �������ͨ�˻�
  */
DECLARE_SERVICE(fund_account_freeze_service)
{
	decalare_service<FundAccountFreeze>(rqst);
}

/**
  * Description: ������Ѷ��Ϣ
  */
DECLARE_SERVICE(fund_insert_infomation_service)
{
	decalare_service<FundInsertInfomation>(rqst);
}

/**
  * Description: ������Ѷ��Ϣ
  */
DECLARE_SERVICE(fund_update_infomation_service)
{
	decalare_service<FundUpdateInfomation>(rqst);
}

/**
  * Description: ���뽻������Ϣ
  */
DECLARE_SERVICE(fund_deal_insert_trans_date_c)
{
	decalare_service<FundInsertTransDate>(rqst);
}


/**
  * Description: ���½�������Ϣ
  */
DECLARE_SERVICE(fund_deal_update_trans_date_c)
{
	decalare_service<FundUpdateTransDate>(rqst);
}
/**
  * Description: ��Ԫ����
  */
DECLARE_SERVICE(fund_deal_test)
{
	decalare_service<FundDealTest>(rqst);
}

