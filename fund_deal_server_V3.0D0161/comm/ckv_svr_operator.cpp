#include "ckv_svr_operator.h"
#include "appcomm.h"

extern GlobalConfig* gPtrConfig;
extern CftLog* gPtrAppLog;
extern CftLog* gPtrCkvErrorLog; // 更新ckv出错日志
extern CftLog*  gPtrCkvSlaveErrorLog; // 更新备份ckv出错日志


CkvSvrOperator::CkvSvrOperator()
{
    m_bSlaveActive = false;
    m_bisTrans = false;
    m_bMasterDelay = false;
}

/**
 * 析构函数
 */
CkvSvrOperator::~CkvSvrOperator()
{
}

int CkvSvrOperator::initCKV(trmem_client_api &stApid, CkvCfg&ckvCfgInfo,bool isMaster)
{
	int ret = 0;
	if (ckvCfgInfo.m_use_l5)
	{
		//支持L5方式访问
		//暂不支持
		/*
		int ret = m_st_api.config_l5_sid(gPtrConfig->m_CkvCfg.l5_modid, gPtrConfig->m_CkvCfg.l5_cmdid);
		if (ret)
		{
			gPtrAppLog->error("config_l5_sid error: ret=%d, errmsg=%s\n", ret, m_st_api.GetLastErr());
			return ret;
		}
		*/
	}
	else 
	{
		vector<TServerAddr> vecServerAddr;
		TServerAddr addr;
		for(vector<string>::iterator iter = ckvCfgInfo.host_vec.begin(); iter != ckvCfgInfo.host_vec.end(); ++iter)
		{
			gPtrAppLog->debug("ckv ip[%s] port[%d]", (*iter).c_str(), ckvCfgInfo.port);
			strncpy(addr.strIP, (*iter).c_str(), sizeof(addr.strIP) - 1);
			addr.usPort = (unsigned short)ckvCfgInfo.port;
			vecServerAddr.push_back(addr);
		}
		
		ret = stApid.config_server_addr(vecServerAddr);
             if (ret)
             {
                  gPtrAppLog->error("m_st_api config_server_addr error ret=%d", ret);
             }
	}

    m_bid = ckvCfgInfo.bid;
	stApid.config_connect_timeout(ckvCfgInfo.timeout);
	ret = stApid.set_passwd(ckvCfgInfo.bid, const_cast<char*>(ckvCfgInfo.passwd.c_str()));
	if (ret)
	{	
		gPtrAppLog->error("m_st_api set_passwd error ret=%d", ret);
             alert(ERR_INIT_CKV, isMaster?"init master ckv fail,process exit!!!":"init slave ckv fail!!");
             if (isMaster)
             {
                 throw CException(ERR_INIT_CKV, "init master ckv fail,process exit! ", __FILE__, __LINE__);
             }
		return ERR_INIT_CKV;
	}

	return 0;
}

void CkvSvrOperator::beginCkvtrans(bool masterDelay)
{
    //调用rollback清理本地缓存
    gPtrAppLog->debug("beginCkvtrans");
    rollBackCkvtrans();
    m_bisTrans = true;
    m_bMasterDelay = masterDelay;
}

void CkvSvrOperator::commitCkvtrans()
{
    gPtrAppLog->debug("commitCkvtrans,set value to ckv");
    m_bisTrans = false;
    
    map<string,TCKVLocalCacheNode>::iterator KVpos=m_key2KNodeInfo.begin();
    for (;KVpos != m_key2KNodeInfo.end();KVpos++)
    {
        int keyno = KVpos->second.keyno;
        string key=KVpos->first;
        string value=KVpos->second.value;
        if (KVpos->second.isv2)
        {
            int cas=-1;
            //v2版本只有备份ckv是延迟写入
            setSlaveCkv_v2(keyno,key,value,cas,KVpos->second.expire,KVpos->second.offset,KVpos->second.len);
        }
        else
        {
            //如果主ckv放到事物之后写入则需要写主ckv
            if (m_bMasterDelay == true)
            {
                setMasterCkv(keyno,key,value);
            }
            //如果有备份ckv，那么写备份ckv
            if (m_bSlaveActive && KVpos->second.writeSlaveCkv)
            {
                setSlaveCkv(keyno, key,  value);
            }
        }
    }
    m_bMasterDelay = false;
    //清理数据
    rollBackCkvtrans();
}

