#include "config_center.h"
#include "error.h"
#include "appcomm.h"
#include <algorithm>
#include <sys/stat.h>   
#include "user_classify.h"

#define SWITCH_CONF_CENTER_ON 1
UserClassify::UserClassify()
{
}

void UserClassify::init(string svr_name, int switch_conf) throw(CException)
{
    m_svr_name = svr_name;
    is_loaded = false;
	is_conf_center = false;

	if (switch_conf == SWITCH_CONF_CENTER_ON)
	{
		m_conf_center.init(svr_name);
		is_conf_center = true;
	}
	else
	{
		is_conf_center = false;
		loadTypeDataFromLocal();
	}
    
	
    TRACE_DEBUG("UserClassify init OK");
}

void UserClassify::loadTypeData()
{
    m_vip_list = m_conf_center.getVector("OUTER_VIP_LIST");
    m_tencent_vip_list = m_conf_center.getVector("TENCENT_VIP_LIST");

    is_loaded = true;
}

void UserClassify::loadTypeDataFromLocal()
{
	TRACE_DEBUG("[Local config][outer_vip_list][value=%s]",gPtrConfig->m_AppCfg.outer_vip_list.c_str());
	TRACE_DEBUG("[Local config][tencent_vip_list][value=%s]",gPtrConfig->m_AppCfg.tencent_vip_list.c_str());
    m_vip_list = gPtrConfig->m_AppCfg.outer_vip_list == "" ? vector<string>() : split(gPtrConfig->m_AppCfg.outer_vip_list, "|");
    m_tencent_vip_list =gPtrConfig->m_AppCfg.tencent_vip_list == "" ? vector<string>() : split(gPtrConfig->m_AppCfg.tencent_vip_list, "|");

    is_loaded = true;
}


bool UserClassify::inUserType(vector<string> &type_data, const string &uin)
{
    vector<string>::iterator ret;
    ret = find(type_data.begin(), type_data.end(), uin);
    if(ret == type_data.end())
        return false;
    else
        return true;
}

UserClassify::UT UserClassify::getUserType(const string &uin) throw(CException)
{
    //Only conf centen need to query
	if (is_conf_center)
	{
		loadTypeData();
	}
    
	
	if (inUserType(m_tencent_vip_list, uin))
    {
    	//TRACE_DEBUG("User type is TENCENT_VIP")
        return TENCENT_VIP;
    }

    if (inUserType(m_vip_list, uin))
    {
    	//TRACE_DEBUG("User type is OUT_VIP")
        return OUT_VIP;
    }

	//TRACE_DEBUG("User type is NORMAL")
    return NORMAL;
}


