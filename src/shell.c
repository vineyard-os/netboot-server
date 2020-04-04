#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "netboot.h"

#define SHELL_TOKEN_BUFFER_LEN 8
#define SHELL_TOKEN_DELIMITER " \t\r\n\a"

struct {
	const char *name;
	bool (*function)(remote_t *remote);
} commands_remote_arg[] = {
	{"fwsetup", shell_cmd_fwsetup},
	{"reboot", shell_cmd_reboot},
	{"info", shell_cmd_info},
	{"boot", shell_cmd_boot},
	{NULL, NULL},
};

char *shell_read_line(void) {
	char *line = readline("vy-netboot > ");

	if(line) {
		add_history(line);
	}

	return line;
}

char **shell_parse_line(char *line) {
	int bufsize = SHELL_TOKEN_BUFFER_LEN;
	int position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if(!tokens) {
		printf("allocation error\n");
		exit(1);
	}

	token = strtok(line, SHELL_TOKEN_DELIMITER);

	while(token) {
		tokens[position] = token;
		position++;

		if(position >= bufsize) {
			bufsize += SHELL_TOKEN_BUFFER_LEN;
			tokens = realloc(tokens, bufsize * sizeof(char*));

			if(!tokens) {
				printf("allocation error\n");
				exit(1);
			}
		}

		token = strtok(NULL, SHELL_TOKEN_DELIMITER);
	}

	tokens[position] = NULL;
	return tokens;
}

bool shell_execute(char **args) {
	if(!args[0]) {
		return true;
	}

	for(size_t i = 0; commands_remote_arg[i].name && commands_remote_arg[i].function; i++) {
		if(!strcmp(commands_remote_arg[i].name, args[0])) {
			if(!args[1]) {
				printf("no remote given\n");
				return true;
			}

			remote_t *remote = remote_get_from_string(args[1]);

			if(!remote) {
				return true;
			}

			return commands_remote_arg[i].function(remote);
		}
	}

	if(!strncmp(args[0], "exit", 5) || !strncmp(args[0], "quit", 5) || !strncmp(args[0], "q", 2)) {
		return false;
	} else if(!strncmp(args[0], "send", 5)) {
		if(!args[1]) {
			printf("no remote specified");
			return true;
		} else if(!args[2]) {
			printf("no file specified");
		} else if(!args[3]) {
			printf("no destination path specified");
		}

		char *local_path = args[2];
		char *remote_path = args[3];

		printf("%s -> remote %s\n", local_path, remote_path);

		shell_cmd_transfer(args[1], local_path, remote_path);
	} else {
		printf("unknown command \"%s\"\n", args[0]);
	}

	return true;
}

void shell_loop(void) {
	bool shell_running = true;

	do {
		char *line = shell_read_line();
		char **args = shell_parse_line(line);
		shell_running = shell_execute(args);

		free(line);
		free(args);
	} while(shell_running);
}
