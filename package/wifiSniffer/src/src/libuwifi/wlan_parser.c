/*
 * libuwifi - Userspace Wifi Library
 *
 * Copyright (C) 2005-2016 Bruno Randolf (br1@einfach.org)
 *
 * This source code is licensed under the GNU Lesser General Public License,
 * Version 3. See the file COPYING for more details.
 */

#include "platform.h"
#include "wlan80211.h"
#include "util.h"
//#include "channel.h"
#include "wlan_util.h"
#include "wlan_parser.h"
#include "log.h"
#include "types.h"

void uwifi_parse_information_elements(unsigned char* buf, size_t bufLen, struct uwifi_packet *p)
{
	int len = bufLen;
	while (len > 2) {
		struct information_element* ie = (struct information_element*)buf;
//		printlog("------ IE %d len %d t len %d\n", ie->id, ie->len, len);

		switch (ie->id) {
		case WLAN_IE_ID_SSID:
			if (ie->len < WLAN_MAX_SSID_LEN-1) {
				memcpy(p->wlan_essid, ie->var, ie->len);
				p->wlan_essid[ie->len] = '\0';
			} else {
				memcpy(p->wlan_essid, ie->var, WLAN_MAX_SSID_LEN-1);
				p->wlan_essid[WLAN_MAX_SSID_LEN-1] = '\0';
			}
			break;

		case WLAN_IE_ID_DSSS_PARAM:
			p->wlan_channel = *ie->var;
			break;

		case WLAN_IE_ID_RSN:
			p->wlan_rsn = 1;
			break;

		case WLAN_IE_ID_HT_CAPAB:
//			printlog("HT %d %x\n", ie->len, ie->var[0]);
			if (ie->var[0] & WLAN_IE_HT_CAPAB_INFO_CHAN_WIDTH_40)
				p->wlan_chan_width = CHAN_WIDTH_40;
			else
				p->wlan_chan_width = CHAN_WIDTH_20;

			if (ie->len >= 26) {
				wlan_ht_streams_from_mcs(&ie->var[3], &p->wlan_rx_streams, &p->wlan_tx_streams);
//				printlog("STREAMS %dx%d\n", p->wlan_tx_streams, p->wlan_rx_streams);
			}
			break;

		case WLAN_IE_ID_HT_OPER:
//			printlog("HT OPER %d %x\n", ie->len, ie->var[0]);
			if (ie->len > 1) {
				switch (ie->var[1] & WLAN_IE_HT_OPER_INFO_CHAN_OFFSET) {
					case 0: p->wlan_chan_width = CHAN_WIDTH_20; break;
					case 1: p->wlan_ht40plus = true; break;
					case 3: p->wlan_ht40plus = false; break;
					default: /*printlog("HT OPER wrong?");*/ break;
				}
			}
			break;

		case WLAN_IE_ID_VHT_OPER:
		case WLAN_IE_ID_VHT_OMN:
//			printlog("VHT OPER %d %x\n", ie->len, ie->var[0]);
			p->wlan_chan_width = CHAN_WIDTH_80; /* minimum, otherwise not AC */
			break;

		case WLAN_IE_ID_VHT_CAPAB:
//			printlog("VHT %d %x\n", ie->len, ie->var[0]);
			if (ie->len >= 12) {
				p->wlan_chan_width = wlan_chan_width_from_vht_capab(ie->var[0]);
				wlan_vht_streams_from_mcs(&ie->var[4], &p->wlan_rx_streams, &p->wlan_tx_streams);
//				printlog("VHT STREAMS %dx%d\n", p->wlan_tx_streams, p->wlan_rx_streams);
			}
			break;

		case WLAN_IE_ID_VENDOR:
			if (ie->len >= 4 &&
			    ie->var[0] == 0x00 && ie->var[1] == 0x50 && ie->var[2] == 0xf2 && /* Microsoft OUI (00:50:F2) */
			    ie->var[3] == 1) {	/* OUI Type 1 - WPA IE */
				p->wlan_wpa=1;
			}

			break;
		}

		buf += ie->len + 2;
		len -= ie->len + 2;
	}
}

