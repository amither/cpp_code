/**
  * FileName: fund_manage_config_service.h
  * Author: elijahyang
  * Version :1.0
  * Date: 2014-12-18
  * Description: 理财通后台管理系统配置管理接口
  */

#ifndef _FUND_MANAGE_CONFIG_H_
#define _FUND_MANAGE_CONFIG_H_

class FundManageConfig
{
public:
	FundManageConfig(CMySQL* mysql);

	void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
	void excute()  throw (CException);
	void packReturnMsg(TRPC_SVCINFO* rqst);
private:
	// 检查参数
	void CheckParams() throw (CException);
	string GenFundToken();
       void CheckToken() throw (CException); 
	void CheckWhiteListParams() throw(CException);
	void CheckSpConfigParams() throw(CException);
	void CheckUserAssetLimitParams() throw(CException);
	void CheckPayCardParams() throw(CException);

	// 管理配置信息
	void ManageConfig() throw (CException);

	// 配置白名单
	void ConfigWhiteList() throw (CException);
	int setWhiteListToCkv();
	int getWhiteListFromCkv();
	int delWhiteListByKey();

	// 配置基金公司购买限额等配置
	void ConfigSpConfig() throw (CException);
	// 配置用户总资产限额
	void ConfigUserAssetLimit() throw (CException);
	// 配置安全卡
	void ConfigPayCard() throw (CException);


	void DeleteUserPayCard() throw (CException);

      void updateUserWhiteList();
      void CheckUserWhiteParams();
private:
	CParams m_params;		// 消息参数
	string m_spid;			// 商户SPID
	CMySQL* m_pFundCon;	// 基金数据库连接句柄
};

#endif 