void CkvSvrOperator::rollBackCkvtrans()
{
    m_bisTrans = false;
    m_key2KNodeInfo.clear();
}

int CkvSvrOperator::init_sub_acc_ckv()
{
	if (gPtrConfig->m_SubAccCkvCfg.m_use_l5)
	{
	}
	else 
	{
		vector<TServerAddr> vecServerAddr;
		TServerAddr addr;
		for(vector<string>::iterator iter = gPtrConfig->m_SubAccCkvCfg.host_vec.begin(); iter != gPtrConfig->m_SubAccCkvCfg.host_vec.end(); ++iter)
		{
			gPtrAppLog->debug("sub acc ckv ip[%s] port[%d]", (*iter).c_str(), gPtrConfig->m_SubAccCkvCfg.port);
			strncpy(addr.strIP, (*iter).c_str(), sizeof(addr.strIP) - 1);
			addr.usPort = (unsigned short)gPtrConfig->m_SubAccCkvCfg.port;
			vecServerAddr.push_back(addr);
		}
		
		m_st_api.config_server_addr(vecServerAddr);
	}

	
	
	m_bid = gPtrConfig->m_SubAccCkvCfg.bid;

	m_st_api.config_connect_timeout(gPtrConfig->m_SubAccCkvCfg.timeout);
	int ret = m_st_api.set_passwd(gPtrConfig->m_SubAccCkvCfg.bid, const_cast<char*>(gPtrConfig->m_SubAccCkvCfg.passwd.c_str()));
	if (ret)
	{	
		gPtrAppLog->error("m_st_api set_passwd error ret=%d", ret);
		throw CException(ERR_INIT_CKV, "init ckv error", __FILE__, __LINE__);
	}

	return 0;
}

int CkvSvrOperator::init()
{
    //初始化主ckv
    gPtrAppLog->debug("start init master ckv");
    initCKV(m_st_api,gPtrConfig->m_CkvCfg,true);
    

    //如果配置了备份ckv，需要初始话备ckv连接
    if (gPtrConfig->m_CkvCfg_slave.host_vec.size()>0)
    {
        
        gPtrAppLog->debug("start init slave ckv");
        if (0 == initCKV(m_st_api_slave,gPtrConfig->m_CkvCfg_slave,false))
        {
            m_bSlaveActive = true;
        }
    }
    else
    {
        m_bSlaveActive = false;
        gPtrAppLog->debug("no slave ckv configed");
    }

    return 0;
}

/**
 * get操作。
 */
int CkvSvrOperator::get(string& key, CParams& paramOrder) 
{
	string data;
	int ret =  get(key, data);
    if (0 == ret) {
        paramOrder.parse(data);
    } else {
        return ret;
    }

	return 0;
}

/**
 * get操作。
 */
int CkvSvrOperator::get(string& key, string& value) 
{
       if (m_bisTrans && m_key2KNodeInfo.find(key) != m_key2KNodeInfo.end())
       {
            value = m_key2KNodeInfo[key].value;
            gPtrAppLog->debug("get ckv local cache key[%s], result[%s]", key.c_str(), value.c_str());
            return 0;
       } 

	int ret =  m_st_api.get(gPtrConfig->m_CkvCfg.bid, key, value);

	//重试一次
	/*if(ret)
	{
		ret =  m_st_api.get(gPtrConfig->m_CkvCfg.bid, key, value);
	}*/

	if (ret)
	{
		/**slackware下取ip代码不兼容，有问题 
		string str_last_access;
		unsigned short u_last_port;
		
		m_st_api.GetLastServer(&str_last_access, &u_last_port);
		*/
		if(-13200 != ret)
		{
			//打印错误日志，13200:数据不存在，很多查询都会数据不存在，避免日志过多，不打印 
			gPtrAppLog->error("get error! key=%s, bid=%d, ret=%d, errmsg=%s\n", key.c_str(),
				gPtrConfig->m_CkvCfg.bid, ret,  m_st_api.GetLastErr());
		}
		else
		{
			gPtrAppLog->debug("get error! key=%s, bid=%d, ret=%d, errmsg=%s\n", key.c_str(),
				gPtrConfig->m_CkvCfg.bid, ret,  m_st_api.GetLastErr());
		}

		return ret;
	}
    
	gPtrAppLog->debug("get ckv key[%s], result[%s]", key.c_str(), value.c_str());

	return 0;
}

