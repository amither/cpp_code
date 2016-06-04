#include "global_access_limit.h"


extern CftLog* gPtrAppLog;
extern CkvSvrOperator *gCkvSvrOperator ; 



GlobalAccessLimit::GlobalAccessLimit(int expire, int64_t limit, string lockKey,string limitKey)
	: 
    m_expire(expire), 
    m_limit(limit), 
    m_globalAccessLockKey(lockKey),
    m_globalAccessLimitKey(limitKey)
    
{
}

/**
 * 析构函数
 */
GlobalAccessLimit::~GlobalAccessLimit()
{
}

void GlobalAccessLimit::init()
{
	gPtrAppLog->debug("init expire=[%d] limit=[%lld] lockKey=[%s] limitKey=[%s]", m_expire, m_limit,m_globalAccessLockKey.c_str(), m_globalAccessLimitKey.c_str());
	//获取全局锁，key不存在则进行初始化
	string strValue;
	int cas = 0;
	int ret = gCkvSvrOperator->get_v2(m_globalAccessLockKey,strValue,cas);

	//m_globalAccessLockKey不存在则写入带指定超时时间的value
	//13200:不存在 
	//13106:key 过期
	if( -13200 == ret || -13106 == ret)
	{
		strValue = string("local_ip=") + getLocalHostIp() + string("&time=") + getSysTime(); 
		ret =gCkvSvrOperator->set_v2(CKV_KEY_UNKNOWN_KEYNO, m_globalAccessLockKey,strValue, cas, m_expire); //key 超过指定时间自动过期
		if(ret)
		{
			throw CException(ERR_INIT_GLOBAL_ACCESS_LIMIT, "init error!", __FILE__, __LINE__);
		}
		ret = gCkvSvrOperator->incr_create(m_globalAccessLimitKey);
		//已存在会返回-13104，不知道为啥
		if(ret != 0 && ret != -13104)
		{
			throw CException(ERR_INIT_GLOBAL_ACCESS_LIMIT, "init error!", __FILE__, __LINE__);
		}
	}

}


bool GlobalAccessLimit::isOverLimit(int64_t value) 
{
	int ret =  gCkvSvrOperator->incr_value_v2(m_globalAccessLimitKey,value);
	if(ret)
	{
		return false;
	}

	if(value > m_limit)
	{
		if(checkReset())
		{		
			return true; //重置成功即认为本次请求有效，因为重置的初始值为1
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool GlobalAccessLimit::checkReset()
{
	//获取全局锁，key不存在则进行初始化
	string strValue;
	int cas = 0;
	int ret = gCkvSvrOperator->get_v2(m_globalAccessLockKey,strValue,cas);

	//m_globalAccessLockKey不存在，则标识累加器值已过期，需要重置, 使用锁过期是利用ckv的时间，而非本地服务器的时间
	//13200:不存在 
	//13106:key 过期
	if( -13200 == ret || -13106 == ret)
	{
		strValue = string("local_ip=") + getLocalHostIp() + string("&time=") + getSysTime(); 
		ret =gCkvSvrOperator->set_v2(CKV_KEY_UNKNOWN_KEYNO, m_globalAccessLockKey,strValue, cas, m_expire); //key 超过指定时间自动过期
		if(ret)
		{
			return false;
		}

		//更新锁成功后更新累加器
		ret = gCkvSvrOperator->incr_init(m_globalAccessLimitKey, 1);
		if(ret)
		{
			return false;
		}
	}

	return false;
}



