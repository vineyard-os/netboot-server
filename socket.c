#include <string.h>

#include "netboot.h"

ssize_t socket_send(int s, struct sockaddr_in6 *dest, vy_netboot_message_t *msg) {
	return sendto(s, msg, msg->message_length, 0, (struct sockaddr *) dest, sizeof(*dest));
}

int socket_create(void) {
	int s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(57778);

	struct timeval time = { .tv_sec = 1 };
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time));

	bind(s, (void *) &addr, sizeof(addr));

	return s;
}
