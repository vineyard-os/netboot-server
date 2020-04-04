#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "netboot.h"
#include "list.h"

#define container_of(ptr, type, member) ({ const typeof(((type *) 0)->member) *__mptr = (ptr); (type *) ((char *) __mptr - offsetof(type,member)); })

static list_t remotes;
size_t remote_id_counter = 1;

void remote_add(int s, remote_t *client) {
	client->id = remote_id_counter++;
	client->socket = s;

	list_append_data(&remotes, client);
}

remote_t *remote_get(size_t id) {
	for(node_t *node = remotes.head; node; node = node->next) {
		remote_t *remote = node->data;

		if(remote->id == id) {
			return remote;
		}
	}

	return NULL;
}

node_t *remote_get_node(size_t id) {
	for(node_t *node = remotes.head; node; node = node->next) {
		remote_t *remote = node->data;

		if(remote->id == id) {
			return node;
		}
	}

	return NULL;
}

remote_t *remote_get_from_string(char *string) {
	remote_t *remote = NULL;

	if(!string || strspn(string, "0123456789") != strlen(string) || !(remote = remote_get(atoi(string)))) {
		printf("unknown remote id %s\n", string);
	}

	return remote;
}

remote_t *remote_get_first(void) {
	if(remotes.head) {
		return remotes.head->data;
	}

	return NULL;
}

remote_t *remote_get_next(remote_t *remote) {
	node_t *node = remote_get_node(remote->id);

	if(node && node->next) {
		return node->next->data;
	} else {
		return NULL;
	}
}

void remote_delete(size_t id) {
	for(node_t *node = remotes.head; node; node = node->next) {
		remote_t *remote = node->data;

		if(remote->id == id) {
			list_remove(&remotes, node);
			free(node);
			free(remote);

			return;
		}
	}
}

void remote_cleanup_all(void) {
	for(remote_t *remote = remote_get_first(); remote;) {
		remote_t *current = remote;

		command_send(remote, CMD_SHUTDOWN, NULL, 0);
		remote = remote_get_next(remote);

		free(current);
	}
}
