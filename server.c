#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "netboot.h"

int main() {
	rl_attempted_completion_function = command_complete;

	/* add some commonly used commands to history to speed things up */
	add_history("send 1 boot/efi/boot/bootx64.efi \\efi\\boot\\bootx64.efi");
	add_history("send 1 boot/kernel \\kernel");

	bool advert_listening = true;

	printf(FORMAT_BOLD FORMAT_CYAN "vineyard netboot server" FORMAT_RESET "\n");

	vy_netboot_message_t *advert;

	int s = socket_create();

	while(advert_listening) {
		remote_t *remote = calloc(1, sizeof(remote_t));
		advert = malloc(offsetof(vy_netboot_message_t, data));

		if(!remote || !advert) {
			printf("allocation error\n");
			exit(1);
		}

		remote->socket = s;

		ssize_t r = 0;

		while(r != NETBOOT_ADVERTISEMENT_LEN) {
			r = command_receive(remote, advert, NETBOOT_ADVERTISEMENT_LEN);
		}

		if(r == NETBOOT_ADVERTISEMENT_LEN && advert->cmd == CMD_ADVERTISE && advert->message_length == NETBOOT_ADVERTISEMENT_LEN) {
			remote_add(s, remote);
			command_print(true, remote, advert);
			command_send(remote, CMD_STOP_ADVERTISING, NULL, 0);
			free(advert);

			advert_listening = false;
		} else {
			free(remote);
			free(advert);
		}

		if(r == -1) {
			printf("%s (%d) while listening for adverts\n", strerror(errno), errno);
			exit(1);
		}
	}

	shell_loop();
	remote_cleanup_all();

	close(s);
}
