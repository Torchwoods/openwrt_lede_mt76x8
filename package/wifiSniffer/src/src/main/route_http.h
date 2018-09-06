#ifndef __ROUTE_HTTP_H__
#define __ROUTE_HTTP_H__
#ifdef __cplusplus
extern "C" {
#endif

#define HTTP_POST_TIME	5

int route_httpPost(char *str,uint16_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif