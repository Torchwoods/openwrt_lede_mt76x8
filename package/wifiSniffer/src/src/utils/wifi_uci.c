#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "log.h"
#include "types.h"
#include "wifi_uci.h"
#include "uci.h"

static enum {	
	CLI_FLAG_MERGE =    (1 << 0),	
	CLI_FLAG_QUIET =    (1 << 1),	
	CLI_FLAG_NOCOMMIT = (1 << 2),	
	CLI_FLAG_BATCH =    (1 << 3),	
	CLI_FLAG_SHOW_EXT = (1 << 4),
}flags;

enum {	
	/* section cmds */	
	CMD_GET,	
	CMD_SET,	
	CMD_ADD_LIST,	
	CMD_DEL_LIST,	
	CMD_DEL,	
	CMD_RENAME,	
	CMD_REVERT,
	CMD_REORDER,
	/* package cmds */	
	CMD_SHOW,	
	CMD_CHANGES,
	CMD_EXPORT,	
	CMD_COMMIT,	
	/* other cmds */
	CMD_ADD,	
	CMD_IMPORT,	
	CMD_HELP,
};


static struct uci_context *ctx;

static void cli_perror(void)
{
	if (flags & CLI_FLAG_QUIET)
		return;	
	uci_perror(ctx, "fszigbeeGw");
}

static void uci_show_value(struct uci_option *o)
{
	struct uci_element *e;
	bool sep = false;

	switch(o->type) {
	case UCI_TYPE_STRING:
		printf("%s\n", o->v.string);
		break;
	case UCI_TYPE_LIST:
		uci_foreach_element(&o->v.list, e) {
			printf("%s%s", (sep ? " " : ""), e->name);
			sep = true;
		}
		printf("\n");
		break;
	default:
		printf("<unknown>\n");
		break;
	}
}

static int uci_do_section_cmd(int cmd, char *cmdline)
{
	struct uci_element *e;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	int dummy;

	if (uci_lookup_ptr(ctx, &ptr, cmdline, true) != UCI_OK) {
		cli_perror();
		return -1;
	}

	if (ptr.value && (cmd != CMD_SET) && (cmd != CMD_DEL) &&
	    (cmd != CMD_ADD_LIST) && (cmd != CMD_DEL_LIST) &&
	    (cmd != CMD_RENAME) && (cmd != CMD_REORDER))
		return -1;

	e = ptr.last;
	switch(cmd) {
	case CMD_GET:
		if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
			ctx->err = UCI_ERR_NOTFOUND;
			cli_perror();
			return -1;
		}
		switch(e->type) {
		case UCI_TYPE_SECTION:
			printf("%s\n", ptr.s->type);
			break;
		case UCI_TYPE_OPTION:
			uci_show_value(ptr.o);
			break;
		default:
			break;
		}
		/* throw the value to stdout */
		break;
	case CMD_RENAME:
		ret = uci_rename(ctx, &ptr);
		break;
	case CMD_REVERT:
		ret = uci_revert(ctx, &ptr);
		break;
	case CMD_SET:
		ret = uci_set(ctx, &ptr);
		break;
	case CMD_ADD_LIST:
		ret = uci_add_list(ctx, &ptr);
		break;
	case CMD_DEL_LIST:
		ret = uci_del_list(ctx, &ptr);
		break;
	case CMD_REORDER:
		if (!ptr.s || !ptr.value) {
			ctx->err = UCI_ERR_NOTFOUND;
			cli_perror();
			return -1;
		}
		ret = uci_reorder_section(ctx, ptr.s, strtoul(ptr.value, NULL, 10));
		break;
	case CMD_DEL:
		if (ptr.value && !sscanf(ptr.value, "%d", &dummy))
			return -1;
		ret = uci_delete(ctx, &ptr);
		break;
	}

	/* no save necessary for get */
	if ((cmd == CMD_GET) || (cmd == CMD_REVERT))
		return 0;

	/* save changes, but don't commit them yet */
	if (ret == UCI_OK)
		ret = uci_save(ctx, ptr.p);

	if (ret != UCI_OK) {
		cli_perror();
		return -1;
	}

	return 0;
}



static int wifi_uci_cmdisExist(char *cmds)
{
	struct uci_element *e;
	struct uci_ptr ptr;
	int ret = UCI_OK;

	if (uci_lookup_ptr(ctx, &ptr, cmds, true) != UCI_OK) {
		cli_perror();
		return 1;
	}

	//e = ptr.last;
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		ctx->err = UCI_ERR_NOTFOUND;
		cli_perror();
		return 1;
	}
	
	return 0;
}

