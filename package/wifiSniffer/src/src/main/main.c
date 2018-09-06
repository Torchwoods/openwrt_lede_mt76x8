#include <getopt.h>
#include "log.h"
#include "types.h"
#include "route_timer.h"
#include "route_http.h"

#define VERSION	"0.1 bate"

bool use_log_debug = false;

char *dev = NULL;

void main_usage(char* exeName)
{
    printf("Usage: ./%s ifname\n", exeName);
    printf("Eample: ./%s ifname\n", exeName);
}

static struct option const cmd_optons[]=
{
	{"help",0,NULL,'h'},
	{"version",0,NULL,'v'},
	{"Debug",0,NULL,'d'},
	{NULL,0,NULL,0}
};

/*****************************************************************************
 * 函 数 名  : main_parse_cmd
 * 负 责 人  : Edward
 * 创建日期  : 2016年5月9日
 * 函数功能  : 解析传递的命令参数
 * 输入参数  : int argc    命令个数
               char**argv  命令
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
uint8_t main_parse_cmd(int argc,char**argv)
{
	int cmd = 0;
	while((cmd = getopt_long(argc,argv,"vVhHdD",cmd_optons,NULL))!=-1)
	{
		switch(cmd)
		{
			case 'v':
			case 'V':
			{
				printf("Verson: %s \n",VERSION);
				return 1;
			}
			break;
			case 'h':
			case 'H':
			{
				printf("Usage: %s [options]\n", argv[0]);
				printf("Options:\n");
				printf("  -v\tDisplay %s version information\n", argv[0]);
				printf("  -h\tDisplay help information\n");
				printf("  -d\tenable show log with console\n");
				printf("Usage: %s ifname \n", argv[0]);
				printf("Eample: %s eth0 -d \n", argv[0]);
				return 1;
			}
			break;
			case 'd':
			case 'D':
			{
				use_log_debug = true;
				printf("RouteProbe enable log debug\n");
			}
			break;
			default:
				return 1;
			break;
		}
	}
	
	return 0;
}

int main(int argc ,char **argv)
{
	if(argc < 2)
	{
		main_usage(argv[0]);
        printf("attempting to use eth0\n");
        return FAILE;
	}
	else
	{
		dev = argv[1];
	}

	if(main_parse_cmd(argc,argv))
	{
		return SUCCE;
	}
	
	log_debug("Device:%s\n",dev);
	
	route_timerInit(CHANNEL_TIME);

	//route_httpInit("211.149.188.172",8080);

	route_devListInit();
	
	route_pcapStart(dev);

	return SUCCE;
}
