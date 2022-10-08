/**
* File Name: aesdsocket.c
* File Desciption: This file contains the main function for AESD Assignment 5 Part 1.
* File Author: Gautama Gandhi
* Reference: Linux man pages and Linux System Programming Textbook
**/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <syslog.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/fs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>

#define BACKLOG 10 // Setting 10 as the connection request limit while listening
#define INITIAL_BUFFER_SIZE 1024 // Buffer size for receive and storage buffers
#define WRITE_BUFFER_SIZE 512 // Write buffer size

// Global Variables
int socketfd; // Socket File Descriptor
int testfile_fd; // Testfile File Descriptor
// int new_fd; // File Descriptor used to receive and send data
//char *storage_buffer; // Pointer to storage buffer


pthread_mutex_t mutex;

// Signal handler for SIGINT and SIGTERM signals
void sig_handler(int signum)
{
    if (signum == SIGINT)
        syslog(LOG_DEBUG, "Caught signal SIGINT, exiting");
    else if (signum == SIGTERM)
        syslog(LOG_DEBUG, "Caught signal SIGTERM, exiting");

    //Closing socketfd FD
    int status = close(socketfd);
    if (status == -1) {
        syslog(LOG_ERR, "Unable to close socket FD with error %d", errno);
    }

    // // Closing testfile FD
    // status = close(new_fd);
    // if (status == -1) {
    //     syslog(LOG_ERR, "Unable to close new_fd FD with error %d", errno);
    // }

    // Closing testfile FD
    status = close(testfile_fd);
    if (status == -1) {
        syslog(LOG_ERR, "Unable to close testfile FD with error %d", errno);
    }

    unlink("/var/tmp/aesdsocketdata");
    if (status == -1) {
        syslog(LOG_ERR, "Unable to unlink aesdsocketdata file with error %d", errno);
    }

    closelog();
    exit(EXIT_SUCCESS);
}

