/**
  * FileName:config_center.h
  * Author: rajeshzhou
  * Version :1.0
  * Date: 2014-06-25
  * Description: 用于配置中心的配置获取
  */
#ifndef _CONFIG_CENTER_H_
#define _CONFIG_CENTER_H_

#include "exception.h"
#include "common.h"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include "config_client.h"


class ConfigCenter
{
public:
	ConfigCenter();  
	void init(string svr_name)throw(CException);  
    string getString(const string &name);
	vector<string> getVector(const string &name, const char* splitter = "|");

private:
	string m_svr_name;
	bool is_svr_ok;
};


#endif

