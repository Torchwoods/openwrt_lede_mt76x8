#ifndef __ROUTE_TIMER_H__
#define __ROUTE_TIMER_H__
#ifdef __cplusplus
extern "C" {
#endif

#define CHANNEL_TIME	500

void route_timerInit(int ms);
int route_getTime(void);
void route_TimeClear(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