int signal_initializer()
{
    struct sigaction act;
    act.sa_handler = &sig_handler;

    sigfillset(&act.sa_mask);

    act.sa_flags = SA_RESTART;

    if(sigaction(SIGINT, &act, NULL) == -1)
	{
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

    if (sigaction(SIGTERM, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

// Function to create a daemon; based on example code given in the LSP textbook
static int daemon_create()
{
    pid_t pid;

    /* create new process */
    pid = fork ();

    if (pid == -1)
        return -1;

    else if (pid != 0)
        exit (EXIT_SUCCESS);
    /* create new session and process group */
    if (setsid () == -1)
        return -1;
    /* set the working directory to the root directory */
    if (chdir ("/") == -1)
        return -1;

    /* redirect fd's 0,1,2 to /dev/null */
    open ("/dev/null", O_RDWR); /* stdin */
    dup (0); /* stdout */
    dup (0); /* stderror */

    return 0;
}

typedef struct {
    pthread_t threadid;
    int complete_flag;
    int connection_fd;
} thread_data_t;

typedef struct node_s {
    thread_data_t thread_data;
    struct node_s *next;
} node_t;

// Insert element into linked list
void ll_insert(node_t **head_ref, node_t *node)
{

}


// Thread Function
void* threadfunc(void* thread_param)
{
    thread_data_t *thread_local_vars = (thread_data_t *)thread_param;

    printf("\n%d\n", thread_local_vars->connection_fd);

    char *storage_buffer = (char *)malloc(INITIAL_BUFFER_SIZE);

    // Static receive buffer of size 1024
    char buffer[INITIAL_BUFFER_SIZE];

    // Variable to store size of storage buffer
    int storage_buffer_size = INITIAL_BUFFER_SIZE;

    // Variable to store return value from recv
    int bytes_received;

    // Variable to store the value of the iterator within the for loop to check for "\n" character in the while loop
    int temp_iterator = 0;

    // Flag that is set when \n is received
    int packet_complete_flag = 0;

    // Variable to track the size of the storage buffer 1 indicates size is 1*1024; used when computing bytes to be written and memcpy operation on the storage buffer
    int storage_buffer_size_count = 1;

    // Variable that stores number of bytes to be written to the local file after completion of a data packet
    int bytes_to_write = 0;

    // Variable that stores bytes written to file and is used to read from the file
    int file_size = 0;

    int previous_byte_count = 0;

    // While loop until bytes received is <= 0
    while ((bytes_received = recv(thread_local_vars->connection_fd, buffer, sizeof(buffer), 0)) > 0 ) {

        printf("Bytes Received = %d\n", bytes_received);

        // Edge case
        if (previous_byte_count + bytes_received > storage_buffer_size) {
            char *temp_ptr  = (char *)realloc(storage_buffer, (storage_buffer_size + INITIAL_BUFFER_SIZE));
            if (temp_ptr) {
                storage_buffer = temp_ptr;
                storage_buffer_size += INITIAL_BUFFER_SIZE;
                storage_buffer_size_count += 1;
            } else {
                printf("Unable to allocate memory\n");
            }
        }

        int i;
        // Iterating over bytes received to check for "\n" character
        for (i = 0; i < bytes_received; i++) {
            if (buffer[i] == '\n') {
                packet_complete_flag = 1;
                temp_iterator = i+1;
                break;
            }
        }

        // Copying from receive buffer to storage buffer 
        memcpy(storage_buffer + (storage_buffer_size_count - 1)*INITIAL_BUFFER_SIZE + previous_byte_count, buffer, bytes_received);

        // Check if data packet is complete and assign bytes to write based on temporary iterator
        if (packet_complete_flag == 1) {
            bytes_to_write = (storage_buffer_size_count - 1)*INITIAL_BUFFER_SIZE + temp_iterator + previous_byte_count;
            previous_byte_count = 0;
            break;
        } else {

            if ((i == bytes_received) && (bytes_received < INITIAL_BUFFER_SIZE)) {
                previous_byte_count += bytes_received;
            }

            else if (bytes_received == INITIAL_BUFFER_SIZE){
                // Increasing size of storage buffer by INITIAL_BUFFER_SIZE amount if no "\n" character found in buffer
                char *temp_ptr  = (char *)realloc(storage_buffer, (storage_buffer_size + INITIAL_BUFFER_SIZE));
                if (temp_ptr) {
                    storage_buffer = temp_ptr;
                    storage_buffer_size += INITIAL_BUFFER_SIZE;
                    storage_buffer_size_count += 1;
                } else {
                    printf("Unable to allocate memory\n");
                }
            }
        }
    }

    if (bytes_received == -1) {
        printf("Error in receive; errno is %d\n", errno);
    }

    printf("Storage Buffer size = %d\n", bytes_to_write);

    // Check if packet_complete; indicating "\n" encountered in received data
    if (packet_complete_flag == 1) {

        packet_complete_flag = 0;

        pthread_mutex_lock(&mutex);
        //Writing to file
        int nr = write(testfile_fd, storage_buffer, bytes_to_write);
        file_size += nr; // Adding to current file length

        lseek(testfile_fd, 0, SEEK_SET); // Setting the FD to start of file

        // TEST
        char *read_buffer = NULL;

        int read_buffer_size;

        if (file_size < WRITE_BUFFER_SIZE) {
            read_buffer = (char *)malloc(file_size);
            read_buffer_size = file_size;
        } else {
            read_buffer = (char *)malloc(WRITE_BUFFER_SIZE);
            read_buffer_size = WRITE_BUFFER_SIZE;
        }

        if (read_buffer == NULL) {
            printf("Unable to allocate memory to read_buffer\n");
        }

        ssize_t bytes_read;
        int bytes_sent;

        while ((bytes_read = read(testfile_fd, read_buffer, read_buffer_size)) > 0) {
            // bytes_sent is return value from send function
            bytes_sent = send(thread_local_vars->connection_fd, read_buffer, bytes_read, 0);

            if (bytes_sent == -1) {
                printf("Error in sending to socket!\n");
            }
        }

        pthread_mutex_unlock(&mutex);
        // Reading into read_buffer
        if (bytes_read == -1)
            perror("read");

        free(read_buffer);

        // ENDTEST

        // Cleanup and reallocation of buffer to original size
        free(storage_buffer);
    }
    thread_local_vars->complete_flag = 1;

    return thread_param;

}

void alarm_handler(int signo)
{
    printf("10 seconds have passed\n");
}

int start_timer()
{
    struct itimerval delay;
    int ret;
    signal (SIGALRM, alarm_handler);
    delay.it_value.tv_sec = 5;
    delay.it_value.tv_usec = 0;
    delay.it_interval.tv_sec = 10;
    delay.it_interval.tv_usec = 0;
    ret = setitimer (ITIMER_REAL, &delay, NULL);
    if (ret) {
        perror ("setitimer");
        return -1;
    }
    return 0;
}

// Main function
int main(int argc, char **argv)
{
    // Opening Log to use syslog
    openlog(NULL, 0,  LOG_USER);

    // Starting timer
    start_timer();

    // Flag that is set when "-d"
    int daemon_flag = 0;

    if (argc == 2){
        char *daemon_argument = argv[1];
        if(strcmp(daemon_argument, "-d") != 0) {
            printf("Invalid argument to run as daemon! Expected \"-d\"\n");
        } else {
            daemon_flag = 1;
        }
    } 

    int yes = 1; // Used in the setsockopt function

    printf("Socket Programming 101!\n");

    int sig_ret_status = signal_initializer();

    if (sig_ret_status == -1) {
        printf("Error in initializing signals\n");
    }

    // Socket SETUP begin
    // Getting a socketfd
    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd == -1) {
        printf("Error creating socket! Error number is %d\n", errno);
        return -1;
    }

    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo; // Points to results

    memset(&hints, 0, sizeof(hints)); // Empty struct
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if ((status = getaddrinfo(NULL, "9000", &hints, &servinfo)) != 0) {
        printf("Error in getting addrinfo! Error number is %d\n", errno);
        return -1;
    }

    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        printf("Error in setting socket FD's options! Error number is %d\n", errno);
        return -1;
    } 

    status = bind(socketfd, servinfo->ai_addr, sizeof(struct addrinfo));
    if (status == -1) {
        printf("Error binding socket! Error number is %d\n", errno);
        return -1;
    }

    freeaddrinfo(servinfo);
    // Socket SETUP end

    // Daemon Flag check
    if (daemon_flag == 1) {
        int ret_status = daemon_create();
        if (ret_status == -1) {
            syslog(LOG_ERR, "Error Creating Daemon!");
        } else if (ret_status == 0) {
            syslog(LOG_DEBUG, "Created Daemon successfully!");
        }
    }

    status = listen(socketfd, BACKLOG);
    if (status == -1) {
        printf("Error listening to connections! Error number is %d\n", errno);
        return -1;
    }

    struct sockaddr_storage test_addr;

    socklen_t addr_size = sizeof test_addr;

    // /var/tmp/aesdsocketdata operations
    // Opening the file specified by filepath with the following flags and allowing the permissions: u=rw,g=rw,o=r
    testfile_fd = open("/var/tmp/aesdsocketdata", O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

    // In case of an error opening the file
    if (testfile_fd == -1) {
        syslog(LOG_ERR, "Error opening file with error: %d", errno);
    } 

    pthread_mutex_init(&mutex, NULL);
    // String to hold the IP address of the client; used when printing logs
    char address_string[INET_ADDRSTRLEN];

    // node_t *head = NULL;

    while(1) {

        // Continuously restarting connections in the while(1) loop
        int new_fd = accept(socketfd, (struct sockaddr *)&test_addr, &addr_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        } else {
            // Using sock_addr_in and inet_ntop function to print the IP address of the accepted client connection
            // Reference: https://stackoverflow.com/questions/2104495/extract-ip-from-connection-that-listen-and-accept-in-socket-programming-in-linux
            struct sockaddr_in *p = (struct sockaddr_in *)&test_addr;
            // TODO: Malloc for address string
            // TODO: Put syslog within thread
            syslog(LOG_DEBUG, "Accepted connection from %s", inet_ntop(AF_INET, &p->sin_addr, address_string, sizeof(address_string)));

            // pthread_t thread;
            // Malloc new node
            node_t *new_node = (node_t *)malloc(sizeof(node_t));
            new_node->thread_data.complete_flag = 0;
            new_node->thread_data.connection_fd = new_fd;

            // Creating a thread to run threadfunc with parameters thread_param
            int ret = pthread_create(&(new_node->thread_data.threadid), NULL, threadfunc, &(new_node->thread_data));
            if (ret != 0) {
                printf("Error in pthread_create with err number: %d", errno);
            } else if (ret == 0) {
                printf("Success in creating thread!\n");
            }
        }

        // close(new_fd);
        syslog(LOG_DEBUG, "Closed connection from %s", address_string);
    }

    return 0;
}
