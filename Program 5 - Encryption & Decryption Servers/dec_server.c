/*
Author:         Santosh Ramesh
Email:          rameshsa@oregonstate.edu
Date:           3-11-22
Description:    This program creates a network of servers and clients that communicate through sockets in order to encrpyt and decrypt information
Specifics:      This is the decoding server, that recieves encrypted information and decodes and sends it back to client
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>                                                                  // ssize_t
#include <sys/socket.h>                                                                 // send(),recv()
#include <netdb.h>                                                                      // gethostbyname()
#include <sys/types.h>                                                                  // pid_t
#include <unistd.h>                                                                     // fork
#include <sys/wait.h>

/* GLOBALS */
int resend = 0; // used soley for error checking

/* FUNCTION */
// Description: Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(1);
} 

/* FUNCTION */
// Description: Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
    memset((char*) address, '\0', sizeof(*address));                                    // Clear out the address struct
    address->sin_family = AF_INET;                                                      // The address should be network capable
    address->sin_port = htons(portNumber);                                              // Store the port number
    address->sin_addr.s_addr = INADDR_ANY;                                              // Allow a client at any address to connect to this server
}

/* FUNCTION */
// Description: Encrypts the file message based on the key
// Note: not used on the "dec_server"
void encryption(char* plaintext, char* keytext, char* encrypted){
    int p_char = 0;
    int k_char = 0;
    int e_char = 0;
    
    for (int i = 0; i < strlen(plaintext); i++){
        // Converting plaintext's characters
        p_char = plaintext[i];
        if (p_char == 32){
            p_char = 26;
        } else{
            p_char = p_char - 65;
        }

        // Converting keytext's characters
        k_char = keytext[i];
        if (k_char == 32){
            k_char = 26;
        } else{
            k_char = k_char - 65;
        }

        // Converting encrypted's characters
        e_char = p_char + k_char;
        e_char = e_char % 27;

        if (e_char < 26){
            e_char = e_char + 65;
        } else{
            e_char = 32;
        }
        encrypted[i] = e_char;
    }
}

/* FUNCTION */
// Description: Decrypts the file message based on the key
// Note: not used on the "enc_server"
void decryption(char* plaintext, char* keytext, char* decrypted){
    int p_char = 0;
    int k_char = 0;
    int d_char = 0;
    
    for (int i = 0; i < strlen(plaintext); i++){
        // Converting plaintext's characters
        p_char = plaintext[i];
        if (p_char == 32){
            p_char = 26;
        } else{
            p_char = p_char - 65;
        }

        // Converting keytext's characters
        k_char = keytext[i];
        if (k_char == 32){
            k_char = 26;
        } else{
            k_char = k_char - 65;
        }

        // Converting decrypted's characters
        d_char = p_char - k_char;

        // Making d_char positive if negative
        if (d_char < 0){
            d_char = d_char + 27;
        }
        if (d_char < 26){
            d_char = d_char + 65;
        } else{
            d_char = 32;
        }
        decrypted[i] = d_char;
    }
}

/* FUNCTION */
// Description: sends packets to the client
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
// Description: sends packets to the client and implements error-checking to ensure all packets are recieved
void secure_packet_send(int socketFD, char* packet, char* buffer){
    int charsWritten;
    int charsRead;
    int checker = 1;

    // Adding the ending token "@" to packet
    if (resend != 1){
        strcat(packet, "@");
    }
    
    while(checker){
        resend++;
        // Write to the server
        // printf("PACKET: %s \n", packet);
        packet_send(socketFD, packet);

        
        memset(buffer, '\0', sizeof(buffer));
        // Read data from the socket, leaving \0 at end
        charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
        if (charsRead < 0){
            error("SERVER: ERROR reading from socket");
        }

        // printf("SERVER VERIFICATION: %s \n", buffer);
        if (strcmp(buffer, "@") == 0){
            // printf("recieved vertification on server \n");
            checker = 0;
        }
        else{
            // printf("didn't recieve vertification on server \n");
            checker = 1;
            strcat(packet, "@");
        } 
    }
    checker = 1;
}

int main(int argc, char *argv[]){
    // Initializing general variables
    int connectionSocket, charsRead;
    int packet_size = 5;
    int cont;
    int keybit = 0;                                                                                 // Used to determine whether packet is for the key or for the plaintext
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t spawnpid = -5;
	int spawn_status = 10;

    // Initializing variables to hold the different messages
    char buffer[1024];
    char packet[packet_size + 2];
    char p_packet[packet_size + 2];                                                                   // Sends over a "plaintext" packet
    char k_packet[packet_size + 2];                                                                   // Sends over a "keytext" packet
    char d_packet[packet_size + 2];                                                                   // Recieved over a "encrypted" packet
    memset(buffer, '\0', sizeof(buffer));

    // Check usage & args
    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    } 
    
    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5); 

    
    // Accept a connection, blocking if one is not available until one connects
    while(1){
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0){
            error("ERROR on accept");
        }

        // Forking a child process
        spawnpid = fork();
	    switch (spawnpid){
            // Forking error
            case -1:
                // Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well
                perror("fork() failed!");
                exit(1);
                break;

            // Forking successful; child creates to communicate with client
            case 0:
                cont = 1;

                // Sending a bit to client to confirm the client's identity
                memset(packet, '\0', sizeof(packet));
                packet_send(connectionSocket, "-");
                charsRead = recv(connectionSocket, packet, (packet_size + 1), 0); 
                if (strcmp(packet, "-") != 0){
                    cont = -1;
                }

                while(cont == 1){
                    // Setting the packet to be read to all zeros
                    memset(packet, '\0', sizeof(packet));

                    // Read the client's message from the socket
                    charsRead = recv(connectionSocket, packet, (packet_size + 1), 0); 
                    if (charsRead < 0){
                        error("ERROR reading from socket");
                    }

                    // Checking if last character is a "$" to indicate end of plaintext
                    if (strcmp(packet, "$") == 0){
                        cont = 0;                                                                     // Close the connection socket for this client
                    }

                    // Checking if all of the packet has been sent by looking for the "@" character
                    else if (strstr(packet, "@") != NULL){
                        // Adding packet to the string "plaintext"
                        if (keybit == 0){
                            packet[strlen(packet) - 1] = '\0';
                            memset(p_packet, '\0', sizeof(p_packet)); 
                            strcpy(p_packet, packet);
                            keybit = 1;
                        } 

                        // Adding packet to the string "keytext"
                        else{
                            packet[strlen(packet) - 1] = '\0';
                            memset(k_packet, '\0', sizeof(k_packet)); 
                            strcpy(k_packet, packet);
                            keybit = 0;
                        }

                        // Verifying with client that packet was recieved by server
                        charsRead = send(connectionSocket, "@\0", 2, 0);
                        if (charsRead < 0){
                            error("ERROR writing to socket");
                        }

                        // Sending back an "encrypted" packet once the "keytext" is recieved
                        if (keybit == 0){
                            memset(d_packet, '\0', sizeof(d_packet)); 
                            decryption(p_packet, k_packet, d_packet);
                            secure_packet_send(connectionSocket, d_packet, buffer);
                        }
                    } 
                    // Sending back the "#" character if the last character wasn't a "@"
                    else{
                        charsRead = send(connectionSocket, "#\0", 2, 0);
                        if (charsRead < 0){
                            error("ERROR writing to socket");
                        }
                        keybit = 0;
                    }
                }

                // Closing the connection
                close(connectionSocket);
                exit(0);
            
            // Parent
            default:
                spawnpid = waitpid(spawnpid, &spawn_status, WNOHANG);
        }
    }
    
    // Close the listening socket
    close(listenSocket); 
    return 0;
}
