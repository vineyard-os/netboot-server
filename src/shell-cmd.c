#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "netboot.h"

bool shell_cmd_fwsetup(remote_t *remote) {
	command_send(remote, CMD_FWSETUP, NULL, 0);

	return true;
}

bool shell_cmd_reboot(remote_t *remote) {
	command_send(remote, CMD_REBOOT, NULL, 0);

	return true;
}

bool shell_cmd_info(remote_t *remote) {
	char ip6[INET6_ADDRSTRLEN];
	inet_ntop(remote->addr.sin6_family, &remote->addr.sin6_addr, ip6, INET6_ADDRSTRLEN);
	uint8_t *ip = (uint8_t *) &remote->addr.sin6_addr;

	printf("remote: %zu\nIPv6: %s\nport: %u\n", remote->id, ip6, ntohs(remote->addr.sin6_port));

	if(ip[0] == 0xFE && ip[1] == 0x80) {
		printf("stateless configuration/link-local: true\nMAC: %02x:%02x:%02x:%02x:%02x:%02x\n", ip[8] ^ 2, ip[9], ip[10], ip[13], ip[14], ip[15]);
	} else {
		printf("stateless configuration/link-local: false\n");
	}

	return true;
}

bool shell_cmd_boot(remote_t *remote) {
	command_send(remote, CMD_BOOT, NULL, 0);

	return false;
}

bool shell_cmd_transfer(char *remote_id, char *local_path, char *remote_path) {
	remote_t *remote = remote_get_from_string(remote_id);

	if(!remote) {
		return true;
	}

	FILE *f = fopen(local_path, "rb");

	if(!f) {
		printf("fopen: %s\n", strerror(errno));
		return true;
	}

	if(fseek(f, 0, SEEK_END)) {
		printf("fseek: %s\n", strerror(errno));
		return true;
	}

	long file_size = ftell(f);

	if(file_size < 0) {
		printf("ftell: %s\n", strerror(errno));
		return true;
	}

	rewind(f);

	size_t info_size = NETBOOT_MSG_LEN(sizeof(vy_netboot_file_info_t) + strlen(remote_path) + 1);
	vy_netboot_file_info_t *info = malloc(info_size);
	vy_netboot_message_t *ack = malloc(NETBOOT_ACK_LEN);

	if(!info || !ack) {
		printf("allocation error\n");
		exit(1);
	}

	vy_netboot_ack_t *ack_data = (void *) ack->data;

	info->file_size = file_size;
	info->data_per_packet = NETBOOT_MSG_MAX_LEN - NETBOOT_MSG_LEN(sizeof(vy_netboot_file_fragment_t));
	info->packets = info->file_size / info->data_per_packet;

	if(info->file_size % info->data_per_packet) {
		info->packets++;
	}

	info->path_len = strlen(remote_path);
	memcpy(info->path, remote_path, info->path_len);
	info->path[info->path_len] = '\0';

	struct timeval start_time;
	gettimeofday(&start_time, NULL);

	command_send(remote, CMD_FILE_SEND_INIT, info, info_size);

	if(command_receive(remote, ack, NETBOOT_ACK_LEN) < 0 || ack->cmd != CMD_ACK) {
		printf(FORMAT_RED "init ack took too long\n" FORMAT_RESET);
		return true;
	}

	int64_t passed_us = 0;

	do {
		struct timeval now;
		gettimeofday(&now, NULL);
		passed_us = (int64_t)(now.tv_sec - start_time.tv_sec) * 1000000 + ((int64_t)now.tv_usec - (int64_t) start_time.tv_usec);
	} while(passed_us < 750);

	for(size_t i = 0; i < info->packets; i++) {
		gettimeofday(&start_time, NULL);

		vy_netboot_file_fragment_t *fragment = malloc(info->data_per_packet + sizeof(vy_netboot_file_fragment_t));

		if(!fragment) {
			printf("allocation error\n");
			exit(1);
		}

		fragment->fragment_id = i;
		fragment->data_len = (i == (info->packets - 1)) ? (info->file_size % info->data_per_packet) : info->data_per_packet;

		fseek(f, i * info->data_per_packet, SEEK_SET);
		fread(&fragment->data, fragment->data_len, 1, f);

		command_send(remote, CMD_FILE_FRAGMENT, fragment, fragment->data_len + sizeof(vy_netboot_file_fragment_t));

		ssize_t r = 0;

		while(ack->cmd != CMD_ACK) {
			r = command_receive(remote, ack, NETBOOT_ACK_LEN);

			if(r < 0) {
				printf(FORMAT_YELLOW "packet %lu/%lu ack took too long, retrying\n" FORMAT_RESET, i, info->packets);

				i--;
				break;
			}

			if(ack_data->file_fragment != i) {
				i = (ack_data->file_fragment > 0) ? (ack_data->file_fragment - 1) : 0;
				break;
			}
		}

		memset(ack, 0, sizeof(*ack));

		do {
			struct timeval now;
			gettimeofday(&now, NULL);
			passed_us = (int64_t)(now.tv_sec - start_time.tv_sec) * 1000000 + ((int64_t)now.tv_usec - (int64_t) start_time.tv_usec);
		} while(passed_us < 750);
	}

	return true;
}
