###################################################################
# Vertion: 1.0
# Author:  edited by Hawkliu 2007-07-25 
# History: 
##################################################################
PATH_INC = ../include
PATH_SRC = ../service
PATH_BIN = ../bin
PATH_OBJ = ../obj
PATH_COM = ../comm
PATH_OPENSSL = /usr/local/ssl/include

# Setting for trpc lib
TRPC_HOME    = /usr/local/middle
LIB_TRPC     = $(TRPC_HOME)/lib/libtrpc.a
LIB_TRPC_SYS = $(TRPC_HOME)/lib/libtrpc_sys.a
LIB_TRPC_SRV = $(TRPC_HOME)/lib/libtrpc_server.a
LIB_TRPC_CLI = $(TRPC_HOME)/lib/libtrpc_client.a

# Setting for business lib
PATH_CFT         = /usr/local/cftlib
PATH_XML         = $(PATH_CFT)/tinyxml
LIB_XML          = $(PATH_CFT)/tinyxml/libtinyxml.a
PATH_XYZ         = $(PATH_CFT)/libxyz
LIB_XYZ	         = $(PATH_CFT)/libxyz/libxyz.a

#relay
PATH_MQ			 = $(PATH_CFT)/relay_mq_api
LIB_MQ			 = $(PATH_MQ)/librelaymq.a

PATH_QPAY_ENCRYPT= $(PATH_CFT)/qpayencryptclient
LIB_QPAY_ENCRYPT = $(PATH_QPAY_ENCRYPT)/libqpayencryptclient.a

PATH_QPAY_TOOL   = $(PATH_CFT)/qpaytools
LIB_QPAY_TOOL    = $(PATH_QPAY_TOOL)/libqpaytools.a

PATH_BILLNO      = $(PATH_CFT)/billnoapi
LIB_BILLNO       = $(PATH_BILLNO)/libbillnoapi.a


# Mysql library
MYSQL_HOME       = /usr/local/cftlib/mysql
MYSQL_LIB        =-L$(MYSQL_HOME)/lib/mysql -lmysqlclient

# common static lib
LIB_SSL          = -L/usr/local/cftlib/openssl/lib -lssl
LIB_CRYPTO       = -L/usr/local/cftlib/openssl/lib -lcrypto
LIB_COMM         = $(PATH_COM)/libcommon.a

# TransApi library
TRANS_HOME       = $(PATH_CFT)/transapi
TRANS_LIB        = $(TRANS_HOME)/libtrans_api.a

#ttcapi
TTC_HOME 		 = $(PATH_CFT)/ttcapi
TTC_LIB			 = -L$(PATH_CFT)/ttcapi -lttc

# passconf library
PASSCONF_HOME    = $(PATH_CFT)/passconf_api
LIB_PASSCONF     = $(PASSCONF_HOME)/libpassconf_api.a
PATH_CFT_CFG_API = $(PATH_CFT)/cft_cnf_api/include
LIB_CFT_CFG_API = -L$(PATH_CFT)/cft_cnf_api/ -lconfcli -lcft_msgs -lconfig_base -lminixml -lprotobuf -lcft -lpthread

#ckv
L5_HOME = $(PATH_CFT)/ckv_lib/dependence/L5_API
CKV_HOME = $(PATH_CFT)/ckv_lib
CKV_LIB =-L$(PATH_CFT)/ckv_lib/lib -L$(PATH_CFT)/ckv_lib/dependence/L5_API/lib -ltrmem \
		 -L$(PATH_CFT)/ckv_lib -lasn1c++ -lqos_client

#sessionapi
SESSIONAPI_HOME=$(PATH_CFT)/sessionapi
SESSIONAPI_INCLUDE=$(PATH_CFT)/sessionapi/include
LIB_SESSIONAPI=$(SESSIONAPI_HOME)/lib/libsessioncgiapi.a $(SESSIONAPI_HOME)/lib/libmemcached.a $(SESSIONAPI_HOME)/lib/libminixml.a

