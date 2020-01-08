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

#ifndef _JSON_H
#define _JSON_H

#include "cJSON.h"


#ifndef JSON_ERROR
#define JSON_ERROR 0
#endif

#ifndef JSON_SUCCESS
#define JSON_SUCCESS 1
#endif

extern void process_arp_list_op(cJSON *root);
extern void process_wol_op(cJSON *root);
extern void json_arp_list(char **msg);
extern void json_wol_response(char **msg, char *mac);
extern int process_json_object(char *msg);

#endif
