#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

#define START_PORT_NUMBER 50065
#define MAX_PORT_NUMBER 50069

#define CMD_CHECK 0
#define CMD_TIME 1
#define CMD_USERS 2
#define CMD_FILE 3
#define CMD_SUM 4
#define CMD_DONE 5
#define CMD_TERMINATE 6

#define READY_MARK "READY"
#define FOUROFOUR_MARK "404 not found"
#define DONE_MARK "Thank you for using MSA. Goodbye!"

int listen_fd;
double max_time;

pthread_mutex_t client_count_mutex;
pthread_mutex_t client_record_mutex;
pthread_cond_t connection_close_cond;
int client_count = 0;

typedef struct ClientRecord {
    int fd;
    struct ClientRecord* next;
} ClientRecord;

ClientRecord* records;

void removeTrailingSpaces(char *str);
void removeSpaces(char *str);
void formatNumber(char *time_str);
int isValidCommand(const char *str);

int printWelcomeMessage(int server_fd, int server_port, double max_time, int max_client) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (getsockname(server_fd, (struct sockaddr *)&addr, &len) < 0) {
        perror("getsockname");
        return 1;
    }

    char ip_str[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &(addr.sin_addr), ip_str, sizeof(ip_str)) == NULL) {
        perror("inet_ntop");
        return 1;
    }

    char time_str[8]; 
    snprintf(time_str, sizeof(time_str), "%.2f", max_time);
    formatNumber(time_str);
    char timeout_str[16]; 
    snprintf(timeout_str, sizeof(timeout_str), "%s seconds", time_str);

    printf("┌────────────────────────────────────────┐\n");
    printf("│         Welcome to MSA Server!         │\n");
    printf("├────────────────────────────────────────┤\n");
    printf("│ Listening Addr/Name : %-16s │\n", ip_str);
    printf("│ Listening Port      : %-16d │\n", server_port);
    printf("│ Max Clients         : %-16d │\n", max_client);
    printf("│ Max Wait Time       : %-16s │\n", timeout_str);
    printf("└────────────────────────────────────────┘\n");
    return 0;
}

void emptyRecords(ClientRecord** head) {
    ClientRecord* current = *head;
    ClientRecord* next_record;
    while (current != NULL) {
        next_record = current->next;
        close(current->fd);
        free(current);
        current = next_record;
        client_count--;
    }
    *head = NULL;
}

void recordClient(ClientRecord** head, int fd) {
    ClientRecord* new_record = (ClientRecord*)malloc(sizeof(ClientRecord));
    new_record->fd = fd;
    new_record->next = *head;
    *head = new_record;
}

char* retrieveClientAddrs(ClientRecord* head) {
    ClientRecord* current = head;
    char* result = malloc(256);
    result[0] = '\0';

    while (current != NULL) {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getpeername(current->fd, (struct sockaddr*)&addr, &addr_len) == 0) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
            int port = ntohs(addr.sin_port);

            char buffer[23];
            snprintf(buffer, sizeof(buffer), "%s:%d\n", ip, port); 
            strncat(result, buffer, 256 - strlen(result) - 1);
        } else {
            perror("getpeername failed");
        }
        current = current->next;
    }

    return result;
}

