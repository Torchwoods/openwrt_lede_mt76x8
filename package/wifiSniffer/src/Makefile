###################项目路径和程序名称#################################
PROJ_DIR=.
LIB_DIR=$(PROJ_DIR)/lib
SRC_DIR=$(PROJ_DIR)/src
OBJ_DIR=$(PROJ_DIR)/obj
PROGRAM=$(PROJ_DIR)/wifiSniffer
###################OBJ文件及路径############################################

VPATH = $(PROJ_DIR)/:$(SRC_DIR)/libuwifi:$(SRC_DIR)/main:$(SRC_DIR)/radiotap:$(SRC_DIR)/utils:$(SRC_DIR)/cJSON

DIRS=$(shell find $(SRC_DIR) -maxdepth 1 -type d)
SRCS=$(notdir $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c)))
OBJECTS=$(SRCS:%.c=${OBJ_DIR}/%.o) 

###################lib文件及路径######################################
LIBS=
	
###################include头文件路径##################################
INCLUDE=\
		-I$(SRC_DIR)/libuwifi			\
		-I$(SRC_DIR)/main				\
		-I$(SRC_DIR)/radiotap			\
		-I$(SRC_DIR)/utils	\
		-I$(SRC_DIR)/cJSON
		       
###################编译选项及编译器###################################
ifeq ($(ARCH),mips)
		CC=mipsel-openwrt-linux-gcc
else
		CC=gcc
endif

CC=mipsel-openwrt-linux-gcc
STRIP=mipsel-openwrt-linux-strip
CFLAGS	+= -c -g -D_GNU_SOURCE -w -Wno-format 

LDFLAGS= -lpcap -lcurl -lm -luci -lubox

###################编译目标###########################################
.PHONY: all clean create_output
 
all:create_output ${PROGRAM}

create_output:	
	@echo "VPATH is $(VPATH)"
	@echo "********************************************************" 
	@echo "Check existance of output folder" 	
	@-if test ! -d $(OBJ_DIR); then mkdir $(OBJ_DIR); fi
 
${PROGRAM}:${OBJECTS}
	@echo "Building $@..."	
	@$(CC) -rdynamic -g  -Wall   $(LIBS) -o $@ $(OBJECTS) $(LDFLAGS)
	@$(STRIP) $@
	@echo "Compile Successful" 

${OBJ_DIR}/%.o:%.c
	@echo "Compiling $< into $@"
	@$(CC) $(CFLAGS) $(INCLUDE) $< -o $@  

clean:
	@rm -rf $(OBJECTS)  $(PROGRAM)