/* return consumed length, 0 for stop parsing, or -1 on error */
int uwifi_parse_80211_header(unsigned char* buf, size_t len, struct uwifi_packet* p)
{
	struct wlan_frame* wh;
	size_t hdrlen;
	uint8_t* ra = NULL;
	uint8_t* ta = NULL;
	uint8_t* bssid = NULL;
	uint16_t fc, cap_i;

	if (len < 10) /* minimum frame size (CTS/ACK) */
		return -1;

	p->wlan_mode = WLAN_MODE_UNKNOWN;

	wh = (struct wlan_frame*)buf;

	fc = le16toh(wh->fc);
	p->wlan_type = (fc & WLAN_FRAME_FC_MASK);
	log_debug("wlan_type %x - type %x - stype %x\n", fc, fc & WLAN_FRAME_FC_TYPE_MASK, fc & WLAN_FRAME_FC_STYPE_MASK);
	log_debug("%s\n", wlan_get_packet_type_name(fc));

	if(!(WLAN_FRAME_IS_DATA(fc))&&!(WLAN_FRAME_IS_MGMT(fc)))
		return -1;

	if (WLAN_FRAME_IS_DATA(fc)) {

		hdrlen = 24;
		if (WLAN_FRAME_IS_QOS(fc)) {
			hdrlen += 2;
			if (fc & WLAN_FRAME_FC_ORDER)
				hdrlen += 4;
		}

		/* AP, STA or IBSS and address fields */
		if ((fc & WLAN_FRAME_FC_FROM_DS) == 0 &&
		    (fc & WLAN_FRAME_FC_TO_DS) == 0) {
			p->wlan_mode = WLAN_MODE_IBSS;
			ra = wh->addr1;
			ta = wh->addr2;
			bssid = wh->addr3;
		} else if ((fc & WLAN_FRAME_FC_FROM_DS) &&
			   (fc & WLAN_FRAME_FC_TO_DS)) {
			p->wlan_mode = WLAN_MODE_4ADDR;
			hdrlen += 6;
			if (WLAN_FRAME_IS_QOS(fc)) {
				uint16_t qos = le16toh(wh->u.addr4_qos_ht.qos);
//				printlog("4ADDR A-MSDU %x\n", qos & WLAN_FRAME_QOS_AMSDU_PRESENT);
				if (qos & WLAN_FRAME_QOS_AMSDU_PRESENT)
					bssid = wh->addr3;
				// in the MSDU case BSSID is unknown
			}
			ra = wh->addr1;
			ta = wh->addr2;
		} else if (fc & WLAN_FRAME_FC_FROM_DS) {
			p->wlan_mode = WLAN_MODE_AP;
			ra = wh->addr1;
			bssid = wh->addr2;
			ta = wh->addr3;
		} else if (fc & WLAN_FRAME_FC_TO_DS) {
			p->wlan_mode = WLAN_MODE_STA;
			bssid = wh->addr1;
			ta = wh->addr2;
			ra = wh->addr3;
		}

		if (len < hdrlen)
			return -1;

		p->wlan_nav = le16toh(wh->duration);
//		printlog("DATA NAV %d\n", p->wlan_nav);
		p->wlan_seqno = (le16toh(wh->seq) & WLAN_FRAME_SEQ_MASK) >> 4;
//		printlog("DATA SEQ %d\n", p->wlan_seqno);

//		printlog("A1 " MAC_FMT "\n", MAC_PAR(wh->addr1));
//		printlog("A2 " MAC_FMT "\n", MAC_PAR(wh->addr2));
//		printlog("A3 " MAC_FMT "\n", MAC_PAR(wh->addr3));
		if (p->wlan_mode == WLAN_MODE_4ADDR) {
//			printlog("A4 " MAC_FMT "\n", MAC_PAR(wh->u.addr4));
		}
//		printlog("ToDS %d FromDS %d\n", (fc & WLAN_FRAME_FC_FROM_DS) != 0, (fc & WLAN_FRAME_FC_TO_DS) != 0);

		/* WEP */
		if (fc & WLAN_FRAME_FC_PROTECTED)
			p->wlan_wep = 1;

		if (fc & WLAN_FRAME_FC_RETRY)
			p->wlan_retry = 1;

	} else if (WLAN_FRAME_IS_CTRL(fc)) {
		if (p->wlan_type == WLAN_FRAME_CTS ||
		    p->wlan_type == WLAN_FRAME_ACK)
			hdrlen = 10;
		else
			hdrlen = 16;

		if (len < hdrlen)
			return -1;

	} else if (WLAN_FRAME_IS_MGMT(fc)) {
		hdrlen = 24;
		if (fc & WLAN_FRAME_FC_ORDER)
			hdrlen += 4;

		if (len < hdrlen)
			return -1;

		ra = wh->addr1;
		ta = wh->addr2;
		bssid = wh->addr3;
		p->wlan_seqno = (le16toh(wh->seq) & WLAN_FRAME_SEQ_MASK) >> 4;
//		printlog("MGMT SEQ %d\n", p->wlan_seqno);

		if (fc & WLAN_FRAME_FC_RETRY)
			p->wlan_retry = 1;
	} else {
		log_err("!!!UNKNOWN FRAME!!!");
		return -1;
	}

	p->wlan_len = len;

	switch (p->wlan_type) {
		case WLAN_FRAME_NULL:
			break;

		case WLAN_FRAME_QDATA:
			p->wlan_qos_class = le16toh(wh->u.qos) & WLAN_FRAME_QOS_TID_MASK;
//			printlog("***QDATA %x\n", p->wlan_qos_class);
			break;

		case WLAN_FRAME_RTS:
			p->wlan_nav = le16toh(wh->duration);
//			printlog("RTS NAV %d\n", p->wlan_nav);
			ra = wh->addr1;
			ta = wh->addr2;
			break;

		case WLAN_FRAME_CTS:
			p->wlan_nav = le16toh(wh->duration);
//			printlog("CTS NAV %d\n", p->wlan_nav);
			ra = wh->addr1;
			break;

		case WLAN_FRAME_ACK:
			p->wlan_nav = le16toh(wh->duration);
//			printlog("ACK NAV %d\n", p->wlan_nav);
			ra = wh->addr1;
			break;

		case WLAN_FRAME_PSPOLL:
			ra = wh->addr1;
			bssid = wh->addr1;
			ta = wh->addr2;
			break;

		case WLAN_FRAME_CF_END:
		case WLAN_FRAME_CF_END_ACK:
			ra = wh->addr1;
			ta = wh->addr2;
			bssid = wh->addr2;
			break;

		case WLAN_FRAME_BLKACK:
		case WLAN_FRAME_BLKACK_REQ:
			p->wlan_nav = le16toh(wh->duration);
			ra = wh->addr1;
			ta = wh->addr2;
			break;

		case WLAN_FRAME_BEACON:
		case WLAN_FRAME_PROBE_RESP:
			;
			struct wlan_frame_beacon* bc = (struct wlan_frame_beacon*)(buf + hdrlen);
			p->wlan_tsf = le64toh(bc->tsf);
			p->wlan_bintval = le16toh(bc->bintval);
			//printlog("TSF %u\n BINTVAL %u", p->wlan_tsf, p->wlan_bintval);

			uwifi_parse_information_elements(bc->ie,
				len - hdrlen - sizeof(struct wlan_frame_beacon) - 4 /* FCS */, p);
//			printlog("ESSID %s \n", p->wlan_essid );
//			printlog("CHAN %d \n", p->wlan_channel );
			cap_i = le16toh(bc->capab);
			if (cap_i & WLAN_CAPAB_IBSS)
				p->wlan_mode = WLAN_MODE_IBSS;
			else if (cap_i & WLAN_CAPAB_ESS)
				p->wlan_mode = WLAN_MODE_AP;
			if (cap_i & WLAN_CAPAB_PRIVACY)
				p->wlan_wep = 1;
			break;

		case WLAN_FRAME_PROBE_REQ:
			uwifi_parse_information_elements(buf + hdrlen,
				len - hdrlen - 4 /* FCS */, p);
			p->wlan_mode = WLAN_MODE_PROBE;
			break;

		case WLAN_FRAME_ASSOC_REQ:
		case WLAN_FRAME_ASSOC_RESP:
		case WLAN_FRAME_REASSOC_REQ:
		case WLAN_FRAME_REASSOC_RESP:
		case WLAN_FRAME_DISASSOC:
			break;

		case WLAN_FRAME_AUTH:
			if (fc & WLAN_FRAME_FC_PROTECTED)
				p->wlan_wep = 1;
				/* no break */
		case WLAN_FRAME_DEAUTH:
			break;

		case WLAN_FRAME_ACTION:
			break;
	}

	if (ta != NULL) {
		memcpy(p->wlan_src, ta, WLAN_MAC_LEN);
//		printlog("TA    " MAC_FMT "\n", MAC_PAR(ta));
	}
	if (ra != NULL) {
		memcpy(p->wlan_dst, ra, WLAN_MAC_LEN);
//		printlog("RA    " MAC_FMT "\n", MAC_PAR(ra));
	}
	if (bssid != NULL) {
		memcpy(p->wlan_bssid, bssid, WLAN_MAC_LEN);
//		printlog("BSSID " MAC_FMT "\n", MAC_PAR(bssid));
	}

	/* only data frames contain more info, otherwise stop parsing */
	if (WLAN_FRAME_IS_DATA(p->wlan_type) && p->wlan_wep != 1) {
		return hdrlen;
	}
	return 0;
}
