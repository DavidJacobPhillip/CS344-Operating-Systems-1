/*
Author:         Santosh Ramesh
Email:          rameshsa@oregonstate.edu
Date:           3-11-22
Description:    This program creates a network of servers and clients that communicate through sockets in order to encrpyt and decrypt information
Specifics:      This is the encoding client, sending the information to be encrypted
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>                                                                  // ssize_t
#include <sys/socket.h>                                                                 // send(),recv()
#include <netdb.h>                                                                      // gethostbyname()
#include <errno.h>

/* GLOBALS */
int resend = 0; // used soley for error checking

/* FUNCTION */
// Description: Error function used for reporting issues
void error(const char *msg) { 
    perror(msg); 
    exit(0); 
} 

/* FUNCTION */
// Description: Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname){
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname); 
    if (hostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(0); 
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

/* FUNCTION */
// Description: sends packets to the server
void packet_send(int socketFD, char* packet){
    int charsWritten;

    charsWritten = send(socketFD, packet, strlen(packet), 0);   
    if (charsWritten < 0){
        error("CLIENT: ERRORs writing to socket");
    }
    if (charsWritten < strlen(packet)){
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
}

/* FUNCTION */
// Description: sends packets to the server and implements error-checking to ensure all packets are recieved
void secure_packet_send(int socketFD, char* packet, char* buffer){
    int charsWritten;
    int charsRead;
    int checker = 1;

    strcat(packet, "@");
    // printf ("CLIENT: %s \n", packet);

    while(checker){
        resend++;
        // Write to the server
        packet_send(socketFD, packet);

        memset(buffer, '\0', sizeof(buffer));
        // Read data from the socket, leaving \0 at end
        charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
        if (charsRead < 0){
            error("CLIENT: ERROR reading from socket");
        }

        // printf("CLIENT VERIFICATION: %s \n", buffer);
        // printf("buffer: %s \n", buffer);
        if (strcmp(buffer, "@") == 0){
            // printf("S: valid \n");
            checker = 0;
        } else{
            // printf("S: invalid \n");
            checker = 0;
            // strcat(packet, "@"); // to be deleted
        } 
    }
    checker = 1;
}

/* FUNCTION */
// Description: sends the plaintext to the server with error-checking capabilities built in
void text_send(char* plaintext_file, char* keytext_file, int socketFD, char* p_packet, char* k_packet, char* e_packet, int packet_size, char* buffer){
    // Opening and reading the plaintext file
    FILE *plaintext;
    plaintext = fopen(plaintext_file, "r");

    // Opening and reading the keyfile
    FILE *keytext;
    keytext = fopen(keytext_file, "r");

    // Initializing variables
    char p_character;                                                                               // The character for the plaintext
    char k_character;                                                                               // The character for the keytext
    int cont = 1;
    int checker = 1;
    int charsRead;

    int current_size = 0;                                                                           // Holds the current size of the packet
    memset(p_packet, '\0', sizeof(p_packet)); 

    while(cont){
        // Reading the character from both the plaintext and key file and assigning it to a packet
        p_character = fgetc(plaintext);                                                             // Reading the file
        k_character = fgetc(keytext);
        
        if (p_character != EOF && p_character != '\n'){                                             // Ignoring all characters that are "EOF" or "/n"
            p_packet[current_size] = p_character;
            k_packet[current_size] = k_character;
            current_size++;
        } 

        // Sending packet over if current size == packet_size
        if (current_size == packet_size){
            current_size = 0;

            // Sending plaintext packets to server in a secure manner
            secure_packet_send(socketFD, p_packet, buffer);
            memset(p_packet, '\0', sizeof(p_packet)); 

            // Sending keytext packets to server in a secure manner
            secure_packet_send(socketFD, k_packet, buffer);
            memset(k_packet, '\0', sizeof(k_packet)); 

            // Recieving the encrypted packet in secure manner
            while (checker == 1){
                memset(e_packet, '\0', sizeof(e_packet)); 
                charsRead = recv(socketFD, e_packet, sizeof(e_packet) - 1, 0); 

                // Verifying with server that e_packet was recieved by client
                if (strstr(e_packet, "@") != NULL){
                    charsRead = send(socketFD, "@\0", 2, 0);
                    if (charsRead < 0){
                        error("ERROR writing to socket");
                    }
                    checker = 0;
                    e_packet[strlen(e_packet) - 1] = '\0';
                    printf("%s", e_packet);
                } 
                
                // Sending back the "#" character if the last character wasn't a "@"
                else{
                    charsRead = send(socketFD, "#\0", 2, 0);
                    if (charsRead < 0){
                        error("ERROR writing to socket");
                    }
                    // printf("R: invalid \n");
                    checker = 1;
                }
            }
            checker = 1;
            
        }   

        // If at the end of the file
        else if (p_character == EOF){
            // Sending plaintext packets to server in a secure manner
            secure_packet_send(socketFD, p_packet, buffer);
            memset(p_packet, '\0', sizeof(p_packet)); 

            // Sending keytext packets to server in a secure manner
            secure_packet_send(socketFD, k_packet, buffer);
            memset(k_packet, '\0', sizeof(k_packet)); 

            // Recieving the encrypted packet in secure manner
            while (checker == 1){
                memset(e_packet, '\0', sizeof(e_packet)); 
                charsRead = recv(socketFD, e_packet, sizeof(e_packet) - 1, 0); 
                // printf("Encrypted Version: %s \n", e_packet);

                // Verifying with server that e_packet was recieved by client
                if (strstr(e_packet, "@") != NULL){
                    charsRead = send(socketFD, "@\0", 2, 0);
                    if (charsRead < 0){
                        error("ERROR writing to socket");
                    }

                    checker = 0;
                    e_packet[strlen(e_packet) - 1] = '\0';
                    printf("%s", e_packet);
                } 
                
                // Sending back the "#" character if the last character wasn't a "@"
                else{
                    charsRead = send(socketFD, "#\0", 2, 0);
                    if (charsRead < 0){
                        error("ERROR writing to socket");
                    }
                    // printf("R: invalid \n");
                }
            }
            checker = 1;

            // Sending the "$" to indicate the loop is complete
            strcpy(p_packet, "$");
            packet_send(socketFD, p_packet);
            cont = 0;
        }
    }

    printf("\n");

    fclose(plaintext);
    fclose(keytext);
}

/* FUNCTION */
// Description: checks if the files are valid
int file_check(char* plaintext_file, char* keytext_file){
    int p_size = 0;
    int k_size = 0;
    char character;
    char error_msg[100];
    memset(error_msg, '\0', sizeof(error_msg));

    // Opening and reading the plaintext file
    FILE *plaintext;
    plaintext = fopen(plaintext_file, "r");

    // Opening and reading the keyfile
    FILE *keytext;
    keytext = fopen(keytext_file, "r");

    // Counting the size of the plaintext file
    while(1){
        character = fgetc(plaintext);                                                              // Reading the file
        if (character == EOF || character == '\n'){
            break;
        } 

        p_size++;

        // Error checking for invalid input
        if ((character < 65 || character > 90) && character != 32){
            strcpy(error_msg, "enc_client error: input contains bad characters \n");
            fprintf(stderr, error_msg);
            exit(1);
        } 
    }

    // Counting the size of the keytext file
    while(1){
        character = fgetc(keytext);                                                                // Reading the file
        if (character == EOF){
            break;
        }
        k_size++;
    }

    if (p_size > k_size){
        // Error if the key is too short
        strcpy(error_msg, "Error: key '");
        strcat(error_msg, keytext_file);
        strcat(error_msg, "' is too short\n");
        // printf("%s \n", error_msg);
        fprintf(stderr, error_msg);
        exit(1);
    }

    // Closing the files
    fclose(plaintext);
    fclose(keytext);
}

/* FUNCTION */
// Description: sends a signal to server to confirm identity
void server_check(int socketFD, char* buffer, char* port){
    int charsRead;
    char error_msg[100];
    memset(error_msg, '\0', sizeof(error_msg));
    memset(buffer, '\0', sizeof(buffer));

    // Sending and recieving a char to check if the server identity is real
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    packet_send(socketFD, "+");
    if(strcmp(buffer, "+") != 0){
        strcpy(error_msg, "Error: could not contact enc_server on port ");
        strcat(error_msg, port);
        strcat(error_msg, "\n");
        fprintf(stderr, error_msg);
        close(socketFD); 
        exit(2);
    }

    memset(buffer, '\0', sizeof(buffer));
}

int main(int argc, char *argv[]) {
    // Setting up variables
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[1000];
    int packet_size = 5;
    char p_packet[packet_size + 2];                                                                   // Sends over a "plaintext" packet
    char k_packet[packet_size + 2];                                                                   // Sends over a "keytext" packet
    char e_packet[packet_size + 2];                                                                   // Recieved over a "encrypted" packet
    char* address = "localhost";

    /* Setting up the files for input and the server connections */
    // Check usage & args
    if (argc < 4) { 
        fprintf(stderr,"USAGE: %s hostname port\n number of arguments: %d \n", argv[0], argc); 
        exit(0); 
    } 

    // Checking if file inputs are valid
    file_check(argv[1], argv[2]);

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    // Connect to server
    setupAddressStruct(&serverAddress, atoi(argv[3]), address);                                       // Set up the server address struct
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }
    server_check(socketFD, buffer, argv[3]);

    // Reading from files and sending it to the server after error-checking the files */
    text_send(argv[1], argv[2], socketFD, p_packet, k_packet, e_packet, packet_size, buffer);

    // Close the socket
    close(socketFD); 
    return 0;
}