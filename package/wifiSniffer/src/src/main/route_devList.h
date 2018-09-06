#ifndef __ROUTE_DEV_LIST_H__
#define __ROUTE_DEV_LIST_H__
#ifdef __cplusplus
extern "C" {
#endif

#define DEV_MAC_LEN		6
#define MAX_SSID_LEN    32
typedef struct _devInfo_t 
{
	int16_t signel;
	uint8_t devmac[DEV_MAC_LEN];
	uint8_t ssid  [MAX_SSID_LEN];
	uint8_t bssid [DEV_MAC_LEN];
	time_t time;
}devInfo_t;

devInfo_t *route_devListGetByMac(char *mac);
int route_devListAdd(devInfo_t *devInfo);
int route_devListDel(char *mac);
int route_devListModify(devInfo_t *devInfo);
char *route_cJsonCreate(void);
void route_cJSONFree(char *out);
void route_devListInit(void);
int route_getCnt(void);








#ifdef __cplusplus
}

#endif /* __cplusplus */
#endif

