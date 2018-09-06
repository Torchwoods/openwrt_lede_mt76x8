#include <stdio.h>  
#include <time.h>  
#include <sys/time.h>  
#include <stdlib.h>  
#include <signal.h> 
#include <stdint.h>
#include <stdbool.h>

#include "log.h"
#include "types.h"
#include "route_timer.h"

extern char *dev;

static uint8_t cur_channel = 1;

static int timeCnt = 0;

static struct itimerval timer;

int route_setChannel(int channel)
{
    char cmd[128] = {0};
    
	snprintf(cmd,128,"iw dev %s set channel %d",dev,channel);
	system(cmd);
	
	return 0;
}

void route_switchChannelCallback(int signo)
{
    if(cur_channel >= 13)
	    cur_channel = 1;
	else
	    cur_channel ++;

	//¶¨Ê±·¢ËÍ
	timeCnt++; 
	
	route_setChannel(cur_channel);
	log_debug("SwitchChannel %d,%d\n",cur_channel,timeCnt);
}

int route_getTime(void)
{
	return (CHANNEL_TIME*timeCnt/1000);
}
void route_TimeClear(void)
{
	timeCnt = 0;
}

void route_startTimer(int ms)
{
    time_t secs, usecs;
    secs = ms/1000;
    usecs = ms%1000 * 1000;

    timer.it_interval.tv_sec = secs;
    timer.it_interval.tv_usec = usecs;
    timer.it_value.tv_sec = secs;
    timer.it_value.tv_usec = usecs;

    setitimer(ITIMER_REAL, &timer, NULL);
}

void route_timerInit(int ms)
{
	signal(SIGALRM, route_switchChannelCallback);
	route_startTimer(ms);
}

