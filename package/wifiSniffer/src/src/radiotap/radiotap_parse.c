#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <endian.h>
#include <errno.h>
#include <string.h>
#include "radiotap_iter.h"
#include "log.h"
#include "types.h"
#include "wlan_parser.h"

//#define IEEE80211_CHAN_A (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)  
//#define IEEE80211_CHAN_G (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM)  

//static char signal = 0;

#if 0
static void print_radiotap_namespace(struct ieee80211_radiotap_iterator *iter)
{
    uint32_t phy_freq = 0;  
    
	switch (iter->this_arg_index) 
	{
	case IEEE80211_RADIOTAP_TSFT:
		printlog("\tTSFT: %llu\n", le64toh(*(unsigned long long *)iter->this_arg));
		break;
	case IEEE80211_RADIOTAP_FLAGS:
		printlog("\tFlags: 0x%02x\n", *iter->this_arg);
		break;
	//速率
	case IEEE80211_RADIOTAP_RATE:
		printlog("\trate: %#.1f Mb/s\n", (double)*iter->this_arg/2);
		break;
		
    // 通信信息  
	case IEEE80211_RADIOTAP_CHANNEL:
	  	phy_freq = le16toh(*(uint16_t*)iter->this_arg); // 信道  
        iter->this_arg = iter->this_arg + 2; 		// 通道信息如2G、5G，等  
        int x = le16toh(*(uint16_t*)iter->this_arg);  
        printlog("\tfreq: %d\n", phy_freq);  
        if ((x & IEEE80211_CHAN_A) == IEEE80211_CHAN_A)  
        {  
            printlog("\ttype: A\n");  
        }  
        else if ((x & IEEE80211_CHAN_G) == IEEE80211_CHAN_G)  
        {  
            printlog("\ttype: G\n");  
        }  
        else if ((x & IEEE80211_CHAN_2GHZ) == IEEE80211_CHAN_2GHZ)  
        {  
            printlog("\ttype: B\n");  
        }  
        break; 
	//信号强度
	case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
	 	signal = *(signed char*)iter->this_arg;  
        printlog("\tsignal: %d dBm\n", signal);  
        break;
    case IEEE80211_RADIOTAP_ANTENNA:
		printlog("\tantenna: %x\n", *iter->this_arg);
		break;
	//接收标志
	case IEEE80211_RADIOTAP_RX_FLAGS:
		printlog("\tRX flags: 0x%#.4x\n",le16toh(*(uint16_t *)iter->this_arg));
		break;
	case IEEE80211_RADIOTAP_DBM_ANTNOISE:
	case IEEE80211_RADIOTAP_LOCK_QUALITY:
	case IEEE80211_RADIOTAP_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DB_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DBM_TX_POWER:
	case IEEE80211_RADIOTAP_DB_ANTSIGNAL:
	case IEEE80211_RADIOTAP_DB_ANTNOISE:
	case IEEE80211_RADIOTAP_TX_FLAGS:
		break;
	case IEEE80211_RADIOTAP_RTS_RETRIES:
	case IEEE80211_RADIOTAP_FHSS:
	case IEEE80211_RADIOTAP_DATA_RETRIES:
		break;
	default:
		printlog("\tBOGUS DATA\n");
		break;
	}
}

int radiotap_parse(char *radiotapbuf,int16_t size)
{
	struct ieee80211_radiotap_iterator iter;
	int err;
	
	ASSERT(radiotapbuf != NULL);
	
	err = ieee80211_radiotap_iterator_init(&iter, (struct ieee80211_radiotap_header *)radiotapbuf, size, NULL);
	if (err) {
		printlog("malformed radiotap header (init returns %d)\n", err);
		return -1;
	}

	while (!(err = ieee80211_radiotap_iterator_next(&iter))) {
		if (iter.is_radiotap_ns)
			print_radiotap_namespace(&iter);
	}

	printlog("error:%d\n",err);
	return 0;
}

char radiotap_getSignal(void)
{
	return signal;
}
#endif

