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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>         /* C99 types */
#include <stdbool.h>        /* bool type */
#include <stdio.h>          /* printf, fprintf, snprintf, fopen, fputs */
			
#include <string.h>         /* memset */
#include <signal.h>         /* sigaction */
#include <time.h>           /* time, clock_gettime, strftime, gmtime */
#include <sys/time.h>       /* timeval */
#include <unistd.h>         /* getopt, access */
#include <stdlib.h>         /* atoi, exit */
#include <errno.h>          /* error messages */
#include <math.h>           /* modf */

#include <assert.h>
#include <net/if.h>  		/* ifreq */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <stddef.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <syslog.h>
#include <stdarg.h>             /* For va_list */
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUILD_VERSION	"V1.0.0"
#define BUILD_TIME		(__DATE__" "__TIME__)

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef IFNAME_BR
#define IFNAME_BR			"br-lan"
#endif

extern bool DEBUG_LOG;
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define MSG(args...) printf(args) /* message that is destined to the user */
#define MSG_DEBUG(fmt, ...)                                                                         \
            do  {                                                                                         \
                if (DEBUG_LOG)                                                                                 \
                    fprintf(stdout, "%s:%d: " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__); \
					fputc('\n', stdout);\
            } while (0)

extern char * safe_strdup(const char *s);
extern void *safe_malloc(size_t size);
extern void get_eth_mac(char *eth, char *mac);
extern int safe_asprintf(char **strp, const char *fmt, ...);
extern char *read_file(const char * filename);
extern int wake_on_lan(char c, int ifdebug, char *wakeMac);
extern int get_name_info(const char* ip, char *hostName);
extern int get_if_addrs(void);
extern void parse_commandline(int argc, char **argv);

#endif

