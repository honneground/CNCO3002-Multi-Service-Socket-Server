#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>

#define DONE_MARK "Thank you for using MSA. Goodbye!"
#define TERMINATION_MARK "Thank you for using MSA. Goodbye!"
#define HEARTBEAT_MARK "ALIVE"
#define READY_MARK "READY"

#define CMD_CHECK 0
#define CMD_TIME 1
#define CMD_USERS 2
#define CMD_FILE 3
#define CMD_SUM 4
#define CMD_DONE 5
#define CMD_TERMINATE 6

#define TIME_OUT_SEC 3

int session_fd ;
void removeTrailingSpaces(char *str);
int isValidCommand(const char *str);
int child_blocking = 0;



void printWelcomeMessage(const char *server_name, int server_port) {
    char timeout_str[17]; 
    snprintf(timeout_str, sizeof(timeout_str), "%d seconds", TIME_OUT_SEC);
    printf("┌────────────────────────────────────────┐\n");
    printf("│         Welcome to MSA Client!         │\n");
    printf("├────────────────────────────────────────┤\n");
    printf("│ Server IP          : %-17s │\n", server_name);
    printf("│ Server Port        : %-17d │\n", server_port);
    printf("│ Response Tolerance : %-17s │\n", timeout_str);
    printf("└────────────────────────────────────────┘\n");
}

void triggerConnectionTimeout(int sig) {
    printf("Connection timed out!\n");
    exit(1);
}

void *executeClientActions(void *arg) {
    char buffer[256];
    char command_buffer[256];

    while (1) {
        printf("Client> ");
        memset(command_buffer, 0, sizeof(command_buffer));
        memset(buffer, 0, sizeof(buffer));

        child_blocking = 1;
        if (fgets(command_buffer, sizeof(command_buffer), stdin) == NULL) {
            perror("Error reading input");
            continue;
        }

        child_blocking = 0;

        command_buffer[strcspn(command_buffer, "\n")] = '\0';

        int command_type = isValidCommand(command_buffer);
        if (command_type < 1) {
            printf("Invalid command, please try again!\n");
            continue;
        } else {
            removeTrailingSpaces(command_buffer);
        }

        int bytes_written = write(session_fd , command_buffer, strlen(command_buffer));
        if (bytes_written < 0) {
            perror("Error writing to socket");
            exit(1);
        }
        if (command_type == CMD_TERMINATE){
            child_blocking = 2;
            return NULL;
        }
        fd_set monited_fds;
        struct timeval timeout_duration;
        timeout_duration.tv_sec = TIME_OUT_SEC;
        timeout_duration.tv_usec = 0;

        FD_ZERO(&monited_fds);
        FD_SET(session_fd , &monited_fds);

        int server_reply_check = select(session_fd + 1, &monited_fds, NULL, NULL, &timeout_duration);
        if (server_reply_check < 0) {
            perror("Error while monitoring server reply");
            exit(1);
        } else if (server_reply_check == 0) {
            printf("\nServer response timed out.\n");
            continue;
        }

        int bytes_read = read(session_fd , buffer, sizeof(buffer) -1);

        if (bytes_read < 0) {
            perror("Error reading from socket");
            exit(1);
        } else if (bytes_read == 0) {
            printf("Server has closed the connection. Exiting...\n");
            break;
        }
        buffer[bytes_read] = '\0'; 
        long expected_file_size;
        FILE *logfile = fopen("log", "a");
        if (logfile == NULL) {
            perror("Error opening log file");
        }

        if (sscanf(buffer, "file: %ld bytes", &expected_file_size) == 1) {
            fprintf(logfile, "%s; the result is: \n", command_buffer);
            ssize_t bytes_received, total_bytes_read;
            total_bytes_read = 0;
            if (write(session_fd, READY_MARK, strlen(READY_MARK)) < 0 ) {
                perror("Fail to send confirmation of transmission to the client.");
                fclose(logfile);
                continue;
            }

            while (total_bytes_read < expected_file_size) {
                memset(buffer, 0, sizeof(buffer));
                timeout_duration.tv_sec = TIME_OUT_SEC;
                timeout_duration.tv_usec = 0;
                FD_ZERO(&monited_fds);
                FD_SET(session_fd , &monited_fds);

                int file_transfer_check = select(session_fd + 1, &monited_fds, NULL, NULL, &timeout_duration);
                if (file_transfer_check < 0) {
                    perror("Error while monitoring file data");
                    break;
                } else if (file_transfer_check == 0) {
                    printf("\nTimeout waiting for file data.\n");
                    break;
                }
                bytes_received = read(session_fd , buffer, sizeof(buffer));
                if (bytes_received == 0) {
                    printf("Connection closed unexpectedly.\n");
                    break;
                }
                if (fwrite(buffer, 1, bytes_received, stdout) != bytes_received) {
                    perror("Error writing to stdout");
                }

                if (fwrite(buffer, 1, bytes_received, logfile) != bytes_received) {
                    perror("Error writing to log file");
                }

                total_bytes_read += bytes_received;
            }
            printf("\n");
            fprintf(logfile, "\n");

            if (total_bytes_read != expected_file_size) {
                printf("Warning: expected %ld bytes, but read %ld bytes\n", expected_file_size, total_bytes_read);

            }
        } else {
            printf("%s\n", buffer);
            if (command_type == CMD_USERS) fprintf(logfile, "%s; the result is: \n%s\n", command_buffer, buffer);
            else fprintf(logfile, "%s; the result is: %s\n", command_buffer, buffer);
        }
        fclose(logfile);
        if (strncmp(buffer, DONE_MARK, strlen(DONE_MARK)) == 0) break;
    }
    close(session_fd );
    exit(0);
}

