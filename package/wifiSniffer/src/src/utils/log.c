#include "log.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>

void gprintf(const char *fmt ,...)
{
	va_list ap;
	va_start(ap,fmt);

	if(use_log_debug)
	{
		vfprintf(stderr, fmt, ap);
	}
	else
	{
		vsyslog(LOG_INFO | LOG_USER, fmt, ap);
	}
	
	va_end(ap);
}

char*  getlocaltime(void)
{
	static char times[24]={0};
	time_t timep;
	struct tm *tp;
	time(&timep);
	tp = localtime(&timep);
	sprintf(times,"%d/%d/%d %d:%d:%d",(1900+tp->tm_year),( 1+tp->tm_mon), tp->tm_mday,tp->tm_hour, tp->tm_min, tp->tm_sec);
	return times;
}


