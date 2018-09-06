#include <stdio.h>
#include <pcap.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <time.h>

#include "route_timer.h"
#include "log.h"
#include "route_pcap.h"
#include "types.h"
#include "route_http.h"
#include "wlan_parser.h"
#include "route_devList.h"
static pcap_t *handle;                         /* Pcap session handle */

static void print_packet_info(const u_char *packet, struct pcap_pkthdr packet_header) {
    log_debug("Packet capture length: %d\n", packet_header.caplen);
    log_debug("Packet total length %d\n", packet_header.len);
}

static int route_getDeviceMac(struct uwifi_packet *p,devInfo_t *devInfo)
{
	log_debug("route_wlanparse++\n");
	log_debug("p->wlan_type=%x\n",p->wlan_type);
	
	switch (p->wlan_type) {
	//管理帧
	case WLAN_FRAME_BEACON:
	case WLAN_FRAME_PROBE_RESP:
	case WLAN_FRAME_ASSOC_REQ:
	case WLAN_FRAME_ASSOC_RESP:
	case WLAN_FRAME_REASSOC_REQ:
	case WLAN_FRAME_REASSOC_RESP:
	case WLAN_FRAME_DISASSOC:
	case WLAN_FRAME_AUTH:
	case WLAN_FRAME_DEAUTH:
	case WLAN_FRAME_ACTION:
	break;
	case WLAN_FRAME_PROBE_REQ: 
		log_debug("WLAN_FRAME_PROBE_REQ\n");
		log_debug("%s\n", wlan_get_packet_type_name(p->wlan_type));
		memcpy(devInfo->devmac,p->wlan_src,WLAN_MAC_LEN);
		log_debug("TA "MAC_FMT"\n",MAC_PAR(p->wlan_src));
		log_debug("RA "MAC_FMT"\n",MAC_PAR(p->wlan_dst));
		return 0;
		break;
	//数据帧
	case WLAN_FRAME_DATA:
		log_debug("WLAN_FRAME_DATA\n");
		memcpy(devInfo->devmac,p->wlan_src,WLAN_MAC_LEN);
		log_debug("TA "MAC_FMT"\n",MAC_PAR(p->wlan_src));
		log_debug("RA "MAC_FMT"\n",MAC_PAR(p->wlan_dst));
		return 0;
		break;
	case WLAN_FRAME_DATA_CF_POLL:	
		log_debug("WLAN_FRAME_DATA_CF_POLL\n");
		memcpy(devInfo->devmac,p->wlan_src,WLAN_MAC_LEN);
		log_debug("TA "MAC_FMT"\n",MAC_PAR(p->wlan_src));
		log_debug("RA "MAC_FMT"\n",MAC_PAR(p->wlan_dst));
		return 0;
		break;
	case WLAN_FRAME_NULL:			
		log_debug("WLAN_FRAME_NULL\n");
		memcpy(devInfo->devmac,p->wlan_src,WLAN_MAC_LEN);
		log_debug("TA "MAC_FMT"\n",MAC_PAR(p->wlan_src));
		log_debug("RA "MAC_FMT"\n",MAC_PAR(p->wlan_dst));
		return 0;
		break;
	case WLAN_FRAME_QDATA:		
		log_debug("WLAN_FRAME_QDATA\n");
		memcpy(devInfo->devmac,p->wlan_src,WLAN_MAC_LEN);
		log_debug("TA "MAC_FMT"\n",MAC_PAR(p->wlan_src));
		log_debug("RA "MAC_FMT"\n",MAC_PAR(p->wlan_dst));
		return 0;
		break;
	case WLAN_FRAME_QDATA_CF_POLL:
		log_debug("WLAN_FRAME_QDATA_CF_POLL\n");
		memcpy(devInfo->devmac,p->wlan_src,WLAN_MAC_LEN);
		log_debug("TA "MAC_FMT"\n",MAC_PAR(p->wlan_src));
		log_debug("RA "MAC_FMT"\n",MAC_PAR(p->wlan_dst));
		return 0;
		break;
	}
	
	log_debug("route_wlanparse--\n");
	return -1;
}

//解析数据
void route_ProceePktHandle(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	static unsigned int idx = 0;
	devInfo_t devInfo;
	char *out = NULL;
	struct uwifi_packet p;
	int ret = 0;

	memset(&p, 0, sizeof(p));
	ret = uwifi_parse_radiotap(packet,header->len,&p);
	if(ret == 0)
		return ;

	ret = uwifi_parse_80211_header(packet + ret, header->len - ret, &p);
	if(ret < 0)
		return;

#if 0
	if((WLAN_FRAME_IS_DATA(p.wlan_type))||(WLAN_FRAME_IS_MGMT(p.wlan_type)))
	{
		memset(&devInfo,0,sizeof(devInfo_t));
		ret = route_getDeviceMac(&p,&devInfo);
	}
#endif

	//娑堟伅杈撳嚭
	printf("Idx  SrcMac             DstMac 		   FrameType      SubType      Ch      RSSI\n");
	printf("%d ",idx++);
	printf(" %02x:%02x:%02x:%02x:%02x:%02x ",p.wlan_src[0],p.wlan_src[1],p.wlan_src[2],p.wlan_src[3],p.wlan_src[4],p.wlan_src[5]);
	printf(" %02x:%02x:%02x:%02x:%02x:%02x ",p.wlan_dst[0],p.wlan_dst[1],p.wlan_dst[2],p.wlan_dst[3],p.wlan_dst[4],p.wlan_dst[5]);
	printf(" %02x             %02x           %02d       %d\n",WLAN_FRAME_TYPE(p.wlan_type),WLAN_FRAME_STYPE(p.wlan_type),p.wlan_channel,p.phy_signal);
	
	
#if 0

	if(!ret)
	{
		time(&devInfo.time);
		devInfo.signel = p.phy_signal;
		route_devListAdd(&devInfo);		
	}
	
	log_debug("time = %d\n",route_getTime());
	
	if(route_getTime() > HTTP_POST_TIME)
	{
		route_TimeClear();
		out = route_cJsonCreate();
		log_debug("%s\n",out);
		route_httpPost(out,strlen(out));
		route_cJSONFree(out);
		route_startTimer(CHANNEL_TIME);
	}
#endif

	return;
}

int route_pcapStart(char *dev)
{
	char errbuf[PCAP_ERRBUF_SIZE]={0};

	log_debug("route_pcapInit++\n");
	
	handle = pcap_open_live(dev, BUFSIZ, 1, 5, errbuf); //5ms recv timeout

	if(!handle)
	{
		log_err("%s\n",errbuf);
		return FAILE;
	}
	
	pcap_loop(handle, -1, route_ProceePktHandle, NULL);
	pcap_close(handle);
	
	log_debug("route_pcapInit--\n");
	return SUCCE;
}

