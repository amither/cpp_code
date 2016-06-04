#ifndef _USER_TTC_H_
#define _USER_TTC_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ttcapi.h"

#include <vector>
#include <string>

using namespace std;
using namespace TTC;

class CUserTTC  
{
public:
	CUserTTC(char* szAddr, int iPort, int iTimeout);
	virtual ~CUserTTC();
public:
	int GetUser(int iUid, int iCurType, Result* pResult);
	int PurgeUser(int iUid, int iCurType, Result* pResult);

private:
	TTC::Server m_UserTableTTC;

};

#endif //_USER_TTC_H_

