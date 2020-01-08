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
#include "cJSON.h"
#include "common.h"

static s_config CONFIG;

cJSON * json_parse_file(const char *filename) {
    char *file_contents = read_file(filename);
	cJSON *item = NULL;
    if (file_contents == NULL)
        return NULL;
	
	MSG_DEBUG("INFO: parse json file:\n%s\n", file_contents);

	item = cJSON_Parse(file_contents);
	free(file_contents);
	if (!item)
	{
		MSG_DEBUG("Error:[%s]", cJSON_GetErrorPtr());
		return NULL;
	}

    return item;
}

static int parse_wol_configuration(const char * conf_file) {
    cJSON  *root_val = NULL;
	cJSON  *val = NULL;
	t_mqtt_server *mqtt_server = CONFIG.mqtt_server;

    /* try to parse JSON */
    root_val = json_parse_file(conf_file);
    if (root_val == NULL) {
        MSG_DEBUG("ERROR: %s is not a valid JSON file\n", conf_file);
        exit(EXIT_FAILURE);
    }
	
    val = cJSON_GetObjectItem(root_val, "encryption"); /* fetch value (if possible) */
    if (val != NULL) {
        mqtt_server->encryption = (bool)val->valueint;
		MSG_DEBUG("encryption:[%s]\n", mqtt_server->encryption?"true":"false");
    } else {
        MSG_DEBUG("WARNING: Data type for encryption seems wrong, please check\n");
        mqtt_server->encryption = false;
    }
	
    val = cJSON_GetObjectItem(root_val, "ssl"); /* fetch value (if possible) */
    if (val != NULL) {
        mqtt_server->ssl = (bool)val->valueint;
		MSG_DEBUG("ssl:[%s]\n", mqtt_server->ssl?"true":"false");
    } else {
        MSG_DEBUG("WARNING: Data type for ssl seems wrong, please check\n");
        mqtt_server->ssl = false;
    }

    val = cJSON_GetObjectItem(root_val, "hostname"); /* fetch value (if possible) */
    if (val != NULL) {
		if(mqtt_server->hostname != NULL){
			free(mqtt_server->hostname);
		}
		mqtt_server->hostname = safe_strdup(val->valuestring);
		MSG_DEBUG("hostname:[%s]\n", mqtt_server->hostname);
    } else {
        MSG_DEBUG("WARNING: Data type for hostname seems wrong, use default hostname [%s]\n", mqtt_server->hostname);
    }

    val = cJSON_GetObjectItem(root_val, "port"); /* fetch value (if possible) */
    if (val != NULL) {
		mqtt_server->port = val->valueint;
		MSG_DEBUG("port:[%d]\n", mqtt_server->port);
    } else {
        MSG_DEBUG("WARNING: Data type for port seems wrong, use default port [%d]\n", mqtt_server->port);
    }
		
	val = cJSON_GetObjectItem(root_val, "presetmac"); /* fetch value (if possible) */
	if (val != NULL) {
		if(CONFIG.presetmac != NULL){
			free(CONFIG.presetmac);
		}
		CONFIG.presetmac = safe_strdup(val->valuestring);
		MSG_DEBUG("presetmac:[%s]\n", CONFIG.presetmac);
	} else {
		MSG_DEBUG("WARNING: Data type for presetmac seems wrong, use default presetmac [%s]\n", CONFIG.presetmac);
	}

	val = cJSON_GetObjectItem(root_val, "id"); /* fetch value (if possible) */
	if (val != NULL) {
		if(mqtt_server->id != NULL){
			free(mqtt_server->id);
		}
		mqtt_server->id = safe_strdup(val->valuestring);
		MSG_DEBUG("id:[%s]\n", mqtt_server->id);
	} else {
		MSG_DEBUG("WARNING: Data type for id seems wrong, use default id [%s]\n", mqtt_server->id);
	}

	if(mqtt_server->ssl == true){
	    val = cJSON_GetObjectItem(root_val, "cafile"); /* fetch value (if possible) */
	    if (val != NULL) {
			if(mqtt_server->cafile != NULL){
				free(mqtt_server->cafile);
			}
			mqtt_server->cafile = safe_strdup(val->valuestring);
			MSG_DEBUG("cafile:[%s]\n", mqtt_server->cafile);
	    } else {
	        MSG_DEBUG("WARNING: Data type for cafile seems wrong, use default cafile [%s]\n", mqtt_server->cafile);
	    }

	    val = cJSON_GetObjectItem(root_val, "crtfile"); /* fetch value (if possible) */
	    if (val != NULL) {
			if(mqtt_server->crtfile != NULL){
				free(mqtt_server->crtfile);
			}
			mqtt_server->crtfile = safe_strdup(val->valuestring);
			MSG_DEBUG("crtfile:[%s]\n", mqtt_server->crtfile);
	    } else {
	        MSG_DEBUG("WARNING: Data type for crtfile seems wrong, use default crtfile [%s]\n", mqtt_server->crtfile);
	    }
		
	    val = cJSON_GetObjectItem(root_val, "keyfile"); /* fetch value (if possible) */
	    if (val != NULL) {
			if(mqtt_server->keyfile != NULL){
				free(mqtt_server->keyfile);
			}
			mqtt_server->keyfile = safe_strdup(val->valuestring);
			MSG_DEBUG("keyfile:[%s]\n", mqtt_server->keyfile);
	    } else {
	        MSG_DEBUG("WARNING: Data type for keyfile seems wrong, use default keyfile [%s]\n", mqtt_server->keyfile);
	    }
	}
	if(mqtt_server->encryption == true){
	    val = cJSON_GetObjectItem(root_val, "username"); /* fetch value (if possible) */
	    if (val != NULL) {
			if(mqtt_server->username != NULL){
				free(mqtt_server->username);
			}
			mqtt_server->username = safe_strdup(val->valuestring);
			MSG_DEBUG("username:[%s]\n", mqtt_server->username);
	    } else {
	        MSG_DEBUG("WARNING: Data type for username seems wrong, use default username [%s]\n", mqtt_server->username);
	    }

	    val = cJSON_GetObjectItem(root_val, "password"); /* fetch value (if possible) */
	    if (val != NULL) {
			if(mqtt_server->password != NULL){
				free(mqtt_server->password);
			}
			mqtt_server->password = safe_strdup(val->valuestring);
			MSG_DEBUG("password:[%s]\n", mqtt_server->password);
	    } else {
	        MSG_DEBUG("WARNING: Data type for password seems wrong, use default password [%s]\n", mqtt_server->password);
	    }

	}
    /* free JSON parsing data*/
    cJSON_Delete(root_val);
    return 0;
}