/**
 * 写主ckv操作
 */
int CkvSvrOperator::setMasterCkv(int key_no, string& key, string& value) 
{
	gPtrAppLog->debug("set setMasterCkv. key=[%s] value=[%s]", key.c_str(), value.c_str());
	int ret =  m_st_api.set(gPtrConfig->m_CkvCfg.bid, key, value);

	//重试一次
	/*if(ret)
	{
	       gPtrAppLog->debug("set setMasterCkv. key=[%s] fail,ret=%d,retry", key.c_str(), ret);
		ret =  m_st_api.set(gPtrConfig->m_CkvCfg.bid, key, value);
	}*/

	if (ret)
	{
	      gPtrAppLog->debug("set setMasterCkv. key=[%s] fail ,ret=%d", key.c_str(), ret);
             //规范ckv写失败的打印日志
		//日志打印内容"版本号|bid|ret|keyno|key|value|errmsg"
		gPtrCkvErrorLog->error("%s|%d|%d|%d|%s|%s|%s", 
		        "1.0", gPtrConfig->m_CkvCfg.bid, 
		        ret, key_no,
		        key.c_str(), 
		        value.c_str() , m_st_api.GetLastErr());

	}

	return ret;
}


/**
 * 写备份ckv操作，异步写入
 */
int CkvSvrOperator::setSlaveCkv(int key_no, string& key, string& value) 
{
    gPtrAppLog->debug("set_unacknowledged ckv. key=[%s] to slave ckv", key.c_str());
    int sret = m_st_api_slave.set_unacknowledged(gPtrConfig->m_CkvCfg_slave.bid, key, value);
    gPtrAppLog->debug("set_unacknowledged ckv. key=[%s] to slave ckv ,ret=%d", key.c_str(),sret);
    if (sret)
    {
        
        //失败打印日志
        gPtrCkvSlaveErrorLog->error("%s|%d|%d|%d|%s|%s|%s", 
		        "1.0", gPtrConfig->m_CkvCfg_slave.bid, 
		        sret, key_no,
		        key.c_str(), 
		        value.c_str() , m_st_api.GetLastErr());
    }

    return sret;
}


/**
 * 写操作
 */
int CkvSvrOperator::set(int key_no, string& key, string& value,bool writeSlaveCkv) 
{
	int ret = 0;
	if(key.empty())
	{
		gPtrAppLog->error("set ckv error,key empty.");
		return -1;
	}

       if (m_bisTrans)
       {
            //如果主ckv不延迟写，则直接写入
            if (m_bMasterDelay == false)
            {
                ret = setMasterCkv(key_no, key,  value);
            }
            
            //缓存key value到本地cache
            m_key2KNodeInfo[key].keyno=key_no;
            m_key2KNodeInfo[key].value=value;
            m_key2KNodeInfo[key].isv2=false;
            m_key2KNodeInfo[key].writeSlaveCkv=writeSlaveCkv;
            gPtrAppLog->debug("set ckv. key=[%s] to local cache", key.c_str());
       }
       else
       {
           //没有启动事物，则直接写主备ckv
           ret = setMasterCkv(key_no, key,  value);
           if (m_bSlaveActive && writeSlaveCkv)
           {
                setSlaveCkv(key_no, key,  value);
           }
       }

	return ret;
}

