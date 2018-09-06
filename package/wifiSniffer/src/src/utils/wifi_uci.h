#ifndef _WIFI_UCI_H_
#define _WIFI_UCI_H_
#ifdef __cplusplus
extern "C" {
#endif

int wifi_uci_set( char*config, char*section,char*option,char*value);
char* wifi_uci_get(char*config, char*section,char*option);
int wifi_uci_del(char*config, char*section,char*option);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif