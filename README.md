# vineyard netboot-server

This is the server-side counterpart to the loader's network boot mode.

This method of booting relies on the fact that IPv6 allows for stateless configuration, which allows us to skip many layers of complexity that a full network stack would have to implement. The client advertises itself by sending a UDP multicast advert that is picked up by the server. After discovery and an ACK, the client stops advertising and starts listening for commands. Commands are sent as UDP packets and must be ACKed before another command can be sent.

## Commands
- fwsetup <remote>: reboot immediately into firmware setup (UEFI)
- reboot <remote>: reboot the client
- info <remote>: dumps network info about the client
- boot <remote>: end the netboot session and let the client's loader continue booting
- send <remote> <local path> <remote path>: copy the file from <local_path> to the clients ESP at <remote_path>

## Roadmap
- support scripting steps (e.g. update files x and y, then boot)
- support waking targets via Wake-on-LAN
- support multiple remotes simultaneously
