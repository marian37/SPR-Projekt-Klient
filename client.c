#include "client.h"

static int interrupted = 0;

char *rtrim(char string[])
{
	int i;	
	for (i = strlen(string); i >= 0; i--) {	
		if (string[i] < 32) {
			string[i] = 0;
		} else {
			break;
		}
	}
	return string;
}

void log_message(int priority, char * message) 
{
	printf("%s\n", message);
	syslog(priority, "%s - %m\n", message);
}

void log_error(char * message)
{
	log_message(LOG_ERR, message);
	exit(EXIT_FAILURE);
}

config read_config_file(char* file_name)
{
	FILE * file;
	config conf; 
	char buffer[SMALL_BUFFER_LENGTH];

	file = fopen(file_name, "r");
	if (file == NULL) {
		log_error("Chyba pri čítaní súboru.");
	}

	fgets(conf.IPaddress, IP_ADDRESS_LENGTH + 1, file);
	fgets(buffer, SMALL_BUFFER_LENGTH + 1, file);
	conf.port = atoi(buffer);

	fclose(file);
	return conf;
}

void print_help()
{
	printf("--------------\n");
	printf("NÁPOVEDA:\n");
	printf("Príkazom \"help\" spustíte nápovedu.\n");
	printf("Príkazom \"list\" vypíšete aktívnych používateľov pripojených k serveru.\n");
	printf("Príkazom \"quit\" ukončíte program.\n");
	printf("Správy posielajte v tvare [nick]#[správa] a potvrďte klávesou Enter.\n");
	printf("Verejnú správu (broadcast) je možné zaslať vo forme []#[správa].\n");
	printf("--------------\n");
}

void print_active(char* buffer)
{
	char *token;
	char * delimiter = ";";

	printf("--------------\n");
	printf("AKTÍVNI POUŽÍVATELIA:\n");

	token = strtok(buffer, delimiter);
	while (token != NULL) {		
		printf("%s\n", token);	
		token = strtok(NULL, delimiter);
	}
	printf("--------------\n");
}

void print_message(char * nick, char * buffer)
{
	char * from, * message;
	char * delimiter = "#";
	char string[MAX_BUFFER_LENGTH];
	char time_buffer[MID_BUFFER_LENGTH];
	time_t rawtime;
	struct tm *info;

	from = strtok(buffer, delimiter);
	message = strtok(NULL, delimiter);

	time(&rawtime);
	info = localtime(&rawtime);

	memset(time_buffer, 0, MID_BUFFER_LENGTH);
	memset(string, 0, MAX_BUFFER_LENGTH);
	strftime(time_buffer, MID_BUFFER_LENGTH, "%d.%m.%Y %H:%M:%S", info);
	sprintf(string, "%s @%s: %s", time_buffer, from, rtrim(message));
	printf("\n");
	log_message(LOG_INFO, string);

	printf("%s: ", nick);
	fflush(stdout);
}

void interrupt(int signal)
{
	interrupted = 1;
}

void quit(int socket_fd, char * buffer, char * nick)
{
	printf("Ukončujem program...\n");
	memset(buffer, 0, MAX_BUFFER_LENGTH);
	strcpy(buffer, "quit ");
	strcat(buffer, nick);
	write(socket_fd, buffer, strlen(buffer));
	close(socket_fd);
	closelog();
	exit(EXIT_SUCCESS);
}

void send_message(int socket_fd, char * my_nick, char * buffer)
{
	char message[MAX_BUFFER_LENGTH];
	char nick[NICK_LENGTH + 1];
	char * token;
	char * delimiter = "#";
	
	memset(message, 0, MAX_BUFFER_LENGTH);
	memset(nick, 0, NICK_LENGTH + 1);

	if (strncmp(delimiter, buffer, 1) == 0) {
		nick[0] = 0;
		token = strtok(buffer, delimiter);	
	} else {
		strcpy(nick, strtok(buffer, delimiter));
		token = strtok(NULL, delimiter);
	}
	
	if (token == NULL) {
		log_message(LOG_WARNING, "Zadali ste správu v nesprávnom tvare! Zadajte správu v tvare [nick]#[správa].");
		return;
	} 

	sprintf(message, "%s%s%s%s%s", my_nick, delimiter, nick, delimiter, token);

	write(socket_fd, message, MAX_BUFFER_LENGTH);
}

