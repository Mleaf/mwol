/**
 * mwol - MQTT Wake On Lan
 * Copyright (C) 2019 Mleaf <mleaf90@gmail.com,350983773@qq.com>
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
#include <sys/types.h>
#include <ifaddrs.h> /*getifaddrs, freeifaddrs - get interface addresses*/
#include <stdio.h>

#include "common.h"
#include "config.h"
#include "cJSON.h"

bool DEBUG_LOG = false;

char *safe_strdup(const char *s) {
	char * retval = NULL;
	if (!s) {
		MSG_DEBUG("ERROR: safe_strdup called with NULL which would have crashed strdup. Bailing out");
		exit(1);
	}
	retval = strdup(s);
	if (!retval) {
		MSG_DEBUG("ERROR: Failed to duplicate a string: %s.  Bailing out", strerror(errno));
		exit(1);
	}
	return (retval);
}

void *safe_malloc(size_t size)
{
    void *retval = NULL;
    retval = malloc(size);
    if (!retval) {
        MSG_DEBUG("ERROR: Failed to malloc %d bytes of memory: %s.  Bailing out", size, strerror(errno));
        exit(1);
    }
    memset(retval, 0, size);
    return (retval);
}

int
safe_vasprintf(char **strp, const char *fmt, va_list ap)
{
    int retval;

    retval = vasprintf(strp, fmt, ap);

    if (retval == -1) {
       MSG_DEBUG("Failed to vasprintf: %s.  Bailing out", strerror(errno));
        exit(1);
    }
    return (retval);
}

int
safe_asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    int retval;

    va_start(ap, fmt);
    retval = safe_vasprintf(strp, fmt, ap);
    va_end(ap);

    return (retval);
}

void copy_mac2str(unsigned char *mac, char *str)
{
	if(!mac || !str)
		return ;
	sprintf(str,"%02x:%02x:%02x:%02x:%02x:%02x",
			mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	return;
}

bool get_dev_macaddr(const char* devName, unsigned char *mac)
{
	struct ifreq ifr;
	if(!mac || !devName)
		return false;          
	int s = socket(AF_INET, SOCK_DGRAM, 0); 
	if (s < 0)
	{   
		MSG_DEBUG("Error: socket create err:%d\n", s);
		return false;
	}   

	strcpy(ifr.ifr_name, devName);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0)
	{   
		close(s);
		MSG_DEBUG("Error: ioctl err:%d\n", s);
		return false; 
	}   
	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
	close(s);
	return true;
}

void get_eth_mac(char *eth, char *mac)
{
	unsigned char mac_arry[8]={0};
	if(true == get_dev_macaddr(eth, mac_arry))
	{
		copy_mac2str(mac_arry, mac);
	}
	else
	{
		strcpy(mac,"01:02:03:04:05:06");
	}

	return;
}

char *read_file(const char * filename) {
    FILE *fp = fopen(filename, "r");
    size_t file_size;
    long pos;
    char *file_contents;
    if (!fp)
        return NULL;
    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    if (pos < 0) {
        fclose(fp);
        return NULL;
    }
    file_size = pos;
    rewind(fp);
    file_contents = (char*)safe_malloc(sizeof(char) * (file_size + 1));
    if (!file_contents) {
        fclose(fp);
        return NULL;
    }
    if (fread(file_contents, file_size, 1, fp) < 1) {
        if (ferror(fp)) {
            fclose(fp);
            free(file_contents);
            return NULL;
        }
    }
    fclose(fp);
    file_contents[file_size] = '\0';
    return file_contents;
}


int get_name_info(const char* ip, char *hostName)
{
    int ret = 0;
	char host[64] = {0};

    if(!ip)
    {
		MSG_DEBUG("invalid params\n");
		return -1;
    }

    struct addrinfo hints;
    struct addrinfo *res, *res_p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME | AI_NUMERICHOST;
    hints.ai_protocol = 0;

    ret = getaddrinfo(ip, NULL, &hints, &res);
    if(ret != 0)
    {
		MSG_DEBUG("getaddrinfo: %s\n", gai_strerror(ret));
		return -1;
    }

    for(res_p = res; res_p != NULL; res_p = res_p->ai_next)
    {

        ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NAMEREQD);
        if(ret != 0)
        {
			MSG_DEBUG("getnameinfo: %s\n", gai_strerror(ret));
        }
        else
        {
			MSG_DEBUG("hostname: %s\n", host);
			memcpy(hostName, host, strlen(host));
        }
    }

    freeaddrinfo(res);
    return ret;
}

int get_if_addrs(void)
{

	struct ifaddrs *ifaddr, *c;
	if (getifaddrs(&ifaddr) == -1) {
		return -1;
	}

	for (c = ifaddr; c; c = c->ifa_next) {
		if (c->ifa_addr && c->ifa_addr->sa_family == AF_PACKET) {
			MSG_DEBUG("name: %s\n", c->ifa_name);
		}
	}
	freeifaddrs(ifaddr);
	return 0;
}


static void usage(void)
{
	fprintf(stdout, "Copyright (C) 2019 Mleaf <mleaf90@gmail.com>\n");
    fprintf(stdout, "Usage: mwol [options]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "options:\n");
    fprintf(stdout, "  -d            Debug\n");
    fprintf(stdout, "  -f            Run in foreground\n");
    fprintf(stdout, "  -s            Log to syslog\n");
    fprintf(stdout, "  -h            Print usage\n");
    fprintf(stdout, "  -v            Print version information\n");
    fprintf(stdout, "\n");
}

void parse_commandline(int argc, char **argv) {
	int c;
	s_config *config = config_get();
	while (-1 != (c = getopt(argc, argv, "c:hvfds"))) {

		switch(c) {

			case 'h':
				usage();
				exit(1);
				break;
			case 'c':
				if (optarg) {
					free(config->configfile);
					config->configfile = safe_strdup(optarg);
					MSG_DEBUG("Use new configfile %s\n",config->configfile);
				}
				break;
			case 'v':
				fprintf(stdout, "version:%s\nbulid time:%s\n", BUILD_VERSION, BUILD_TIME);
				exit(1);
				break;
			case 'f':
				break;
			
			case 'd':
				DEBUG_LOG = true;
				break;
			case 's':
				break;

			default:
				usage();
				exit(1);
				break;

		}

	}

}

