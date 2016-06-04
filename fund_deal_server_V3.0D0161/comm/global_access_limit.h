#ifndef _GLOBAL_ACCESS_LIMIT_H_
#define _GLOBAL_ACCESS_LIMIT_H_

#include <sstream>
#include "ckv_svr_operator.h"




/**
 * 全局流量控制服务类
 * 该类提供统一的接口，所有访问流量是否达到限制
 * 支持多机多进程，及并发条件下的有效性判断
 */
class GlobalAccessLimit
{
public:
    GlobalAccessLimit(int expire, int64_t limit, string lockKey = "globalAccessLockKey",string limitKey = "globalAccessLimitKey");
    /**
     * 析构函数
     */
    virtual~GlobalAccessLimit();

	/**
	* 初始化函数，初始化全局配置
	*/
	void init();

	/**
     * 判断是否达到限制
     * input 
	 * value:本次消耗数量
     */
    bool isOverLimit(int64_t value = 1); 

private:
	bool checkReset();


private:
	int m_expire; //过期时长,单位为秒
	int64_t m_limit; //限制上限
	string m_globalAccessLockKey;
	string m_globalAccessLimitKey;
	

};

#endif

