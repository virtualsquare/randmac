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

#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <strcase.h>

#define EUICSV "/var/lib/ieee-data/oui.csv"
#define RANDOM_DEVICE "/dev/urandom"

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

static void usage(char *progname, int exitcode) {
	if (exitcode != 0)
		fprintf(stderr, "\n");
	fprintf(exitcode ? stderr : stdout,
			"Usage\n"
			"   %s [options]\n\n"
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
			"   -h, --help         print help (this message) and exit\n\n"
			"For more details see randmac(1).\n",
			progname);
	exit(exitcode);
}

static void panic(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(2);
}

static unsigned int read_oui(char *s) {
	if (strchr(s, ':')) {
		unsigned int byte[3];
		if (sscanf(s, "%x:%x:%x", byte, byte+1, byte+2) != 3)
			panic("Invalid OUI specification (expected OUI in the form xx:xx:xx)\n");
		return ((byte[0] & 0xff) << 16) + ((byte[1] & 0xff) << 8) + (byte[2] & 0xff);
	} else {
		switch (strcase(s)) {
			case STRCASE(q,e,m,u): return 0x525400;
			case STRCASE(x,e,n):   return 0x00163e;
			default:
				if (strlen(s) < 1 || strlen(s) > 6)
					panic("Invalid OUI specification (expected between 1 and 6 hex digits, got %zu)\n", strlen(s));
				char *endptr;
				unsigned int oui = strtoul(s, &endptr, 16);
				if (*endptr != '\0')
					panic("Invalid OUI specification (aborted at %c due to invalid character)\n", *endptr);
				return oui;
		}
	}
}

static unsigned int vendor_oui(char *s) {
	FILE *stream = fopen(EUICSV, "r");
	char *line = NULL;
	size_t len, count, index;
	len = count = index = 0;
	if (stream == NULL)
		panic("Failed to open %s. File possibly missing, try installing the ieee-data package.\n", EUICSV);
	while (getline(&line, &len, stream) >= 0) {
		if (strncmp(line, "MA-L,", 5) == 0) {
			char *tail = line + strlen("MA-L,000000");
			if (*tail == ',') {
				tail++;
				if (*tail == '"') tail++;
				if (strncmp(s, tail, strlen(s)) == 0) {
					count++;
				}
			}
		}
	}
	if (count == 0)
		goto notfound;

	rewind(stream);
	index = rand() % count;
	while (getline(&line, &len, stream) >= 0) {
		if (strncmp(line, "MA-L,", 5) == 0) {
			char *tail;
			unsigned int eui = strtol(line + 5, &tail, 16);
			if (*tail == ',') {
				tail++;
				if (*tail == '"') tail++;
				if (strncmp(s, tail, strlen(s)) == 0) {
					if (index == 0) {
						free(line);
						fclose(stream);
						return eui;
					}
					index--;
				}
			}
		}
	}

	notfound:
		if (line != NULL)
			free(line);
		fclose(stream);
		panic("Invalid vendor OUI\n");
		return 0; // make compiler happy
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
		int c;
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
			case 'h': usage(progname, 0); break;
			default: usage(progname, 1);
		}
	}
	if (optind < argc) {
		fprintf(stderr, "%s: extra operand -- '%s'\n", progname, argv[optind]);
		usage(progname, 1);
	}

	unsigned int seed;
	int urandom = open(RANDOM_DEVICE, O_RDONLY);
	if (urandom < 0) {
		panic("%s: failed to open %s\n", progname, RANDOM_DEVICE);
	}
	if (read(urandom, &seed, sizeof(seed)) != sizeof(seed)) {
		panic("%s: failed to read from %s\n", progname, RANDOM_DEVICE);
	}
	close(urandom);
	srand(seed);

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