int config_read_from_json(char *cfg_path)
{
	int ret;
	if (access(cfg_path, R_OK) == 0) {
		MSG_DEBUG("INFO: found wol configuration file %s, parsing it\n", cfg_path);
		ret = parse_wol_configuration(cfg_path);
		if (ret != 0) {
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

s_config *
config_get(void)
{
    return &CONFIG;
}

void config_init(void)
{
	char ap_mac[24]={0};

	memset(ap_mac, 0 , sizeof(ap_mac));
	get_eth_mac("eth0", ap_mac);

	CONFIG.configfile = safe_strdup(DEFAULT_CONFIGFILE);
	CONFIG.gw_id =  safe_strdup(ap_mac);
	CONFIG.presetmac	= safe_strdup(DEFAULT_PRESET_MAC);
	t_mqtt_server *mqtt_server = (t_mqtt_server *)safe_malloc(sizeof(t_mqtt_server));
	mqtt_server->hostname	= safe_strdup(DEFAULT_MQTT_SERVER);
	mqtt_server->port 		= DEFAULT_MQTT_PORT;
	mqtt_server->ssl        = false;
	mqtt_server->cafile  = safe_strdup(DEFAULT_CA_FILE);
	mqtt_server->crtfile	= safe_strdup(DEFAULT_CRT_FILE);
	mqtt_server->keyfile	= safe_strdup(DEFAULT_KEY_FILE);
	mqtt_server->encryption = false;
	mqtt_server->username	= safe_strdup(DEFAULT_MQTT_USERNAME);
	mqtt_server->password	= safe_strdup(DEFAULT_MQTT_PASSWORD);
	mqtt_server->id	= safe_strdup(ap_mac);

	CONFIG.mqtt_server	= mqtt_server;

}

