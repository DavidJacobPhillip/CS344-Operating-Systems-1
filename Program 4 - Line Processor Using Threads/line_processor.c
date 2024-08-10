/*
Author:         Santosh Ramehs
Email:          rameshsa@oregonstate.edu
Date:           2-20-22
Description:    This program takes in input and processes it through a series of 3 threads, using the producer-consumer approach
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h> 
#include <sys/wait.h> 
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>


/* CONSTANTS */
#define MAX_INPUT 1050                                                      // Max size of input line (with space for adding newline characters)
#define MAX_LINES 50                                                        // Max number of lines to be inputted before "STOP" is reached

/* GLOBALS */
// Creating a temporary buffer to be used for the input itself
char buffer[MAX_INPUT];                                                     // Double char array of size MAX_INPUT by MAX_LINES
int current_line = 0;                                                       // Tracks the current line 

// Buffer 1
char buffer_1[MAX_LINES][MAX_INPUT];                                        // Buffer 1, shared resource between input thread and line-seperator thread
int count_1 = 0;                                                            // Number of items in the buffer
int prod_idx_1 = 0;                                                         // Index where the input thread will put the next item
int con_idx_1 = 0;                                                          // Index where the line-seperator thread will pick up the next item
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;                        // Initialize the mutex for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;                           // Initialize the condition variable for buffer 1
int stop = 0;                                                               // Pauses input if the stop is found (to be deleted)

// Buffer 2
char buffer_2[MAX_LINES][MAX_INPUT];                                        // Buffer 2, shared resource between line-seperator thread and plus-sign thread
int count_2 = 0;                                                            // Number of items in the buffer
int prod_idx_2 = 0;                                                         // Index where the line-seperator thread will put the next item
int con_idx_2 = 0;                                                          // Index where the plus-sign thread will pick up the next item
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;                        // Initialize the mutex for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;                           // Initialize the condition variable for buffer 2

// Buffer 3
char buffer_3[MAX_LINES][MAX_INPUT];                                        // Buffer 3, shared resource between plus-sign thread and output thread
int count_3 = 0;                                                            // Number of items in the buffer
int prod_idx_3 = 0;                                                         // Index where the line-seperator thread will put the next item
int con_idx_3 = 0;                                                          // Index where the output thread will pick up the next item
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;                        // Initialize the mutex for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;                           // Initialize the condition variable for buffer 3
int full_output = 0;                                                        // Tracks output amount
char output_buffer[100];                                                    // Holds the output to be printed

/* FUNCTIONS */
// Description: Thread #1: Takes in the input from standard in and stores it into a buffer array
void *input(void *args){
  while(current_line != MAX_LINES){
    memset(buffer,'\0', MAX_INPUT);                                           // Initializing array to have all null characters
    if(stop == 0){
      // Scanning input & placing it into the static array
      scanf("%[^\n]s", buffer);                                             // Specific format specifier derived from online resources  
      while ((getchar()) != '\n');
      fflush(stdout);

      // If "STOP" is found
      if (strcmp(buffer, "STOP") == 0){
        stop = 1;
      } 
    }
    pthread_mutex_lock(&mutex_1);                                           // Lock the mutex before putting the item in the buffer
    strcpy(buffer_1[prod_idx_1], buffer);                                     // Put the item in the buffer
    prod_idx_1 = prod_idx_1 + 1;                                             // Increment the index where the next item will be put.
    count_1++;                        
    pthread_cond_signal(&full_1);                                            // Signal to the consumer that the buffer is no longer empty
    pthread_mutex_unlock(&mutex_1);                                          // Unlock the mutex
    current_line++;
  }
}

// Description: Thread #2: replaces all the newline characters with a space
void *line_seperator(void *args){
  char current_buff[MAX_INPUT];
  // sleep(2);

  for (int i = 0; i < MAX_LINES; i ++){
    // Retrieving from buffer_1
    pthread_mutex_lock(&mutex_1);
    while (count_1 == 0){
      pthread_cond_wait(&full_1, &mutex_1);                                   // Buffer is empty. Wait for the producer to signal that the buffer has data
    }
    strcpy(current_buff, buffer_1[con_idx_1]);
    con_idx_1 = con_idx_1 + 1;                                                // Increment the index from which the item will be picked up
    count_1--;
    pthread_mutex_unlock(&mutex_1);                                           // Unlock the mutex

    // Creating a string to represent a space to be added at the end of each line (which is where the newline character would have been)
    char space[2] = " ";
    strcat(current_buff, space);

    // Placing into buffer_2
    pthread_mutex_lock(&mutex_2);                                             // Lock the mutex before putting the item in the buffer          
    strcpy(buffer_2[prod_idx_2], current_buff);                               // Put the item in the buffer             
    prod_idx_2 = prod_idx_2 + 1;                                              // Increment the index where the next item will be put.
    count_2++;
    pthread_cond_signal(&full_2);                                             // Signal to the consumer that the buffer is no longer empty
    pthread_mutex_unlock(&mutex_2);                                           // Unlock the mutex
  }
}

