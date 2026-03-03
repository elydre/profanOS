#ifndef NETDB_H
#define NETDB_H

struct hostent {
	char *h_name;
	char **h_aliases;
	int h_addrtype;
	int h_length;
	char **h_addr_list;
};

extern int h_errno;

struct hostent *gethostbyname(const char *name);

#define HOST_NOT_FOUND 0xFF
#define NO_DATA 0xFE
#define NO_RECOVERY 0xFD
#define TRY_AGAIN 0xFC

#endif