//判断config是否存在,不存在创建文件
static int wifi_uci_package( char*config)
{

	char cmd[512]="\0";
	struct stat st;
	memset(&st,0,sizeof(st));
	snprintf(cmd,512,"/etc/config/%s",config);
	
	if(stat(cmd,&st))
	{
		snprintf(cmd,512,"touch /etc/config/%s\0",config);
		system(cmd);
	}	

	return 0;
}

//判断section是否存在，不存在创建section
static int wifi_uci_section( char*config, char*section)
{	
	char cmd[512]="\0";
	int ret = UCI_OK;
	
	snprintf(cmd,512,"%s.%s",config,section);

	if(wifi_uci_cmdisExist(cmd))
	{
		snprintf(cmd,512,"%s.%s=%s",config,section,config);
		ret = uci_do_section_cmd(CMD_SET,cmd);
	}
	

	return ret;
}

static int wifi_uci_option( char*config, char*section,char*option,char*value)
{
	
	char cmd[512]="\0";
	int ret = UCI_OK;
	snprintf(cmd,512,"%s.%s.%s=%s",config,section,option,value);
	printf("Option:%s\n",cmd);
	ret = uci_do_section_cmd(CMD_SET,cmd);

	return ret;
}

static int wifi_uci_commit(char *config)
{
	struct uci_ptr ptr;
	int ret = 0;

	ASSERT(config!=NULL);

	if (uci_lookup_ptr(ctx, &ptr, config, true) != UCI_OK) {
		cli_perror();
		return 1;
	}

//	if (flags & CLI_FLAG_NOCOMMIT)
//		return 0;
//		
	if (uci_commit(ctx, &ptr.p, false) != UCI_OK) {
		cli_perror();
		ret = 1;
	}
	
	if (ptr.p)
		uci_unload(ctx, ptr.p);

	return ret;
}

int wifi_uci_set( char*config, char*section,char*option,char*value)
{

	ASSERT(config!=NULL && section!=NULL && option!=NULL);

	int ret = -1;
	
	ctx = uci_alloc_context();
	
	if(!ctx)
	{
		log_err("Out of memory\n");
		return -1;
	}
	
	wifi_uci_package(config);
	
	do{
		ret = wifi_uci_section(config,section);
		if(ret != UCI_OK)
		{
			log_err("wifi_uci_option error\n");
			break;
		}
		
		ret = wifi_uci_option(config,section,option,value);
		if(ret != UCI_OK)
		{
			log_err("wifi_uci_option error\n");
			break;
		}

		ret = wifi_uci_commit(config);
		if(ret != UCI_OK)
		{
			log_err("wifi_uci_commit error\n");
			break;
		}
		
	}while(0);
	
	uci_free_context(ctx);
	return ret;
}

char* wifi_uci_get(char*config, char*section,char*option)
{
	struct uci_ptr ptr;
	int ret = UCI_OK;
	char cmds[512]="\0";
	char *value = NULL;
	ASSERT(config!=NULL && section!=NULL && option!=NULL);
	
	ctx = uci_alloc_context();

	if(!ctx)
	{
		log_err("Out of memory\n");
		return NULL;
	}

	snprintf(cmds,512,"%s.%s.%s",config,section,option);
	if (uci_lookup_ptr(ctx, &ptr, cmds, true) != UCI_OK) {
		cli_perror();
		return NULL;
	}

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) 
	{
		ctx->err = UCI_ERR_NOTFOUND;
		cli_perror();
		return NULL;
	}

	value = strdup(ptr.o->v.string);

	uci_free_context(ctx);
	
	return value;
}

int wifi_uci_del(char*config, char*section,char*option)
{
	struct uci_ptr ptr;
	int ret = UCI_OK;
	char cmds[512]="\0";

	ASSERT(config!=NULL);

	ctx = uci_alloc_context();

	if(!ctx)
	{
		log_err("Out of memory\n");
		return -1;
	}
	
	snprintf(cmds,512,"%s",config);

	if(section != NULL)
	{
		sprintf(cmds+strlen(cmds),".%s",section);
		if(option!=NULL)
		{
			sprintf(cmds+strlen(cmds),".%s",option);
		}
	}
	
	do{
		ret = uci_do_section_cmd(CMD_DEL,cmds);
		if(ret != UCI_OK)
		{
			log_err("uci_do_section_cmd\n");
			break;
		}

		ret = wifi_uci_commit(config);
		if(ret != UCI_OK)
		{
			log_err("wifi_uci_commit\n");
			break;
		}
	}while(0);
	
	uci_free_context(ctx);

	return ret;
}

