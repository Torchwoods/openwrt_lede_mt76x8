#ifndef _LOG_H__
#define _LOG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <syslog.h>

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

extern bool use_log_debug;

void gprintf(const char *fmt ,...);
char*  getlocaltime(void);

//#ifdef UNDEBUG

#define ASSERT(expr)                                          \
	 do {													  \
	  if (!(expr)) {										  \
		fprintf(stderr, 									  \
				"Assertion failed in %s on line %d: %s\n",	  \
				__FILE__,									  \
				__LINE__,									  \
            #expr);                                       	  \
		abort();											  \
	  } 													  \
	 } while (0)
	 
#define  printlog(M,...)		fprintf(stderr,M,##__VA_ARGS__);
#define  log_debug(M,...)	gprintf("[%s][Debug][%s:%d]" M,getlocaltime(),__FUNCTION__,__LINE__,##__VA_ARGS__);
//#else
//#define ASSERT(expr) NULL
//#define log_debug(M...)	NULL
//#define printlog(M,...)		NULL

//#endif

//#define clean_errno() 			(errno==0 ? "None":strerror(errno))
#define log_err(M,...)			gprintf("[%s][Error][%s:%d]" M,getlocaltime(),__FUNCTION__,__LINE__,##__VA_ARGS__);//fprintf(stderr,"[Error][%s:%d] %s " M,__FUNCTION__,__LINE__,clean_errno(),##__VA_ARGS__);
#define log_warn(M,...) 		gprintf("[%s][Warring][%s:%d]" M,getlocaltime(),__FUNCTION__,__LINE__,##__VA_ARGS__);//fprintf(stderr,"[Warring][%s:%d] %s " M,__FUNCTION__,__LINE__,clean_errno(),##__VA_ARGS__);
#define log_info(M,...) 		gprintf("[%s][Info][%s:%d]" M,getlocaltime(),__FUNCTION__,__LINE__,##__VA_ARGS__);//fprintf(stderr,"[Info][%s:%d] " M,__FUNCTION__,__LINE__,##__VA_ARGS__);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