/**
 * 写操作
 */
int CkvSvrOperator::del(int key_no, string& key) 
{
    int ret =  m_st_api.del(gPtrConfig->m_CkvCfg.bid, key);
    if (ret && ERR_CKV_DEL_KEY_NO_EXIST != ret)
    {
        gPtrAppLog->error("del key error! key=%s, bid=%d, ret=%d, errmsg=%s\n", key.c_str(),
			gPtrConfig->m_CkvCfg.bid, ret,  m_st_api.GetLastErr());

        /* 如果key已删除，再删除时对key不存在错误认为正常 */
      
        //规范ckv写失败的打印日志,del操作日与set操作日志相同,
        //所有del失败通过set来补单,不能直接用del操作补单(del操作补单会存在将正确的key误删除)
        //日志打印内容"版本号|bid|ret|keyno|key|value|errmsg"
        gPtrCkvErrorLog->error("%s|%d|%d|%d|%s|%s|%s", 
                "1.0",
                gPtrConfig->m_CkvCfg.bid, 
		        ret, key_no,
		        key.c_str(), "del_ckv_key",
		        m_st_api.GetLastErr());
    }
    else
    {
        ret = 0;
    }
    gPtrAppLog->debug("del ckv key[%s],ret=%d", key.c_str(),ret);

    //如果有配置备份ckv，需要删除备份ckv数据。备份cvk写失败不重试，只打印日志
    if (m_bSlaveActive)
    {
        gPtrAppLog->debug("del ckv. key=[%s] from slave ckv", key.c_str());
        int sret = m_st_api_slave.del(gPtrConfig->m_CkvCfg_slave.bid, key);
        if (sret && ERR_CKV_DEL_KEY_NO_EXIST != sret)
        {
             //失败打印日志
            gPtrCkvSlaveErrorLog->error("%s|%d|%d|%d|%s|%s|%s", 
                "1.0",
                gPtrConfig->m_CkvCfg_slave.bid, 
		        sret, key_no,
		        key.c_str(), "del_ckv_key",
		        m_st_api.GetLastErr());
        }
    }
   
    return ret;
}

int CkvSvrOperator::incr_create(string &key, uint64_t value)
{
	gPtrAppLog->debug("incr_create ckv. key=[%s], value=[%lld]", key.c_str(), value);
    int ret =  m_st_api.incr_create(gPtrConfig->m_CkvCfg.bid, key, value);
	if (ret)
	{
		gPtrAppLog->error("incr_create error! key=%s, bid=%d, ret=%d, errmsg=%s\n", key.c_str(),
			gPtrConfig->m_CkvCfg.bid, ret,  m_st_api.GetLastErr());

		return ret;
	}
	return 0;
}


int CkvSvrOperator::incr_init(string &key, uint64_t value)
{
	gPtrAppLog->debug("incr_init ckv. key=[%s], value=[%lld]", key.c_str(), value);
    int ret =  m_st_api.incr_init(gPtrConfig->m_CkvCfg.bid, key, value);
	if (ret)
	{
		gPtrAppLog->error("incr_init error! key=%s, bid=%d, ret=%d, errmsg=%s\n", key.c_str(),
			gPtrConfig->m_CkvCfg.bid, ret,  m_st_api.GetLastErr());

		return ret;
	}
	return 0;
}

int CkvSvrOperator::incr_value_v2(string &key, int64_t& value)
{
	int ret =  m_st_api.incr_value_v2(gPtrConfig->m_CkvCfg.bid, key, value);
	if (ret)
	{
		gPtrAppLog->error("incr_value_v2 error! key=%s, bid=%d, ret=%d, errmsg=%s\n", key.c_str(),
			gPtrConfig->m_CkvCfg.bid, ret,  m_st_api.GetLastErr());

		return ret;
	}
	gPtrAppLog->debug("ckv incr_value_v2 result value[%lld]", value);
	return 0;
}

