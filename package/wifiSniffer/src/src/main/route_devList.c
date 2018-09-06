#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

#include "route_devList.h"
#include "route_dev.h"
#include "queue.h"
#include "log.h"
#include "types.h"
#include "cJSON.h"
#include "util.h"
//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER) 

#define container_of(ptr, type, member) ({	    \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})


typedef struct devList_t 
{
	devInfo_t devInfo;
	LIST_ENTRY(devList_t) entry_;
}devList_t;

LIST_HEAD(route_devList, devList_t) ;

static int devCnt = 0;

static struct route_devList devListHead ;

devInfo_t *route_devListGetByMac(char *mac)
{
	devList_t  *devList=NULL;

	ASSERT(mac != NULL);

	log_debug("vEventList_GetEventByEventID++\n");

    LIST_FOREACH(devList,&devListHead,entry_)
    {
		if(memcmp(mac,devList->devInfo.devmac,strlen(mac))==0)
		{
			return &(devList->devInfo);
		}
    }	

    return NULL;
}

int route_devListAdd(devInfo_t *devInfo)
{
	devInfo_t  *dev=NULL;
	devList_t  *devList = NULL;
	
	ASSERT(devInfo != NULL);

	dev = route_devListGetByMac(devInfo->devmac);
	if(dev == NULL)
	{
		devList = malloc(sizeof(devList_t));
		if(devList == NULL)
		{
			log_err("malloc failed\n");
			return FAILE;
		}

		memcpy(&devList->devInfo,devInfo,sizeof(devInfo_t));
		LIST_INSERT_HEAD(&devListHead,devList,entry_);
		devCnt++;
	}
	else
	{
		route_devListModify(dev);
	}

	return SUCCE;
}

int route_devListDel(char *mac)
{
	devList_t *devList = NULL;
	devInfo_t *devInfo = NULL;
	
	ASSERT(mac != NULL);
	
	log_debug("route_devListDel++\n");

	do{
		devInfo = route_devListGetByMac(mac);
		if(devInfo ==NULL)
			break;

		devList = container_of(devInfo,devList_t,devInfo);

		LIST_REMOVE(devList,entry_);

//		if(devList->devInfo.ssid != NULL)
//			free(devList->devInfo.ssid);

//		if(devList->devInfo.bssid!= NULL)
//			free(devList->devInfo.bssid);
		free(devList);
		devCnt--;
		return SUCCE;
		
	}while(0);

	log_debug("route_devListDel--\n");

	return FAILE;
}

int route_devListModify(devInfo_t *devInfo)
{
	devInfo_t *dev = NULL;
	
	ASSERT(devInfo != NULL);

	dev = route_devListGetByMac(devInfo->devmac);
	if(dev == NULL)
	{
		log_err("Not Fount devInfo");
		return FAILE;
	}

	if(dev->signel != devInfo->signel)
		dev->signel = devInfo->signel;
		
	if(dev->time != devInfo->time)
		dev->time = devInfo->time;

	if(memcmp(devInfo->ssid,dev->ssid,strlen(devInfo->ssid))!=0)
		memcpy(devInfo->ssid,dev->ssid,strlen(devInfo->ssid));
	
	if(memcmp(devInfo->bssid,dev->bssid,strlen(devInfo->bssid))!=0)
		memcpy(devInfo->bssid,dev->bssid,strlen(devInfo->bssid));
	
	return SUCCE;
}

char *route_cJsonCreate(void)
{
	int cnt = 0;
	char *out = NULL;
	char *devmac = NULL;
	cJSON *arrays=NULL,*objects = NULL;
	devList_t  *devList=NULL;

	uint8_t climac[32] = {0};
	
	uint8_t bssid[32] = {0};

	
	log_debug("route_devListGetAllForJson++\n");
	devmac = route_devGetMac();
	
	if(devmac == NULL)
		return NULL;
	
	arrays = cJSON_CreateArray();
	
    LIST_FOREACH(devList,&devListHead,entry_)
    {
		cJSON_AddItemToArray(arrays,objects=cJSON_CreateObject());
		cJSON_AddStringToObject(objects, "devmac", devmac);

		if(strlen(devList->devInfo.devmac))
		{
			sprintf(climac,MAC_FMT,MAC_PAR(devList->devInfo.devmac));
			cJSON_AddStringToObject(objects, "climac", climac);
		}
		else
			cJSON_AddStringToObject(objects, "climac", "");

		if(strlen(devList->devInfo.bssid))
		{
			sprintf(bssid,MAC_FMT,MAC_PAR(devList->devInfo.bssid));
			cJSON_AddStringToObject(objects, "bssid", climac);
		}
		else
			cJSON_AddStringToObject(objects, "bssid", "");
			
		cJSON_AddStringToObject(objects, "ssid", devList->devInfo.ssid);
		cJSON_AddNumberToObject(objects, "time", devList->devInfo.time);
		cJSON_AddNumberToObject(objects, "signal", devList->devInfo.signel);

		LIST_REMOVE(devList,entry_);

//		if(devList->devInfo.ssid != NULL)
//			free(devList->devInfo.ssid);

//		if(devList->devInfo.bssid!= NULL)
//			free(devList->devInfo.bssid);
		free(devList);
		devCnt--;
    }	

	out=cJSON_Print(arrays);	
	cJSON_Delete(arrays);	
//	log_debug("%s\n",out);
	log_debug("route_devListGetAllForJson--\n");
	return out;
}

int route_getCnt(void)
{
	return devCnt;
}

void route_cJSONFree(char *out)
{
	if(out != NULL) free(out);
}

void route_devListInit(void)
{
	LIST_INIT(&devListHead);
}