void removeClientRecord(ClientRecord** head, int fd) {
    ClientRecord* current = *head;
    ClientRecord* prev = NULL;

    while (current != NULL) {
        if (current->fd == fd) {
            if (prev == NULL) {
                *head = current->next;  
            } else {
                prev->next = current->next;  
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

char *getIpAddress(int socket_fd) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(socket_fd, (struct sockaddr *)&addr, &addr_len) == -1) {
        perror("getpeername failed");
        return NULL;
    }

    char *ip_port_str = malloc(INET_ADDRSTRLEN + 8);
    if (ip_port_str == NULL) {
        perror("malloc failed");
        return NULL;
    }

    if (inet_ntop(AF_INET, &addr.sin_addr, ip_port_str, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop failed");
        free(ip_port_str);
        return NULL;
    }

    snprintf(ip_port_str + strlen(ip_port_str), 8, ":%d", ntohs(addr.sin_port));

    return ip_port_str;
}

char* calculateSum(const char* input) {
    double x, y;
    char* result_str = (char*)malloc(256 * sizeof(char));
    if (result_str == NULL) {
        perror("malloc failed");
        return NULL;
    }

    if (sscanf(input, "SUM(%lf,%lf)", &x, &y) == 2) {
        sprintf(result_str, "%lf", x + y);
        formatNumber(result_str);
    } else {
        sprintf(result_str, "Invalid input format");
    }
    return result_str;
}

void sendFileToClient(const char *filename, int socket_fd, char* ip_adddr) {
    FILE *file;
    char buffer[256];

    file = fopen(filename, "r");
    if (file == NULL) {
        write(socket_fd, FOUROFOUR_MARK, sizeof(FOUROFOUR_MARK));
        printf("File \"%s\" not found, sent code 404 to %s\n", filename, ip_adddr); 
        return;
    }

    fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    snprintf(buffer, sizeof(buffer), "file: %ld bytes\n", file_size);
    printf("File  \"%s\" (%ld bytes) founded, initiating the file transfer...\n", filename, file_size); 
    if (write(socket_fd, buffer, strlen(buffer))<0){
        perror("write");
        fclose(file);
        return;
    }
    memset(buffer, 0, sizeof(buffer));
    if (read(socket_fd, buffer, sizeof(buffer)) < 0){
        perror("read");
        fclose(file);
        return;
    }

    if (strncmp(buffer, READY_MARK, strlen(READY_MARK)) != 0 ){
        printf("Fail to receive confirmation of transmission from the client.\n");
        fclose(file);
        return;
    }
    
    size_t bytes_read, bytes_written, total_bytes_written;
    total_bytes_written = 0;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        char *buf_ptr = buffer;
        while (bytes_read > 0) {
            bytes_written = write(socket_fd, buf_ptr, bytes_read);
            if (bytes_written < 0) {
                perror("write");
                fclose(file);
                return;
            }
            bytes_read -= bytes_written;
            buf_ptr += bytes_written;
            total_bytes_written += bytes_written;
        }
    }

    if (bytes_read < 0) {
        perror("read");
    }

    if (total_bytes_written < 256) {
        printf("Sent \"%s\" (%ld bytes) to %s\n", buffer, total_bytes_written, ip_adddr); 
    } else {
        printf("Sent file \"%s\" (%ld bytes) to %s\n", filename, total_bytes_written, ip_adddr); 
    }

    fclose(file);
}

void *RequestProcessor(void *arg) {
    int client_fd = *(int*)arg;
    char *response = NULL;
    char *client_ip = getIpAddress(client_fd);
    if (!client_ip) {
        perror("Failed to get IP address");
        close(client_fd);
        pthread_exit(NULL);
    }
    printf("Connection with %s built.\n", client_ip);
    struct timeval timeout_duration;

    while (1) {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        fd_set monited_fds;
        timeout_duration.tv_sec = (int)max_time;
        timeout_duration.tv_usec = (int)((max_time - timeout_duration.tv_sec) * 1000000);
        FD_ZERO(&monited_fds);
        FD_SET(client_fd, &monited_fds);

        int select_ret = select(client_fd + 1, &monited_fds, NULL, NULL, &timeout_duration);
        if (select_ret == -1) {
            perror("select failed");
            break;
        } else if (select_ret == 0) {
            printf("Session with %s timeout.\n", client_ip);
            break;
        }
        int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 1) {
            if (bytes_read == -1) {
                perror("read failed");
            } else {
                printf("Client %s closed the connection!\n", client_ip);
            }
            break;
        }
        buffer[bytes_read] = '\0'; 

        int command_type = isValidCommand(buffer);
        printf("Recived \"%s\" from client %s\n",buffer,client_ip);
        removeTrailingSpaces(buffer);
        if (command_type == -1) {
            printf("Invalid command revived from client %s, connection closed for safety concideration\n", client_ip);
            break;
        } else if (command_type == CMD_CHECK) {
            if (write(client_fd, buffer, strlen(buffer)) < 0) {
                perror("Respond failed");
                break;
            }
            printf("Sent \"%s\" to %s\n", buffer, client_ip);
            continue;
        } else if (command_type == CMD_TIME){
            time_t now = time(NULL);
            response = strdup(ctime(&now));
        } else if (command_type == CMD_USERS){
            response = retrieveClientAddrs(records);
        } else if (command_type == CMD_FILE){
            char filename[250];
            if (sscanf(buffer, "FILE \"%249[^\"]\"", filename) == 1 || sscanf(buffer, "FILE %249s", filename) == 1){
                sendFileToClient(filename, client_fd, client_ip);
                continue;
            } else {
                response = strdup("Invalid Filename format.");
            }
        } else if (command_type == CMD_SUM){
            removeSpaces(buffer);
            response = calculateSum(buffer);
        } else if (command_type == CMD_DONE){
            response = strdup(DONE_MARK);
        } else if (command_type == CMD_TERMINATE){
            pthread_mutex_lock(&client_record_mutex);
            emptyRecords(&records);
            pthread_mutex_unlock(&client_record_mutex);
            printf("Server terminating...\n");
            exit(0);
        }

        if (response != NULL) {
            size_t len = strlen(response);
            if (len > 0 && response[len - 1] == '\n') {
                response[len - 1] = '\0';
            }
        }
        if (write(client_fd, response, strlen(response)) < 0) {
            perror("write failed");
            free(response);
            break;
        }
        printf("Sent \"%s\" to %s\n", response, client_ip);
        free(response);
    }
    pthread_mutex_lock(&client_record_mutex);
    removeClientRecord(&records, client_fd);
    pthread_mutex_unlock(&client_record_mutex);
    close(client_fd);
    free(client_ip);
    
    pthread_mutex_lock(&client_count_mutex);
    client_count--;
    pthread_cond_signal(&connection_close_cond);
    pthread_mutex_unlock(&client_count_mutex);

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    int max_client;
    records = NULL;

    if(argc != 3){
        printf("Invalid numbers of command line arguments.\nUsage: ./server max_client max_time\n(max_client 1 to 10; max_time 1 to 120 seconds.)\n");
        return 1;
    }
    max_client = atoi(argv[1]);
    char* endptr;
    max_time = strtod(argv[2], &endptr);

    if(max_client < 1 || max_client > 10 || max_time < 1 || max_time > 120 || *endptr != '\0'){
        printf("Invalid argument.\nUsage: ./server max_client max_time\n(max_client 1 to 10; max_time 1 to 120 seconds.)\n");
        return 1;
    }

    struct sockaddr_in serv_addr;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        printf("Unable to create socket\n");
        return 1;
    }

    int optval = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int binded = 0;
    int port;
    for (port = START_PORT_NUMBER; port <= MAX_PORT_NUMBER; port++) {
        serv_addr.sin_port = htons(port);
        if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            if (port != MAX_PORT_NUMBER) printf("Port %d occupied, tring the next port...\n", port);
            continue;
        }
        binded = 1;
        break;
    }

    if (!binded) {
        printf("Unable to bind socket to any port between %d and %d.\n", START_PORT_NUMBER, MAX_PORT_NUMBER);
        return 1;
    }

    if (listen(listen_fd, max_client) < 0) {
        perror("listen failed");
        return 1;
    }

    printWelcomeMessage(listen_fd, port, max_time, max_client);

    pthread_mutex_init(&client_count_mutex, NULL);
    pthread_cond_init(&connection_close_cond, NULL);

    while (1) {
        pthread_mutex_lock(&client_count_mutex);
        while (client_count >= max_client) {
            pthread_cond_wait(&connection_close_cond, &client_count_mutex);
        }
        pthread_mutex_unlock(&client_count_mutex);

        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(listen_fd, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0) {
            printf("Error on accept\n");
            continue;
        }
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, RequestProcessor, &newsockfd) < 0){
            perror("pthread_create failed");
            close(newsockfd);
            continue;
        }
        if (pthread_detach(thread_id) != 0) {
            perror("pthread_detach failed");
            close(newsockfd);
            continue;
        }
        pthread_mutex_lock(&client_count_mutex);
        client_count++;
        pthread_mutex_unlock(&client_count_mutex);
        pthread_mutex_lock(&client_record_mutex);
        recordClient(&records, newsockfd);
        pthread_mutex_unlock(&client_record_mutex);
    }
    pthread_mutex_destroy(&client_count_mutex);
    pthread_cond_destroy(&connection_close_cond);
    return 0;
}