int CkvSvrOperator::get_v2(string &key, string &data, int &cas, int offset, int len, TRspExt* rsp_ext)
{	
	int ret =  m_st_api.get_v2(gPtrConfig->m_CkvCfg.bid, key, data, cas, offset,len, rsp_ext);
	
	//重试一次
	/*if(ret)
	{
		ret =  m_st_api.get_v2(gPtrConfig->m_CkvCfg.bid, key, data, cas, offset,len, rsp_ext);
	}*/
	
	if (ret)
	{
		gPtrAppLog->error("get_v2 error! key=%s, bid=%d, ret=%d, errmsg=%s\n", key.c_str(),
			gPtrConfig->m_CkvCfg.bid, ret,  m_st_api.GetLastErr());

		return ret;
	}
	gPtrAppLog->debug("get_v2 ckv key[%s], result[%s], cas[%d]", key.c_str(), data.c_str(), cas);
	return 0;
}

int CkvSvrOperator::set_v2(int key_no,string &key, string &data, int &cas, int expire, int offset, int len)
{
	if(key.empty())
	{
		gPtrAppLog->error("set ckv error,key empty.");
		return -1;
	}
	
	gPtrAppLog->debug("set_v2 ckv. key=[%s] value=[%s] cas=[%d]", key.c_str(), data.c_str(),cas);
	int ret =  m_st_api.set_v2(gPtrConfig->m_CkvCfg.bid, key, data, cas, expire,offset,len);

	//重试一次
	/*if(ret)
	{
		ret =  m_st_api.set_v2(gPtrConfig->m_CkvCfg.bid, key, data, cas, expire,offset,len);
	}*/
	
	if (ret)
	{
		//规范ckv写失败的打印日志
		//日志打印内容"版本号|bid|ret|keyno|key|value|errmsg"
		gPtrCkvErrorLog->error("%s|%d|%d|%d|%s|%s|%s", 
                "1.0",
                gPtrConfig->m_CkvCfg.bid, 
		        ret, key_no,
		        key.c_str(), data.c_str(),
		        m_st_api.GetLastErr());

	}

       //如果有配置备份ckv，需要写入。备份cvk写失败不重试，只打印日志
       if (m_bSlaveActive && ret==0)
       {
            setSlaveCkv_v2(key_no, key, data, -1, expire, offset, len);
       }
    
	return ret;
}


/**
 * 写备份ckv操作
 */
int CkvSvrOperator::setSlaveCkv_v2(int key_no,string &key, string &data, int cas, int expire, int offset, int len) 
{
    int sret = 0;
    if (m_bisTrans)
    {
         m_key2KNodeInfo[key].keyno=key_no;
         m_key2KNodeInfo[key].value=data;
         m_key2KNodeInfo[key].isv2=true;
         m_key2KNodeInfo[key].expire=expire;
         m_key2KNodeInfo[key].offset=offset;
         m_key2KNodeInfo[key].len=len;
    }
    else
    {
        int scas=-1;
        if (expire>0 || offset>0 || len != -1)
        {
            gPtrAppLog->debug("set_v2 ckv. key=[%s] to slave ckv", key.c_str());
            sret = m_st_api_slave.set_v2(gPtrConfig->m_CkvCfg_slave.bid, key, data,scas,expire,offset,len);
            gPtrAppLog->debug("set_v2 ckv. key=[%s] to slave ckv,ret=%d", key.c_str(),sret);
        }
        else
        {
            gPtrAppLog->debug("set_unacknowledged ckv. key=[%s] to slave ckv", key.c_str());
            sret = m_st_api_slave.set_unacknowledged(gPtrConfig->m_CkvCfg_slave.bid, key, data);
            gPtrAppLog->debug("set_unacknowledged ckv. key=[%s] to slave ckv,ret=%d", key.c_str(),sret);
        }
        if (sret)
        {
              //失败打印日志
             gPtrCkvSlaveErrorLog->error("%s|%d|%d|%d|%s|%s|%s", 
                 "1.0",
                 gPtrConfig->m_CkvCfg_slave.bid, 
    	        sret, key_no,
            key.c_str(), data.c_str(),
            m_st_api.GetLastErr());
        }
    }

    return sret;
}


