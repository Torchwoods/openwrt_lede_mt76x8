#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>
#include <stdbool.h>

#include "log.h"
#include "types.h"
#include "route_dev.h"
#include "wifi_uci.h"

static char *g_devmac = NULL;
static char *g_url = NULL;

char* route_devGetMac(void)
{
	if(g_devmac != NULL)
		return g_devmac;
		
	g_devmac = wifi_uci_get("network","lan","macaddr");
	
	return g_devmac;
}

char* route_getUrl(void)
{
	if(g_url != NULL)
		return g_url;
		
	g_url = wifi_uci_get("sniffer","service","url");

	log_debug("URL:%s\n",g_url);
	return g_url;
}


