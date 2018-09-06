#ifndef __RADIOTAP_PARSE_H__
#define __RADIOTAP_PARSE_H__

int radiotap_parse(char *radiotapbuf,int16_t size);
char radiotap_getSignal(void);

#endif