int main(int argc, char *argv[])
{
	char nick[NICK_LENGTH + 1];
	char buffer[MAX_BUFFER_LENGTH];
	int socket_fd, len, result;
	int errno;
	struct sockaddr_in address;
	fd_set sockset;
	config conf;
	struct timeval timeout;

	errno = 0;

	/* otvorenie system logger-u */
	openlog(PROGRAM_TITLE, LOG_PID|LOG_CONS, LOG_USER);

	/* načítanie konfiguráku */
	conf = read_config_file(CONFIG_FILE_NAME);

	/* vytvorenie soketu */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (socket_fd == -1) {
		log_error("Chyba pri vytváraní soketu.");
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(conf.IPaddress);
	address.sin_port = htons(conf.port);
	len = sizeof(address);

	/* pripojenie soketu */	
	result = connect(socket_fd, (struct sockaddr *)&address, len);
	if (result == -1) {
		log_error("Chyba pri pripojení k soketu.");
	}

	/* Zadanie nick-u */
	printf("Zadajte nick: [Maximálne 10 znakov]\n");
	if (fgets(nick, NICK_LENGTH + 1, stdin) == NULL) {
		log_error("Chyba pri čítaní nick-u.");
	}
	rtrim(nick);	

	/* overenie nick-u */
	strcpy(buffer, "check ");
	strcat(buffer, nick); 
	write(socket_fd, buffer, strlen(buffer));
	read(socket_fd, buffer, SMALL_BUFFER_LENGTH);

	/* zlý nick - opakovanie voľby */
	while (strncmp("ok", buffer, 2) != 0) {
		log_message(LOG_NOTICE, "Zadali ste zlý nick!");
		
		write(socket_fd, "list", 4);
		memset(buffer, 0, MAX_BUFFER_LENGTH);
		read(socket_fd, buffer, MAX_BUFFER_LENGTH);				
		print_active(buffer);
		
		printf("Zadajte nový nick: [Maximálne 10 znakov]\n");

		if (fgets(nick, NICK_LENGTH + 1, stdin) == NULL) {
			log_error("Chyba pri čítaní nick-u.\n");
		}
		rtrim(nick);	

		/* overenie nick-u */
		strcpy(buffer, "check ");
		strcat(buffer, nick); 
		write(socket_fd, buffer, strlen(buffer));
		read(socket_fd, buffer, SMALL_BUFFER_LENGTH);
	}

	printf("--------------\n");
	printf("Váš nick je: %s\n", nick);
	printf("Pre zobrazenie nápovedy napíšte \"help\".\n");
	printf("--------------\n");

	printf("%s: ", nick);
	fflush(stdout);

	signal(SIGINT, interrupt);

	while (1) {
		/* ukončenie program pomocou SIGINT */
		if (interrupted) {
			quit(socket_fd, buffer, nick);
		}

		/* checknutie či nedošla správa */
		FD_ZERO(&sockset);
		FD_SET(socket_fd, &sockset);
		timeout.tv_usec = 100000;
		result = select(socket_fd + 1, &sockset, NULL, NULL, &timeout);
		if (result < 0) {
			log_error("Chyba príkazu select na sokete.");
		} else {
			if  (result == 1) {
				if (FD_ISSET(socket_fd, &sockset)) {
					memset(buffer, 0, MAX_BUFFER_LENGTH);
					read(socket_fd, buffer, MAX_BUFFER_LENGTH);
					if (strcmp("server quit", buffer) == 0) {
						log_message(LOG_INFO, "Server bol zastavený, alebo vypršal časový limit. Ukončujem klienta...");
						close(socket_fd);
						exit(EXIT_SUCCESS);
					}
					if (strcmp("warning", buffer) == 0) {
						log_message(LOG_WARNING, "Poslali ste správu používateľovi, ktorý nie je aktívny. Príkazom \"list\" zobrazíte aktívnych používateľov.");
						printf("%s: ", nick);
						fflush(stdout);
					} else {
						print_message(nick, buffer);
					}
				}
			} 
		}

		FD_ZERO(&sockset);
		FD_SET(0, &sockset);
		timeout.tv_sec = 1;
		result = select(1, &sockset, NULL, NULL, &timeout);
		if (result < 0) {
			if (interrupted) {
				quit(socket_fd, buffer, nick);
			}
			log_error("Chyba príkazu select na stdin.");
		} else {
			if (result == 1) {
				if (FD_ISSET(0, &sockset)) {
					/* zadávanie príkazov */
					if (fgets(buffer, MAX_BUFFER_LENGTH, stdin) != NULL) {
						if (strncmp("list", buffer, 4) == 0) {
							write(socket_fd, "list", 4);
							memset(buffer, 0, MAX_BUFFER_LENGTH);
							read(socket_fd, buffer, MAX_BUFFER_LENGTH);				
							print_active(buffer);
							printf("%s: ", nick);
							fflush(stdout);
							continue;
						}
						if (strncmp("help", buffer, 4) == 0) {
							print_help();
							printf("%s: ", nick);
							fflush(stdout);
							continue;
						}
						if (strncmp("quit", buffer, 4) == 0) {
							/* upovedomenie serveru */
							quit(socket_fd, buffer, nick);
						}
						/* odoslanie správy */						
						send_message(socket_fd, nick, buffer);
						printf("%s: ", nick);
						fflush(stdout);
					}
				}
			}
		}				
	}

	close(socket_fd);
	closelog();
	return EXIT_SUCCESS;
}
