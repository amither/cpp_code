/**
  * FileName:user_classify.h
  * Author: rajeshzhou
  * Version :1.0
  * Date: 2014-06-25
  * Description: 用于用户分类
  */
#ifndef _USER_CLASSIFY_H_
#define _USER_CLASSIFY_H_

#include "exception.h"
#include "common.h"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include "config_center.h"



class UserClassify
{
public:
	UserClassify();  
	void init(string svr_name,  int switch_conf) throw(CException);
	typedef  enum user_type { NORMAL, OUT_VIP, TENCENT_VIP} UT;
	UT getUserType(const string &uin) throw(CException);

public:
	//enum user_type { NORMAL, OUT_VIP, TENCENT_VIP};

private:
	void loadTypeData();
	void loadTypeDataFromLocal();
	bool inUserType(vector<string> &type_data, const string &uin);
private:
	string m_svr_name;
	bool is_loaded;
	bool is_conf_center;
	
	ConfigCenter m_conf_center; //配置中心句柄
	vector<string> m_vip_list; //公司外申购白名单
	vector<string> m_tencent_vip_list; //公司内申购白名单
	
};


#endif