// Description: Thread #3: Replaces all "++" with "^" for each array input recieved
void *plus_sign(void *args){
  char current_buff[MAX_INPUT];

  for (int i = 0; i < MAX_LINES; i ++){
    // Retrieving from buffer_2
    pthread_mutex_lock(&mutex_2);
    while (count_2 == 0){
      pthread_cond_wait(&full_2, &mutex_2);                                   // Buffer is empty. Wait for the producer to signal that the buffer has data
    }
    strcpy(current_buff, buffer_2[con_idx_2]);
    con_idx_2 = con_idx_2 + 1;                                                // Increment the index from which the item will be picked up
    count_2--;
    pthread_mutex_unlock(&mutex_2);                                           // Unlock the mutex

    // sleep(4);
    // Creating temporary arrays to do plus-sign processing 
    char recieve[MAX_INPUT];
    strcpy(recieve, current_buff);
    char temporary[2048] = "";
    char adder[2];
    adder[1] = '\0';

    // Looping through the input to find the "$$"
    for(int i = 0; i < strlen(recieve); i++){
      if(i + 1 < strlen(recieve)){

        // Copying over a character to temporary array if not "$$"
        if(recieve[i] != '+' || recieve[i+1] != '+'){
          adder[0] = recieve[i];
          strcat(temporary, adder);
        } 
        
        // Copying the "^" if "++"
        else{
          strcat(temporary, "^");
          i = i + 1;
        }
      } 
      
      // edge case for last character
      else{
        adder[0] = recieve[i];
        strcat(temporary, adder);
      }
    }

    strcpy(current_buff, temporary);

    // Placing into buffer_3
    pthread_mutex_lock(&mutex_3);                                             // Lock the mutex before putting the item in the buffer          
    strcpy(buffer_3[prod_idx_3], current_buff);                               // Put the item in the buffer             
    prod_idx_3 = prod_idx_3 + 1;                                              // Increment the index where the next item will be put.
    count_3++;
    pthread_cond_signal(&full_3);                                             // Signal to the consumer that the buffer is no longer empty
    pthread_mutex_unlock(&mutex_3);                                           // Unlock the mutex
  }
}

// Thread #4: Output
void *output(void *args){
  char current_buff[MAX_INPUT];

  for (int i = 0; i < MAX_LINES; i ++){

    // Retrieving from buffer_3
    pthread_mutex_lock(&mutex_3);
    while (count_3 == 0){
      pthread_cond_wait(&full_3, &mutex_3);                                   // Buffer is empty. Wait for the producer to signal that the buffer has data
    }
    strcpy(current_buff, buffer_3[con_idx_3]);
    con_idx_3 = con_idx_3 + 1;                                                // Increment the index from which the item will be picked up
    count_3--;
    pthread_mutex_unlock(&mutex_3);                                           // Unlock the mutex

    // sleep(6);
    // Creating temporary arrays to do output processing 
    char recieve[MAX_INPUT];
    strcpy(recieve, current_buff);
    char adder[2];
    adder[1] = '\0';

    // Only looking at lines where there are values
    if(strlen(recieve) > 1){
      for (int i = 0; i < strlen(recieve); i++){
        // Printing if 80 characters are in buffer
        if(strlen(output_buffer) == 80){
          printf("%s\n", output_buffer);
          fflush(stdout);
          memset(output_buffer, '\0', 80);
          int full_output = 0;
          i--;                                                                // Decrementing because otherwise a character is skipped
        } 
        
        // Adding to output_buffer if not reached 80 characters yet
        else{
          adder[0] = recieve[i];
          strcat(output_buffer, adder);
          full_output++;
        }
      }
    }
  }
}

int main(){
  pthread_t input_t, line_seperator_t, plus_sign_t, output_t;

  // Create the threads
  pthread_create(&input_t, NULL, input, NULL);
  pthread_create(&line_seperator_t, NULL, line_seperator, NULL);
  pthread_create(&plus_sign_t, NULL, plus_sign, NULL);
  pthread_create(&output_t, NULL, output, NULL);

  // Wait for the threads to terminate
  pthread_join(input_t, NULL);
  pthread_join(line_seperator_t, NULL);
  pthread_join(plus_sign_t, NULL);
  pthread_join(output_t, NULL);
  
  return 0;
}