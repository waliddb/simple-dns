#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>

#include "dns_protocol.h"
#include "server.h"

struct dns_server dns_server;

/*
 * initialize dns_server
 */
void dns_init (void)
{
	printf ("Initializing DNS server.\n");

	dns_server.config.config_file = NULL;
	dns_server.config.fg = 0;

	dns_server.config.port = 53;
	dns_server.config.host = "0.0.0.0";

	dns_server.listenfd = 0;
}

/*
 * open listen sockets and set flags
 */
void open_sockets (void)
{
	struct sockaddr_in sin;
	int sd;

	printf ("Opening sockets.\n");

	/* TODO - O_CLOEXEC and O_NONBLOCK */
	assert ((sd = socket (AF_INET, SOCK_DGRAM, 0)) > 0);

	memset ((char *) &sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons (dns_server.config.port);
	sin.sin_addr.s_addr = inet_addr (dns_server.config.host);
	assert (bind (sd, (struct sockaddr *) &sin, sizeof (sin)) == 0);

	dns_server.listenfd = sd; 
}

/*
 * after parsing configuration, start the dns_server
 */
void dns_start (void)
{
	printf ("Starting DNS server.\n");

	openlog ("simple-dns", LOG_PID, LOG_LOCAL0);

	open_sockets ();
}

/*
 * loop accepting new requests
 */
void dns_loop (void)
{
	int client;
	char buf[PACKET_SIZE+4];
	struct sockaddr_in from;
	socklen_t from_len;
	struct dns_packet *req;

	from_len = sizeof (from);

	printf ("Accepting connections...\n");
	for (;;)
	{
		client = recvfrom (dns_server.listenfd, buf, PACKET_SIZE + 4, 0, (struct sockaddr *) &from, &from_len);
		printf ("client: %d %s\n", client, strerror (errno));
		req = (struct dns_packet *) buf;
		dns_print_packet (req);
//		dns_parse_request (&req, buf, PACKET_SIZE + 4);
	}
}
