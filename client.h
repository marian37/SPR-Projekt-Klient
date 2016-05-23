#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

#define NICK_LENGTH 10
#define IP_ADDRESS_LENGTH 15
#define SMALL_BUFFER_LENGTH 10
#define MID_BUFFER_LENGTH 100
#define MAX_BUFFER_LENGTH 1024
#define CONFIG_FILE_NAME "client.conf"
#define PROGRAM_TITLE "chat-client"

typedef struct c {
	char IPaddress[IP_ADDRESS_LENGTH];
	int port;
} config;

char *rtrim(char retazec[]);
void log_message(int priority, char * message);
void log_error(char * message);
config read_config_file(char* file_name);
void print_help(void);
void print_active(char* buffer);
void interrupt(int signal);
void quit(int socket_fd, char * buffer, char * nick);
void send_message(int socket_fd, char * my_nick, char * buffer);

#endif /* __CLIENT_H__ */
