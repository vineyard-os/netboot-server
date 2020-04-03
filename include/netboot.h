#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "list.h"

#define NETBOOT_MSG_LEN(x) (offsetof(vy_netboot_message_t, data) + (x))
#define NETBOOT_ADVERTISEMENT_LEN NETBOOT_MSG_LEN(0)
#define NETBOOT_ACK_LEN NETBOOT_MSG_LEN(sizeof(vy_netboot_ack_t))

#define NETBOOT_MSG_MAX_LEN 1452UL

#define FORMAT_BOLD "\x1B[1m"
#define FORMAT_RED "\x1B[31m"
#define FORMAT_GREEN "\x1B[32m"
#define FORMAT_YELLOW "\x1B[33m"
#define FORMAT_CYAN "\x1B[36m"
#define FORMAT_RESET "\x1B[0m"

typedef enum {
	CMD_ADVERTISE,
	CMD_STOP_ADVERTISING,
	CMD_SHUTDOWN,
	CMD_FWSETUP,
	CMD_REBOOT,
	CMD_FILE_SEND_INIT,
	CMD_FILE_FRAGMENT,
	CMD_ACK,
	CMD_BOOT,
} vy_netboot_cmd_t;

typedef struct {
	vy_netboot_cmd_t cmd;
	uint64_t message_length;
	uint8_t data[0];
} vy_netboot_message_t;

typedef struct {
	size_t file_size;
	size_t data_per_packet;
	size_t packets;
	size_t path_len;
	char path[0];
} vy_netboot_file_info_t;

typedef struct {
	size_t fragment_id;
	size_t data_len;
	uint8_t data[0];
} vy_netboot_file_fragment_t;

typedef struct {
	vy_netboot_cmd_t type;
	size_t file_fragment;
} vy_netboot_ack_t;

typedef struct {
	size_t id;
	int socket;
	struct sockaddr_in6 addr;
} remote_t;

ssize_t socket_send(int s, struct sockaddr_in6 *dest, vy_netboot_message_t *msg);
int socket_create(void);

void command_print(bool in, remote_t *remote, vy_netboot_message_t *msg);
ssize_t command_send(remote_t *remote, vy_netboot_cmd_t cmd, void *data, size_t data_len);
ssize_t command_receive(remote_t *remote, vy_netboot_message_t *msg, size_t msg_size);

char **command_complete(const char *text, int start, int end);
char *command_complete_generator(const char *text, int state);

char *shell_read_line(void);
char **shell_parse_line(char *line);
bool shell_execute(char **args);
void shell_loop(void);

void remote_add(int s, remote_t *client);
remote_t *remote_get(size_t id);
node_t *remote_get_node(size_t id);
remote_t *remote_get_from_string(char *string);
remote_t *remote_get_first(void);
remote_t *remote_get_next(remote_t *remote);
void remote_delete(size_t id);
void remote_cleanup_all(void);

bool shell_cmd_fwsetup(remote_t *remote);
bool shell_cmd_reboot(remote_t *remote);
bool shell_cmd_info(remote_t *remote);
bool shell_cmd_boot(remote_t *remote);
bool shell_cmd_transfer(char *remote, char *local_path, char *remote_path);

extern size_t remote_id_counter;
