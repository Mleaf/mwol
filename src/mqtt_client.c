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

/***********************************************************
@protocol
网关发布主题：

网关上行数据发布接口：/etherWakeData/网关mac

网关订阅主题：

平台下行数据发布接口：/etherWakeCtrl/网关mac

网关连接broker：

etherWake|网关mac

网关数据格式：

{
    "header": {
        "action": "xxx"
    },
    "payload": {
        "deviceId": "72:42:2D:88:16:BF",
        "status": "success",
        "data": [{}]
    }
}

平台数据格式：

{
    "header": {
        "action": "xxx"
    },
    "payload": {
        "deviceId": "72:42:2D:88:16:BF",
        "status": "success",
        "data": [{}]
    }
}

***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <stdbool.h>        /* bool type */
#include <sys/stat.h>
#include <stdint.h>         /* C99 types */
#include <stdbool.h>        /* bool type */
#include <stdio.h>          /* printf, fprintf, snprintf, fopen, fputs */
#include <errno.h>          /* error messages */
#include <pthread.h>
#include <stdarg.h>         /* va_list */

#include "mqtt_client.h"
#include "config.h"
#include "common.h"
#include "json.h"

struct mosquitto *mosq = NULL;

static void
send_mqtt_response(struct mosquitto *mosq, const char *msg)
{	
	char *format_msg = NULL;
	cJSON *item =NULL;
	
	s_config *config = config_get();
	char *topic = NULL;
	char *res_data = NULL;
	safe_asprintf(&topic, GETWAY_MQTT_TOPIC_PUB, config->mqtt_server->id);
	safe_asprintf(&res_data, "%s", msg==NULL?"null":msg);
	mosquitto_publish(mosq, NULL, topic, strlen(res_data), res_data, 0, false);

	item = cJSON_Parse(msg);
	format_msg = cJSON_Print(item);
	
	MSG_DEBUG("send mqtt response: topic is %s msg is %s", topic, format_msg);
	
	free(topic);
	free(res_data);
	if(NULL != format_msg){
		free(format_msg);
	}

	if(NULL != item){
		cJSON_Delete(item);
	}
}


void mqtt_arp_list(void)
{
	char *json_msg = NULL;

	json_arp_list(&json_msg);
	if(json_msg!=NULL){
		
		MSG_DEBUG("%s\n",json_msg);

		send_mqtt_response(mosq, json_msg);

		free(json_msg);
	}
}

void mqtt_wol_response(char *mac)
{
	char *json_msg = NULL;

	json_wol_response(&json_msg, mac);
	if(json_msg!=NULL){
		
		MSG_DEBUG("%s\n",json_msg);

		send_mqtt_response(mosq, json_msg);

		free(json_msg);
	}
}

static void
process_mqtt_reqeust(struct mosquitto *mosq, void *data, s_config *config)
{
	process_json_object((char *)data);
}

static void 
mqtt_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	s_config *config = obj;

	if (message->payloadlen) {
		MSG_DEBUG("topic is: %s, data is: %s\n", message->topic, (char *)message->payload);
		process_mqtt_reqeust(mosq, message->payload, config);
	}
}

static void
mqtt_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	s_config 		*config = obj;
	char *default_topic = NULL;
	safe_asprintf(&default_topic, GETWAY_MQTT_TOPIC_SUB, config->mqtt_server->id);//指定消息所发布的主题
	MSG_DEBUG("default_topic:%s\n", default_topic);
	if(!result){
		/* Subscribe to broker information topics on successful connect. */
		MSG_DEBUG("Connect success: %s\n", default_topic); 
		mosquitto_subscribe(mosq, NULL, default_topic, 0); // qos is 0

	}else{
		MSG_DEBUG("MQTT Connect failed\n");
	}

	free(default_topic);
}

