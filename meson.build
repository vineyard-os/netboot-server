project('netboot-server', 'c')
add_project_arguments('-Wformat', language: 'c')

warning_level = 3

include_dir = include_directories('include')
readline = dependency('readline')

src = [
	'src/command.c',
	'src/list.c',
	'src/remote.c',
	'src/server.c',
	'src/shell-cmd.c',
	'src/shell.c',
	'src/socket.c'
]

executable('server', src, include_directories: include_dir, dependencies: [readline], install: true)
