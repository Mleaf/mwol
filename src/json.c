/**
 * mwol - MQTT Wake On Lan
 * Copyright (C) 2019 Mleaf <mleaf90@gmail.com, 350983773@qq.com>
 * 微信公众号 《WiFi物联网》
 *
 * mwol is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mwol is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config.h"
#include "common.h"
#include "json.h"
#include "mqtt_client.h"

static struct wol_json_op {
	char	*action;
	void	(*process_wol_json_op)(cJSON *);
} wol_op[] = {

	{"arp", process_arp_list_op},
	{"wol", process_wol_op},

	{NULL, NULL}

};

/**********************************************************************/

/*
{
	"header": {
		"action": "arp"
	},
	"payload": {}
}
*/
void process_arp_list_op(cJSON *root)
{
	mqtt_arp_list();

}

/*
{
	"header": {
		"action": "wol"
	},
	"payload": {
		"deviceId": "06:f0:21:34:5f:1d",
		"mac": "00:25:22:42:b6:93",
		"broadcast": true
	}
}
*/

void process_wol_op(cJSON *root)
{
	cJSON *payload = NULL;
	
	char *deviceId = NULL;
	char *mac = NULL;
	bool broadcast = false;

	payload = cJSON_GetObjectItem(root, "payload");
	deviceId = cJSON_GetObjectItem(payload, "deviceId")->valuestring;
	mac = cJSON_GetObjectItem(payload, "mac")->valuestring;
	broadcast = (bool)cJSON_GetObjectItem(payload, "broadcast")->valueint;
	MSG_DEBUG("deviceId:%s\n", deviceId);
	if(mac != NULL){
		MSG_DEBUG("wol mac:%s\n", mac);
		if(broadcast){
			wake_on_lan('b', true, mac);
		}else{
			wake_on_lan('D', true, mac);
		}
		mqtt_wol_response(mac);
	}

}

/**********************************************************************/

static void
json_parse_token(char *action, cJSON *root)
{
	int i = 0;

	for (; wol_op[i].action != NULL; i++) {
		if(strcmp(action, wol_op[i].action) == 0 && wol_op[i].process_wol_json_op){
			
			wol_op[i].process_wol_json_op(root);
			break;
		}
	}
}

static int validate_json_Object(cJSON *item)
{
	cJSON *found_header = NULL;
	cJSON *found_payload = NULL;

	found_header = cJSON_GetObjectItem(item,"header");
	if(NULL == found_header)
	{
		return JSON_ERROR;
	}

	found_payload = cJSON_GetObjectItem(item,"payload");
	if(NULL == found_payload)
	{
		return JSON_ERROR;
	}

	if( found_header->type == cJSON_Object && found_payload->type == cJSON_Object)  
	{	 
		return JSON_SUCCESS;
	}
	else{
		return JSON_ERROR;
	}

}

