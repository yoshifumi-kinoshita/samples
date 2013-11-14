#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#define FILE_SIZE 1024*1024

int main(int argc, char *argv[]) {
	struct sockaddr_in server;
	int sock;
	char *deststr;
	char *filename;

	deststr = argv[1];
	filename = argv[2];

	sock = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_port = htons(12345);

	server.sin_addr.s_addr = inet_addr(deststr);
	if (server.sin_addr.s_addr == 0xffffffff) {
		struct hostent *host;

		host = gethostbyname(deststr);
		if (host == NULL) {
			return 1;
		}
		server.sin_addr.s_addr = *(unsigned int *)host->h_addr_list[0];
	}

	connect(sock, (struct sockaddr *)&server, sizeof(server));
	int fd = open(filename, O_RDONLY);
	int result = sendfile( sock, fd, NULL, FILE_SIZE);

	close(sock);
	return 0;
}

