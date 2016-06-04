#ifndef _APP_COMM_H_
#define _APP_COMM_H_

#include "parameter.h"
#include "common.h"
#include "cftlog.h"
#include "sqlapi.h"
#include "trpcwrapper.h"
#include "globalconfig.h"
#include "relay_client.h"
#include "mid_sessionapi.h"
#include <set>


extern CftLog* gPtrAppLog;
extern CftLog* gPtrSysLog;

extern GlobalConfig* gPtrConfig;
extern tenpaymq::CRelayClient* gPtrRelayClient;



/*#define TRACE_DEBUG   gPtrAppLog->debug
#define TRACE_WARN    gPtrAppLog->warning
#define TRACE_NORMAL  gPtrAppLog->normal
#define TRACE_ERROR   gPtrAppLog->error*/


#define TRACE_DEBUG(fmt, args...)   gPtrAppLog->debug(fmt" %s:%d", ##args, __FILE__, __LINE__);
#define TRACE_WARN(fmt, args...)    gPtrAppLog->warning(fmt" %s:%d", ##args, __FILE__, __LINE__);
#define TRACE_NORMAL(fmt, args...)  gPtrAppLog->normal(fmt" %s:%d", ##args, __FILE__, __LINE__);
#define TRACE_ERROR(fmt, args...)   gPtrAppLog->error(fmt" %s:%d", ##args, __FILE__, __LINE__);
#define CHECK gPtrCheckLog->normal

/**
 * 对mysql结果集句柄做一个包装，包装后通过析构函数自动释放内存
 */
class scope_mysql_res {
public:
    scope_mysql_res(MYSQL_RES* pRes)
        :pMysqlRes(pRes)
    {}

    MYSQL_RES* handle()
    {
        return pMysqlRes;
    }
    
    ~scope_mysql_res()
    {
        if (NULL != pMysqlRes) {
            mysql_free_result(pMysqlRes);
            pMysqlRes = NULL;
        }
    }
private:
    MYSQL_RES* pMysqlRes;
};


string getDefSpid();

std::string common_tcp_send(const string &ip, int port, int timeout, const string &req_str);

void sendMsg2Mq(char* strRequest);

//操作鉴权
void checkSession(const string &qlskey,const string &qluin,const string &request_type) throw(CException);
void sendCouponMsg2Mq(const string& strRequest);

/**
 * TRPC 
 */
void alert(int e, const string & message);
#endif