int main(int argc, char *argv[]) {
    int port_number;
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;

    if (argc != 3) {
        printf("Invalid numbers of command line arguments.\nUsage: \n./client Server_IP_address port\nor\n./client Server_machine_name port\n");
        return 1;
    }

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        struct hostent *host_info;
        struct in_addr **server_address;

        if ((host_info = gethostbyname(argv[1])) == NULL) {
            herror("gethostbyname");
            return 1;
        }

        server_address = (struct in_addr **)host_info->h_addr_list;

        if (server_address[0] == NULL) {
            fprintf(stderr, "No address found for hostname: %s\n", argv[1]);
            return 1;
        }

        servaddr.sin_addr = *server_address[0];
    }

    port_number = atoi(argv[2]);
    if (port_number == 0) {
        fprintf(stderr, "Invalid port number: %s\n", argv[2]);
        return 1;
    }

    servaddr.sin_port = htons(port_number);

    session_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (session_fd < 0) {
        perror("Error creating socket");
        exit(1);
    }
    signal(SIGALRM, triggerConnectionTimeout);
    alarm(TIME_OUT_SEC);

    if (connect(session_fd , (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Error connecting to server");
        exit(1);
    }

    if (write(session_fd ,HEARTBEAT_MARK,strlen(HEARTBEAT_MARK))<0){
        perror("Error sending heartbeat");
        exit(1);
    }

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    if (read(session_fd , buffer, sizeof(buffer))<0 || strncmp(buffer, HEARTBEAT_MARK, 4) != 0){
        printf("Invalid response from the server.\n");
        exit(1);
    }

    printWelcomeMessage(argv[1], port_number);

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, executeClientActions, NULL) != 0) {
        perror("Error creating client thread");
        exit(1);
    }
    pthread_detach(client_thread);
    alarm(0);

    fd_set monited_fds;
    int activity;

    while (1) {
        FD_ZERO(&monited_fds);
        FD_SET(session_fd , &monited_fds);

        activity = select(session_fd  + 1, &monited_fds, NULL, NULL, NULL);

        if (activity < 0) {
            break;
        } else if (activity == 0 || child_blocking > 0) {
            close(session_fd );
            if (child_blocking == 1) printf("\nServer has closed the connection. Exiting...\n");
            else printf("Server has closed the connection. Exiting...\n");
            break;
        }
    }
    return 0;
}