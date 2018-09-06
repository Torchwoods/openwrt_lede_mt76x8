#include <stdio.h>
#include <stdint.h>


#include <string.h>
#include <curl/curl.h>

#include "log.h"
#include "types.h"
#include "route_http.h"
#include "route_dev.h"

#define MAX_URL_SIEZ 128

//static char url[MAX_URL_SIEZ]={0};

void route_setShareHandle(CURL *curl_handle)
{
	static CURLSH *share_handle = NULL;
	if(!share_handle)
	{
		share_handle = curl_share_init();
		curl_share_setopt(share_handle,CURLSHOPT_SHARE,CURL_LOCK_DATA_DNS);
	}
	curl_easy_setopt( curl_handle, CURLOPT_SHARE,share_handle);
	curl_easy_setopt( curl_handle, CURLOPT_DNS_CACHE_TIMEOUT,60*5);
}

int route_httpPost(char *str,uint16_t len)
{
	char *url = NULL;
	CURL *curl = NULL;
	CURLcode res ;
	struct curl_slist *headers=NULL;
	struct curl_slist *plist = NULL;

	ASSERT(str != NULL);

	log_debug("route_httpPost++\n");
	
	url = route_getUrl();
	if(url == NULL)
	{
		log_err("url is disable\n");
		return FAILE;
	}

	curl_global_init(CURL_GLOBAL_ALL);
	
	curl = curl_easy_init();
	log_debug("route_httpPost++1\n");
	if(curl)
	{
	/* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. 
     */ 
     
    curl_easy_setopt(curl, CURLOPT_URL, url);
	log_debug("route_httpPost++2\n");
	//route_setShareHandle(curl);

//	curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L ); //����Ļ��ӡ�������ӹ��̺ͷ���http����
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10 );//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	curl_easy_setopt(curl, 	CURLOPT_AUTOREFERER, 1); // ����3��Ϊ�ض�������
//	curl_easy_setopt(curl, 	CURLOPT_FOLLOWLOCATION, 1); //���ص�ͷ������Location(һ��ֱ�������urlû�ҵ�)�����������Location��Ӧ������ 
	curl_easy_setopt(curl, 	CURLOPT_MAXREDIRS, 1);//���Ҵ�������ֹ����̫��
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 3 );//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���

	//����http���͵���������ΪJSON
	plist = curl_slist_append(headers,"Content-Type:application/json;charset=UTF-8");
	log_debug("route_httpPost++3\n");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
	log_debug("route_httpPost++4\n");

    /* Now specify the POST data  ����json����*/ 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str);
 	log_debug("route_httpPost++5\n");
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
	log_debug("route_httpPost++6\n");
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 	log_debug("route_httpPost++7\n");
    /* always cleanup */ 
    curl_easy_cleanup(curl);
    log_debug("route_httpPost++8\n");
  }
  
  curl_global_cleanup();
  
  log_debug("route_httpPost++\n");

  return (res!=CURLE_OK)?FAILE:SUCCE;
}