/*
{
    "header": {
        "action": "xxx"
    },
    "payload": {
        "deviceId": "72:42:2D:88:16:BF",
        "status": "success",
        "data": [{}]
}

*/
int process_json_object(char *msg)
{
	cJSON *item = NULL;
	cJSON *header;
	int ret = JSON_ERROR;
	char *action;

	MSG_DEBUG("Process json object string=%s\n", msg);

	item = cJSON_Parse(msg);

	if (!item)
	{
		MSG_DEBUG("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(item);
		return JSON_ERROR;
	}
	ret = validate_json_Object(item);
	if(JSON_SUCCESS != ret)
	{
		MSG_DEBUG("Error Json Object.\n");
		return JSON_ERROR;
	}
	else{

		header = cJSON_GetObjectItem(item, "header");
		action = cJSON_GetObjectItem(header, "action")->valuestring;

		json_parse_token(action, item);

	}

	cJSON_Delete(item);
	return JSON_SUCCESS;
}

/*
{
	"header": {
		"action": "arpList"
	},
	"payload": {
		"deviceId": "06:f0:21:34:5f:1d",
		"data": [{
				"mac": "00:03:7f:11:23:1f",
				"ip": "10.13.35.31",
				"hostName": "* *"
			},
			{
				"mac": "00:25:22:42:b6:93",
				"ip": "192.168.10.235",
				"hostName": "DESKTOP-VVNKNR8.lan"
			},
			{
				"mac": "00:03:7f:11:23:1f",
				"ip": "192.168.10.113",
				"hostName": "* *"
			}
		]
	}
}

*/
void json_arp_list(char **msg)
{
	FILE *fp;
	unsigned int arp_flags;
	char buffer[256], arp_mac[32], arp_if[32];
	char hostName[64] = {0};
	int ret = -1;

	char *out=NULL;
	cJSON *dir1=NULL;
	cJSON *dir2=NULL;
	cJSON *dataArry=NULL;
	cJSON *dir3=NULL;
#if defined (USE_IPV6)
		char s_addr1[INET6_ADDRSTRLEN];
#else
		char s_addr1[INET_ADDRSTRLEN];
#endif
	s_config *board_config = config_get();

	cJSON *root = cJSON_CreateObject();
	dataArry = cJSON_CreateArray();
	
	cJSON_AddItemToObject(root, "header", dir1=cJSON_CreateObject());

	cJSON_AddStringToObject(dir1, "action", "arp");

	cJSON_AddItemToObject(root, "payload", dir2=cJSON_CreateObject());
	cJSON_AddStringToObject(dir2, "deviceId", board_config->gw_id);

	dir3=cJSON_CreateObject();
	cJSON_AddStringToObject(dir3, "mac", board_config->presetmac);
	cJSON_AddStringToObject(dir3, "ip", "* *");
	cJSON_AddStringToObject(dir3, "hostName", "preset mac");
	cJSON_AddItemToArray(dataArry, dir3);
	
	fp = fopen("/proc/net/arp", "r");
	if (fp) {
		// skip first line
		fgets(buffer, sizeof(buffer), fp);
		
		while (fgets(buffer, sizeof(buffer), fp)) {
			arp_flags = 0;
			if (sscanf(buffer, "%s %*s 0x%x %31s %*s %31s", s_addr1, &arp_flags, arp_mac, arp_if) == 4) {
				if((arp_flags & 0x02)&&(strcmp(arp_if, IFNAME_BR) == 0) && (strcmp(arp_mac, "00:00:00:00:00:00"))){
					//printf("s_addr1:%s,arp_mac:%s\n",s_addr1,arp_mac);
					//printf("arp_flags:0x%02x\n",arp_flags);
					//printf("arp_if:%s\n",arp_if);
					dir3=cJSON_CreateObject();
					memset(hostName, 0, sizeof(hostName));
					ret = get_name_info(s_addr1, hostName);
					cJSON_AddStringToObject(dir3, "mac", arp_mac);
					cJSON_AddStringToObject(dir3, "ip", s_addr1);
					if(ret == 0){
						cJSON_AddStringToObject(dir3, "hostName", hostName);
					}else{
						cJSON_AddStringToObject(dir3, "hostName", "* *");
					}
					cJSON_AddItemToArray(dataArry, dir3);
				}
			}
		}
		
		fclose(fp);
	}
	cJSON_AddItemToObject(dir2, "data", dataArry);

	out = cJSON_PrintUnformatted(root);

	*msg = (char *)safe_malloc(strlen(out)+1);

	strncpy(*msg, out, strlen(out));

	//MSG_DEBUG("%s\n",out);
	cJSON_Delete(root);
	if(out)
	{
		free(out);
	}
}

/*
{
	"header": {
		"action": "wol"
	},
	"payload": {
		"mac": "00:03:7f:11:23:1f",
		"status": "success"
	}
}
*/
void json_wol_response(char **msg, char *mac)
{
	char *out;
	cJSON *dir1;
	cJSON *dir2;

	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "header", dir1=cJSON_CreateObject());

	cJSON_AddStringToObject(dir1, "action", "wol");

	cJSON_AddItemToObject(root, "payload", dir2=cJSON_CreateObject());
	cJSON_AddStringToObject(dir2, "mac", mac);
	cJSON_AddStringToObject(dir2, "status", "success");

	out = cJSON_PrintUnformatted(root);
	*msg = (char *)safe_malloc(strlen(out)+1);

	strncpy(*msg, out, strlen(out));

	cJSON_Delete(root);
	if(out)
	{
		free(out);
	}
}

