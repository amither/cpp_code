

#ifndef  SESSIONAPI_INC_912723
#define  SESSIONAPI_INC_912723

#include <map>
#include <vector>
#include <iterator>
#include <string>
#include "sessionapi.h"
#include "exception.h"

using namespace std;
using namespace SESSION_API_WITH_AUDIT;

// 登陆类型LOGINTYPE
#define CFTLOGIN 0x1
#define QQLOGIN 0x2
#define PRELOGIN 0x4 //初级登陆
#define FUNDLOGIN 0x8 //只注册基金交易帐户用户登录
#define WXLOGIN 0x10 //微信验证登录


class CSessionApi
{
public:
    CSessionApi();
    ~CSessionApi();

    /*
     * 获取session数据
     */
    void getSessionData(const string& strKey, map<string, string> &exKeys, map<string, string> &sessionValues) throw(CException);

    /*
     * 登录态校验
     */
    void checkLogin(const string& strKey, const string& strUin, map<string, string> &exKeys) throw(CException);
private:
    void checkWxLogin(const string& strKey, const string& strUin, 
        const map<string, string> &sessionValues) throw(CException);

    void checkMQQLogin(const string& strKey, const string& strUin,
        const map<string, string> &sessionValues) throw(CException);
private: 
    /*
     * session服务链接
     */
    CSession m_Session;
    CSessionCfg m_SessionCfg;

};

#endif   /* ----- #ifndef SESSIONAPI_INC_912723  ----- */

