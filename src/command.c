#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <readline/readline.h>

#include "netboot.h"

const char *command_strings[] = {
	"advertise",
	"stop_advertising",
	"shutdown",
	"fwsetup",
	"reboot",
	"send file init",
	"file fragment",
	"ack",
};

const char *command_complete_list[] = {
	"reboot",
	"exit",
	"quit",
	"info",
	"fwsetup",
};

void command_print(bool in, remote_t *remote, vy_netboot_message_t *msg) {
#ifdef DEBUG
	printf("[remote %zu] "FORMAT_BOLD"(%s)"FORMAT_RESET" %s (%zd)\n", remote->id, in ? "<-" : "->", command_strings[msg->cmd], msg->message_length);
#endif
}

ssize_t command_send(remote_t *remote, vy_netboot_cmd_t cmd, void *data, size_t data_len) {
	vy_netboot_message_t *msg = calloc(1, NETBOOT_MSG_LEN(data_len));

	msg->cmd = cmd;
	msg->message_length = NETBOOT_MSG_LEN(data_len);

	if(data_len) {
		memcpy(msg->data, data, data_len);
	}

	ssize_t ret = socket_send(remote->socket, &remote->addr, msg);

	if(ret > 0) {
		command_print(false, remote, msg);
	}

	free(msg);

	return ret;
}

ssize_t command_receive(remote_t *remote, vy_netboot_message_t *msg, size_t msg_size) {
	socklen_t addr_size = sizeof(struct sockaddr_in6);
	bool temporary_remote = (!remote->id) ? true : false;

	if(!msg) {
		msg = malloc(1500);

		if(!msg) {
			printf("allocation error\n");
			exit(1);
		}
	}

	ssize_t ret = recvfrom(remote->socket, msg, msg_size, 0, (void *) &remote->addr, &addr_size);

	if(!temporary_remote && ret > 0) {
		command_print(true, remote, msg);
	}

	return ret;
}

char **command_complete(const char *text, int start, __attribute__((unused)) int end) {
	rl_attempted_completion_over = 1;

	if(start) {
		return NULL;
	}

	return rl_completion_matches(text, command_complete_generator);
}

char *command_complete_generator(const char *text, int state) {
	static int list_index;
	static size_t len;
	const char *cmd;

	if(!state) {
		list_index = 0;
		len = strlen(text);
	}

	while((cmd = command_complete_list[list_index++])) {
		if(!strncmp(cmd, text, len)) {
			return strdup(cmd);
		}
	}

	return NULL;
}