static void get_radiotap_info(struct ieee80211_radiotap_iterator *iter, struct uwifi_packet* p)
{
	uint16_t x;
	signed char c;
	unsigned char known, flags, ht20, lgi;

	switch (iter->this_arg_index) {
	/* ignoring these */
	case IEEE80211_RADIOTAP_TSFT:
	case IEEE80211_RADIOTAP_FHSS:
	case IEEE80211_RADIOTAP_LOCK_QUALITY:
	case IEEE80211_RADIOTAP_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DB_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DBM_TX_POWER:
	case IEEE80211_RADIOTAP_RX_FLAGS:
	case IEEE80211_RADIOTAP_RTS_RETRIES:
	case IEEE80211_RADIOTAP_DATA_RETRIES:
	case IEEE80211_RADIOTAP_AMPDU_STATUS:
		break;
	case IEEE80211_RADIOTAP_TX_FLAGS:
		/* when TX flags are present we can conclude that a userspace
		 * program has injected this packet */
		p->phy_injected = true;
		break;
	case IEEE80211_RADIOTAP_FLAGS:
		/* short preamble */
//		printlog("[flags %0x", *iter->this_arg);
		if (*iter->this_arg & IEEE80211_RADIOTAP_F_SHORTPRE) {
			p->phy_flags |= PHY_FLAG_SHORTPRE;
//			printlog(" shortpre");
		}
		if (*iter->this_arg & IEEE80211_RADIOTAP_F_BADFCS) {
			p->phy_flags |= PHY_FLAG_BADFCS;
//			printlog(" badfcs");
		}
//		printlog("]");
		break;
	case IEEE80211_RADIOTAP_RATE:
		//TODO check!
		//printlog("\trate: %lf\n", (double)*iter->this_arg/2);
//		printlog("[rate %0x]", *iter->this_arg);
		p->phy_rate = (*iter->this_arg)*5; /* rate is in 500kbps */
		p->phy_rate_idx = wlan_rate_to_index(p->phy_rate);
		break;
#define IEEE80211_CHAN_A \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)
#define IEEE80211_CHAN_G \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM)
	case IEEE80211_RADIOTAP_CHANNEL:
		/* channel & channel type */
		p->phy_freq = le16toh(*(uint16_t*)iter->this_arg);
//		printlog("[freq %d", p->phy_freq);
		iter->this_arg = iter->this_arg + 2;
		x = le16toh(*(uint16_t*)iter->this_arg);
		if ((x & IEEE80211_CHAN_A) == IEEE80211_CHAN_A) {
			p->phy_flags |= PHY_FLAG_A;
//			printlog("A]");
		}
		else if ((x & IEEE80211_CHAN_G) == IEEE80211_CHAN_G) {
			p->phy_flags |= PHY_FLAG_G;
//			printlog("G]");
		}
		else if ((x & IEEE80211_CHAN_2GHZ) == IEEE80211_CHAN_2GHZ) {
			p->phy_flags |= PHY_FLAG_B;
//			printlog("B]");
		}
		break;
	case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
		c = *(signed char*)iter->this_arg;
//		printlog("[sig %0d]", c);
		/* we get the signal per rx chain with newer drivers.
		 * save the highest value, but make sure we don't override
		 * with invalid values */
		if (c < 0 && (p->phy_signal == 0 || c > p->phy_signal))
			p->phy_signal = c;
		break;
	case IEEE80211_RADIOTAP_DBM_ANTNOISE:
//		printlog("[noi %0x]", *(signed char*)iter->this_arg);
		// usually not present
		//p->phy_noise = *(signed char*)iter->this_arg;
		break;
	case IEEE80211_RADIOTAP_ANTENNA:
//		printlog("[ant %0x]", *iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_DB_ANTSIGNAL:
//		printlog("[snr %0x]", *iter->this_arg);
		// usually not present
		//p->phy_snr = *iter->this_arg;
		break;
	case IEEE80211_RADIOTAP_DB_ANTNOISE:
		//printlog("\tantnoise: %02d\n", *iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_MCS:
		/* Ref http://www.radiotap.org/defined-fields/MCS */
		known = *iter->this_arg++;
		flags = *iter->this_arg++;
//		printlog("[MCS known %0x flags %0x index %0x]", known, flags, *iter->this_arg);
		if (known & IEEE80211_RADIOTAP_MCS_HAVE_BW)
			ht20 = (flags & IEEE80211_RADIOTAP_MCS_BW_MASK) == IEEE80211_RADIOTAP_MCS_BW_20;
		else
			ht20 = 1; /* assume HT20 if not present */

		if (known & IEEE80211_RADIOTAP_MCS_HAVE_GI)
			lgi = !(flags & IEEE80211_RADIOTAP_MCS_SGI);
		else
			lgi = 1; /* assume long GI if not present */

//		printlog(" %s %s", ht20 ? "HT20" : "HT40", lgi ? "LGI" : "SGI");

		p->phy_rate_idx = 12 + *iter->this_arg;
		p->phy_rate_flags = flags;
		p->phy_rate = wlan_ht_mcs_to_rate(*iter->this_arg, ht20, lgi);

//		printlog(" RATE %d ", p->phy_rate);
		break;
	default:
		log_err("UNKNOWN RADIOTAP field %d", iter->this_arg_index);
		break;
	}
}

int uwifi_parse_radiotap(unsigned char* buf, size_t len, struct uwifi_packet* p)
{
	struct ieee80211_radiotap_header* rh;
	struct ieee80211_radiotap_iterator iter;
	int err, rt_len;

	if (len < sizeof(struct ieee80211_radiotap_header))
		return -1;

	rh = (struct ieee80211_radiotap_header*)buf;
	rt_len = le16toh(rh->it_len);

	err = ieee80211_radiotap_iterator_init(&iter, rh, rt_len, NULL);
	if (err) {
		log_err("malformed radiotap header (init returns %d)\n", err);
		return -1;
	}

//	printlog("Radiotap: ");
	while (!(err = ieee80211_radiotap_iterator_next(&iter))) {
		if (iter.is_radiotap_ns) {
			get_radiotap_info(&iter, p);
		}
	}

//	printlog("\nSIG %d", p->phy_signal);

	/* sanitize */
	if (p->phy_rate == 0 || p->phy_rate > 6000) {
		/* assume min rate for mode */
		printlog("*** fixing wrong rate\n");
		if (p->phy_flags & PHY_FLAG_A)
			p->phy_rate = 120; /* 6 * 2 */
		else if (p->phy_flags & PHY_FLAG_B)
			p->phy_rate = 20; /* 1 * 2 */
		else if (p->phy_flags & PHY_FLAG_G)
			p->phy_rate = 120; /* 6 * 2 */
		else
			p->phy_rate = 20;
	}

//	printlog("\nrate: %.2f = idx %d\n", (float)p->phy_rate/10, p->phy_rate_idx);
//	printlog("signal: %d\n", p->phy_signal);

	if (p->phy_flags & PHY_FLAG_BADFCS) {
		/* we can't trust frames with a bad FCS - stop parsing */
		log_err("=== bad FCS, stop ===\n");
		return 0;
	} else {
		return rt_len;
	}
}


