###################################################################
# Description: This makefile is edited for create share middle
#              service lib.
# Vertion: 1.0
# Author:  edited by Hawkliu 2007-03-05 
# History: 
##################################################################
PATH_INC = ../include
PATH_BIN = /usr/local/middle/bin

# Setting for trpc lib
TRPC_HOME    = /usr/local/middle
LIB_TRPC     = $(TRPC_HOME)/lib/libtrpc.a
LIB_TRPC_SYS = $(TRPC_HOME)/lib/libtrpc_sys.a
LIB_TRPC_SRV = $(TRPC_HOME)/lib/libtrpc_server.a
LIB_TRPC_CLI = $(TRPC_HOME)/lib/libtrpc_client.a

# Setting for compile
CC       := g++
CFLAGS   += -I$(PATH_INC) -I$(TRPC_HOME)/include 
CFLAGS   += -O3 -DNDEBUG -Wall 
LIBS     += $(LIB_TRPC_SRV) $(LIB_TRPC) $(LIB_TRPC_SYS)  

# Target to create
EXE   := fund_server
OBJS  := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

$(EXE) : $(OBJS)
	$(CC) -g -o $@ $(CFLAGS) $^ $(LIBS) -ldl

%.o :  %.cpp 
	$(CC) $(CFLAGS) -c $< -o $@

# Dependency Files
include $(subst .cpp,.d,$(wildcard *.cpp))
%.d : %.cpp
	@$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Tools
.PHONY : clean install
clean:
	rm -f $(OBJS) $(EXE) $(PATH_SRC)/*.d

install:
	mv $(EXE) $(PATH_BIN)/$(EXE)
