/*
Author:         Santosh Ramesh
Email:          rameshsa@oregonstate.edu
Date:           3-11-22
Description:    This program creates a network of servers and clients that communicate through sockets in order to encrpyt and decrypt information
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>                                                                  // ssize_t
#include <sys/socket.h>                                                                 // send(),recv()
#include <netdb.h>                                                                      // gethostbyname()
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    // Initialization of time
    srand(time(NULL));   

    // Taking in input for the number of characters
    int length = atoi(argv[1]);
    int character = 0;

    // Printing out characters to STDOUT 
    for (int i = 0; i < length; i++){
        character = (rand() % 27) + 65;

        // Changing the character to a space if it is the value "91"
        if (character == 91){
            character = 32;
        }

        printf("%c", character);
    }

    printf("\n");
    return 0;
}