int CkvSvrOperator::set_v2_with_get_v2(int key_no, string &key, string &data, int expire, int offset, int len)
{
    string szValue;

	int cas = 0;
	int ret = get_v2(key,szValue,cas);

	if( ERR_KEY_NOT_EXIST == ret || ERR_KEY_EXPIRE == ret)
	{
		cas = 0; //key不存在，返回的cas为-1，cas=-1可以任意修改ckv,此处要设置初始cas=0
	}else if(0 != ret)
	{
		return ret;
	}

	ret = set_v2(key_no, key,data, cas, expire); //key 超过指定时间自动过期
	if(ret)
	{
		return ret;
	}
	
	return 0;
}

int CkvSvrOperator::get_by_col(string key,unsigned int col,string &value,int &cas)
{
     int rslt =  m_st_api.get_col(m_bid, key,  col,  value,  cas);
     if(rslt)
    {
        string str_last_access;
        unsigned short u_last_port;

        m_st_api.GetLastServer(&str_last_access, &u_last_port);
        
        if(-13200 != rslt)
        {
            gPtrAppLog->error("get error! key=%s, bid=%d, access ip:port=%s:%d, rslt=%d, errmsg=%s\n",
                            key.c_str(),
                            m_bid,
                            str_last_access.c_str(),
                            u_last_port,
                            rslt,
                            m_st_api.GetLastErr());

        }
        else
        {
            gPtrAppLog->debug("get error! key=%s, bid=%d, access ip:port=%s:%d, rslt=%d, errmsg=%s\n",
                            key.c_str(),
                            m_bid,
                            str_last_access.c_str(),
                            u_last_port,
                            rslt,
                            m_st_api.GetLastErr());
        }

        return rslt;
    }
    gPtrAppLog->debug("get by col ckv,col[%u] key[%s], value[%s]",col, key.c_str(), value.c_str());
    return 0;
}

int CkvSvrOperator::get_by_multi_col(TKeyGetListNode & node)
{
	if(m_bid==0)
        return -1;
    
    int result =  m_st_api.get_mul_col(m_bid, node);
    
	string datas;
    bool bfirst_node = true;
	for(vector<TGetNode>::iterator it=node.v_col_node.begin();it!=node.v_col_node.end();it++)
	{   
        if (!bfirst_node) {
            datas += ",";
        }
        
		datas += "<col=" + toString(it->col);
		datas += ",data(";
		datas += it->data;
		datas += "),retcode=";
		datas += toString(it->retcode);
		datas += ">";
        
        if (bfirst_node) {
            bfirst_node = false;
        }
	}

    if (0 != result) {
        string str_last_access;
        unsigned short u_last_port;

        m_st_api.GetLastServer(&str_last_access, &u_last_port);

        if (result < 0 && CKV_E_NO_DATA != result) {
            gPtrAppLog->error("get_mul_col error!key=%s,size=%zd,value=[%s],ip=%s,port=%u,result=%d,errmsg=%s",
                node.key.c_str(),node.v_col_node.size(), datas.c_str(), 
                str_last_access.c_str(), u_last_port,
                result, m_st_api.GetLastErr());
        } else {

            gPtrAppLog->debug("get_mul_col error!key=%s,size=%zd,value=[%s],ip=%s,port=%u,result=%d,errmsg=%s",
                node.key.c_str(), node.v_col_node.size(),datas.c_str(), 
                str_last_access.c_str(), u_last_port,
                result, m_st_api.GetLastErr());
        }

        return result;
    }

    gPtrAppLog->debug("get_mul_col,key=%s,size=%zd,value=[%s],result=%d",
                node.key.c_str(),node.v_col_node.size(), datas.c_str(), result);
    
    return result;
}




