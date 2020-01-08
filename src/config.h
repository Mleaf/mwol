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

#ifndef _CONFIG_H
#define _CONFIG_H
#include <stdbool.h>        /* bool type */

#define DEFAULT_MQTT_SERVER		"www.mleaf.org"
#define DEFAULT_MQTT_PORT       1883
#define DEFAULT_ENCRYPTION      false
#define DEFAULT_SSL      		false
#define DEFAULT_MQTT_USERNAME       "mleaf"
#define DEFAULT_MQTT_PASSWORD       "www.mleaf.org"
#define DEFAULT_PRESET_MAC       "00:0c:29:eb:5f:12"	
#define	DEFAULT_CA_FILE		"/usr/share/etherwake/ca.crt"
#define	DEFAULT_CRT_FILE	"/usr/share/etherwake/client.crt"
#define	DEFAULT_KEY_FILE	"/usr/share/etherwake/client.key"
			
#define DEFAULT_CONFIGFILE  "/tmp/wol_conf.json"

/*
{
	"hostname": "www.mleaf.org",
	"port": 1883,
	"ssl": false,
	"cafile": "/usr/share/etherwake/ca.crt",
	"crtfile": "/usr/share/etherwake/client.crt",
	"keyfile": "/usr/share/etherwake/client.key",
	"encryption": true,
	"username": "mleaf",
	"password": "www.mleaf.org"
}
*/
typedef struct _mqtt_server_t {
	char 	*hostname;
	short	port;
	bool 	ssl;
	char 	*cafile;
	char 	*crtfile;
	char 	*keyfile;
	bool 	encryption;
	char 	*username;
	char 	*password;
	char 	*id;
	
}t_mqtt_server;

typedef struct{
	char *configfile;
	char *gw_id;
	char *presetmac;
	t_mqtt_server	*mqtt_server;
}s_config;

extern int config_read_from_json(char *cfg_path);
extern void config_init(void);
extern s_config *config_get(void);

#endif

