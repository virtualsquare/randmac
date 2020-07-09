/*
 * randmac: generate random mac addresses
 * Copyright (C) 2020  Renzo Davoli, Virtualsquare
 *
 * randmac is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <getopt.h>
#include <strcase.h>

#define EUICSV "/var/lib/ieee-data/oui.csv"

static char *short_options = "lgumUeqxo:v:h";
static struct option long_options[] = {
	{"local", no_argument, 0, 'l'},
	{"global", no_argument, 0, 'g'},
	{"unicast", no_argument, 0, 'u'},
	{"multicast", no_argument, 0, 'm'},
	{"uppercase", no_argument, 0, 'U'},
	{"qemu", no_argument, 0, 'q'},
	{"xen", no_argument, 0, 'x'},
	{"eui64", no_argument, 0, 'e'},
	{"oui", required_argument, 0, 'o'},
	{"vendor", required_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}};

static char *printfmt[] = {
	"%02x:%02x:%02x:%02x:%02x:%02x\n",
	"%02X:%02X:%02X:%02X:%02X:%02X\n",
	"%02x:%02x:%02x:ff:fe:%02x:%02x:%02x\n",
	"%02X:%02X:%02X:FF:FE:%02X:%02X:%02X\n",
};

static void usage(char *progname) {
	fprintf(stderr,
			"Usage %s [options]\n\n"
			"Options:\n"
			"   -l, --local        local administered\n"
			"   -g, --global       global unique\n"
			"   -u, --unicast      unicast\n"
			"   -m, --multicast    multicast address\n"
			"   -U, --uppercase    uppercase hex\n"
			"   -e, --eui64        generate eui64\n"
			"   -o <oui_addr>\n"
			"   --oui <oui_addr>   set oui addr\n"
			"   -v <vendor>\n"
			"   --vendor <vendor>  set oui from vendor\n"
			"   -q, --qemu         set qemu oui 52:54:00\n"
			"   -x, --xen          set xen oui 00:16:3e\n"
			"   -h, --delp         Print Help (this message) and exit\n",
			progname);
	exit(1);
}

static unsigned int read_oui(char *s) {
	if (strchr(s, ':')) {
		unsigned int byte[3];
		if (sscanf(s, "%x:%x:%x", byte, byte+1, byte+2) != 3) {
			fprintf(stderr, "Invalid oui specification\n");
			exit(2);
		}
		return ((byte[0] & 0xff) << 16) + ((byte[1] & 0xff) << 8) + (byte[2] & 0xff);
	} else {
		switch (strcase(s)) {
			case STRCASE(q,e,m,u): return 0x525400;
			case STRCASE(x,e,n):   return 0x00163e;
			default:
														 return strtol(s, NULL, 16);
		}
	}
}

static unsigned int vendor_oui(char *s) {
	FILE *stream = fopen(EUICSV, "r");
	char *line = NULL;
	size_t len = 0;
	if (stream == NULL) {
		fprintf(stderr, "eui csv file not found\n");
		exit(2);
	}
	while (getline(&line, &len, stream) >= 0) {
		if (strncmp(line, "MA-L,", 5) == 0) {
			char *tail;
			unsigned int eui = strtol(line + 5, &tail, 16);
			if (*tail == ',') {
				tail++;
				if (*tail == '"') tail++;
				if (strncmp(s, tail, strlen(s)) == 0) {
					free(line);
					fclose(stream);
					return eui;
				}
			}
		}
	}
	fprintf(stderr, "Invalid vendor oui\n");
	exit(2);
}

int main(int argc, char *argv[]) {
	char *progname = basename(argv[0]);
	unsigned char mac[6];
	unsigned int oui;
	unsigned int nic;
	char *oui_string = NULL;
	char *vendor = NULL;
	unsigned int local = 0;
	unsigned int global = 0;
	unsigned int unicast = 0;
	unsigned int multicast = 0;
	unsigned int uppercase = 0;
	unsigned int eui64 = 0;
	while (1) {
		char c;
		int option_index = 0;
		c = getopt_long(argc, argv, short_options,
				long_options,  &option_index);
		if (c == -1) break;
		switch (c) {
			case 'l': local = 1; break;
			case 'g': global = 1; break;
			case 'u': unicast = 1; break;
			case 'm': multicast = 1; break;
			case 'U': uppercase = 1; break;
			case 'e': eui64 = 1; break;
			case 'q': oui_string = "qemu"; break;
			case 'x': oui_string = "xen"; break;
			case 'o': oui_string = optarg; break;
			case 'v': vendor = optarg; break;
			default: usage(progname);
		}
	}
	if (optind < argc)
		usage(progname);

	srand(time(NULL) + getpid());
	if (vendor == NULL) {
		if (oui_string == NULL)
			oui = (rand() & 0xfcffff) | 0x020000;
		else
			oui = read_oui(oui_string);
	} else
		oui = vendor_oui(vendor);
	nic = rand() & 0xffffff;
	if (global) oui &= ~0x020000;
	if (local) oui |= 0x020000;
	if (unicast) oui &= ~0x010000;
	if (multicast) oui |= 0x010000;
	if (eui64) oui ^= 0x020000;

	mac[0] = oui >> 16;
	mac[1] = oui >> 8;
	mac[2] = oui;
	mac[3] = nic >> 16;
	mac[4] = nic >> 8;
	mac[5] = nic;

	printf(printfmt[(eui64 << 1) + uppercase],
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return 0;
}


