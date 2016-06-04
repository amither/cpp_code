
#include "config_center.h"
#include "error.h"
#include "appcomm.h"
#include <sys/stat.h>   
#include "fund_common.h"
#include "fund_deal_service.h"

#include "config_client.h"
#include "config_center.h"



ConfigCenter::ConfigCenter()
{
	is_svr_ok = false;
}

void ConfigCenter::init(string svr_name) throw(CException)
{
    m_svr_name = svr_name;

	try
	{
		if (CONFIG4APP->Initialize() != 0)
	    {
	        //alert(ERR_INIT_CFG_CENTER, "初始化配置中心失败");
	        //throw CException(ERR_INIT_CFG_CENTER, string("初始化配置中心失败"), __FILE__, __LINE__);
			TRACE_ERROR("[初始化配置中心][失败]");
			is_svr_ok = false;
	    }
		else
		{
			TRACE_DEBUG("[初始化配置中心][成功]");
			is_svr_ok = true;
		}
	}
	catch(...)
	{
		TRACE_ERROR("[初始化配置中心][失败]");
		is_svr_ok = false;
	}  
    
}

string ConfigCenter::getString(const string &name)
{
	if (!is_svr_ok)
	{
		return "";
	}
	
    string value;
    string strMemo;
    
    int ret = CONFIG4APP->getConfigItem("common", name, m_svr_name, value,strMemo );
    
    TRACE_DEBUG("[配置中心读取结果] ret=%d,name=%s,value=%s",ret,name.c_str(),value.c_str());
    
    if (ret < 0)
    {
        //alert(ERR_READ_CFG_CENTER, string("配置中心读取失败")+name);

        TRACE_ERROR("[配置中心读取失败.] ret=%d,name=%s,value=%s",ret,name.c_str(),value.c_str());

		value = "";
    }
    
    return value;
}



vector<string> ConfigCenter::getVector(const string &name, const char* splitter)
{
	string value = getString(name);
	
	if ("" == value)
	{
		TRACE_DEBUG("[配置中心读取结果][%s is empty]",name.c_str());
		return vector<string>();
	}
	return split(value, splitter);
}



