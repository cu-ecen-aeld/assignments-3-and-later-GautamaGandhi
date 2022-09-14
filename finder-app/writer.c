/**
* File Name: writer.c
* File Desciption: This file contains the main function for AESD Assignment 2.
* File Author: Gautama Gandhi
* Reference: Linux man pages and Linux System Programming Chapter 2
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

// Function that displays an initial/welcome message to tell the user how to call the application
static void initial_message()
{
    printf("Writer application to write a <string> onto a <file>. Format is mentioned below.\n");
    printf("Format: ./writer <filepath> <text to be written>\n");
}


int main(int argc, char **argv)
{

    initial_message();

    // Check argc value to validate correct number of arguements
    if (argc != 3) {
        // Providing error message upon invalid invocation
        printf("Error. Invalid number of arguments. Expected 2 arguments after executable!\n");
        printf("Correct format: ./writer <filepath> <text to be written>\n");
        return EXIT_FAILURE;
    }

    // File Descriptor
    int fd;

    // Variable to store bytes written
    ssize_t nr;

    // Filepath and string to be written
    char *filepath = argv[1];
    char *string = argv[2];

    // Opening Log to use syslog
    openlog(NULL, 0,  LOG_USER);

    // Opening the file specified by filepath with the following flags and allowing the permissions: u=rw,g=rw,o=r
    fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

    // In case of an error opening the file
    if (fd == -1) {
        syslog(LOG_ERR, "Error opening file with error: %d", errno);
    } 
    
    // Writing the string into the file
    nr = write(fd, string, strlen(string));

    // Debug statement specified by the assignment document
    syslog(LOG_DEBUG, "Writing %s to %s", string, filepath);
    
    // Check if any error occured when trying to write to the file
    if (nr == -1) {
        syslog(LOG_ERR, "Error writing string \"%s\" to file with error %d", string, errno);
    } else if (nr != strlen(string)) { 
        // Error in case of partial write
        syslog(LOG_ERR, "Error writing complete string \"%s\" to file", string);
    }

    closelog();
    close(fd);

    return 0;
}