void init_mqtt(void)
{
	s_config *config = NULL;
	int major = 0, minor = 0, revision = 0;

	char *client_id = NULL;
	int protocol_version;
	bool ssl = false;
	char *host = NULL;
	int  port = 1883;
	char *cafile = NULL;
	char *certfile = NULL;
	char *keyfile = NULL;
	bool encryption = false;
	char *username = NULL;
	char *password = NULL;
	
	int  keepalive = 90;
	int  retval = 0;

	config = config_get();
	if (config == NULL){
		MSG_DEBUG("Error: CONFIG.\n");
		exit(0);
	}
	host = config->mqtt_server->hostname;
	port = config->mqtt_server->port;
	encryption = config->mqtt_server->encryption;
	ssl = config->mqtt_server->ssl;

	MSG_DEBUG("mqtt hostname: %s\n",host);
	MSG_DEBUG("mqtt port: %d\n", port);

	protocol_version = MQTT_PROTOCOL_V311;

	mosquitto_lib_version(&major, &minor, &revision);

	MSG_DEBUG("Mosquitto library version : %d.%d.%d\n", major, minor, revision); 

	mosquitto_lib_init();

	MSG_DEBUG("gw_id: %s\n", config->gw_id); 
	
	safe_asprintf(&client_id, GETWAY_MQTT_CLIENT_ID, config->mqtt_server->id);
	
	MSG_DEBUG("client_id: %s\n", client_id); 

	mosq = mosquitto_new(client_id, true, config);
	if( mosq == NULL ) {
		switch(errno){
			case ENOMEM:
				MSG_DEBUG("Error: Out of memory.\n");
				break;
			case EINVAL:
				MSG_DEBUG("Error: Invalid id and/or clean_session.\n");
				break;
		}
		mosquitto_lib_cleanup();
		return;
	}

	if(encryption == true){
		username = config->mqtt_server->username;
		password = config->mqtt_server->password;
		if (username == NULL && password == NULL){
			MSG_DEBUG("Error: username or password.\n");
			exit(0);
		}

		MSG_DEBUG("mqtt username: %s\n",username); 
		MSG_DEBUG("mqtt password: %s\n", password);
		mosquitto_username_pw_set(mosq, username, password);
	}

	mosquitto_opts_set(mosq, MOSQ_OPT_PROTOCOL_VERSION, &protocol_version);

	if(ssl == true){
		cafile = config->mqtt_server->cafile;
		certfile = config->mqtt_server->crtfile;
		keyfile = config->mqtt_server->keyfile;
		if (cafile == NULL || certfile == NULL || keyfile == NULL){
			MSG_DEBUG("Error: ssl file.\n");
			exit(0);
		}
		MSG_DEBUG("mqtt cafile: %s\n",cafile); 
		MSG_DEBUG("mqtt certfile: %s\n", certfile);
		MSG_DEBUG("mqtt keyfile: %s\n", keyfile);

		/*
		int mosquitto_tls_set(	mosq,	 
			cafile,  
			capath,  
			certfile,	 
			keyfile,	 
			(*pw_callback)(char *buf, int size, int rwflag, void *userdata));	 
		*/
		if (mosquitto_tls_set(mosq, cafile, "/usr/share/mwol/ssl/", certfile, keyfile, NULL)) {
			MSG_DEBUG("Error : Problem setting TLS option");
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			return;
		}

		if (mosquitto_tls_insecure_set(mosq, true)) {
			MSG_DEBUG("Error : Problem setting TLS insecure option");
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			return;
		}
	}

	mosquitto_connect_callback_set(mosq, mqtt_connect_callback);

	mosquitto_message_callback_set(mosq, mqtt_message_callback);

	switch( retval = mosquitto_connect(mosq, host, port, keepalive) ) {
		case MOSQ_ERR_INVAL:
			MSG_DEBUG("Error : %s\n", mosquitto_strerror(retval)); 
			break;
		case MOSQ_ERR_ERRNO:
			MSG_DEBUG("Error : %s\n", strerror(errno));
			break;

		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
		return;
	}

	retval = mosquitto_loop_forever(mosq, -1, 1);
	
	free(client_id);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

}