#LCT LIB
LIBLCT_COMM_INCLUDE  = /usr/local/cftlib/lctlib/lctcommon/include
LIB_LCT_COMM         = -L/usr/local/cftlib/lctlib/lctcommon/lib -llctcommon

# Setting for compile
CC       := g++
CFLAGS   += -I$(PATH_XYZ) -I$(PATH_QPAY_TOOL) -I$(PATH_COM) -I$(PATH_INC) -I$(PATH_QPAY_ENCRYPT) \
            -I$(PATH_BILLNO) -I$(TRPC_HOME)/include -I$(PATH_XML) -I$(MYSQL_HOME)/include/mysql \
            -I$(PATH_OPENSSL)  -I$(TRANS_HOME)  -I$(TTC_HOME)  -I$(PASSCONF_HOME) \
            -I$(CKV_HOME) -I$(L5_HOME) -I$(SESSIONAPI_INCLUDE)  -I$(PATH_MQ) -I$(PATH_MQ) -I$(PATH_CFT_CFG_API) \
	    -I$(LIBLCT_COMM_INCLUDE)
CFLAGS   +=  $(TENPAY_TEST_FLAG) 
            
#CFLAGS   += -g -O -DNDEBUG -Wall -fpic 
CFLAGS   += -g -DNDEBUG -Wall -fpic 

SERVER_VERSION_SIGN = $(shell cat /dev/urandom | tr -dc A-Za-z0-9_ | head -c31 | sed 's/[ \r\b]//g')
CFLAGS   += -DSERVER_VERSION_SIGN="\"$(SERVER_VERSION_SIGN)\""

LIBS     += $(LIB_PASSCONF) $(LIB_COMM) $(LIB_TRPC) $(LIB_TRPC_SYS) $(LIB_TRPC_SRV) $(LIB_TRPC_CLI) \
            $(LIB_QPAY_ENCRYPT)  $(LIB_BILLNO) $(LIB_SSL) $(LIB_CRYPTO) $(LIB_XML) \
            $(LIB_QPAY_TOOL) $(TRANS_LIB) $(TRANS_COMM) $(MYSQL_LIB) $(LIB_XYZ)  $(LIB_CFT_CFG_API) \
	    $(TTC_LIB) -ldl -lpthread $(CKV_LIB) $(LIB_SESSIONAPI) $(LIB_MQ) $(LIB_LCT_COMM)
LIBS     += $(TENPAY_TEST_LIBS)

# Target to create
SO   := $(PATH_BIN)/fund_deal_service.so
OBJS := $(patsubst $(PATH_SRC)/%.cpp,$(PATH_OBJ)/%.o,$(wildcard $(PATH_SRC)/*.cpp))

$(SO) : $(LIB_COMM) $(OBJS)
	$(CC) -O -shared -lc -o $@ $(CFLAGS) $^ $(LIBS)

$(PATH_OBJ)/%.o :  $(PATH_SRC)/%.cpp 
	$(CC) $(CFLAGS) -c $< -o $@

# Create libcommon.a 
COMS := $(patsubst $(PATH_COM)/%.cpp,$(PATH_COM)/%.o,$(wildcard $(PATH_COM)/*.cpp))
$(LIB_COMM) : $(COMS)
	@ar rv $@ $^

$(PATH_COM)/%.o :  $(PATH_COM)/%.cpp 
	$(CC) $(CFLAGS) -c $< -o $@

# Tools
.PHONY : clean install
clean:
	rm -f $(COMS) $(LIB_COMM) $(PATH_COM)/*.d $(OBJS) $(SO) $(PATH_SRC)/*.d

install: 
	$(TRPC_HOME)/tools/install_so $(SO) fund_deal_server 0 28001 

dev: 
	cp ../conf/fund_deal_server.xml /usr/local/middle/fund_deal_server/conf/
	$(TRPC_HOME)/tools/install_so $(SO) fund_deal_server 0 28001

# test 的目的是生成一个可执行文件，检查是否有函数没有连接进来
test: main.cpp $(SO)
	$(CXX) -o $@ $^ $(LIBS)