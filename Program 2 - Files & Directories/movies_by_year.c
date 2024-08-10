/*
Author:         Santosh Ramesh
Email:          rameshsa@oregonstate.edu
Date:           1-22-21
Description:    This program looks through file directories, processes movies, and creates new files based on movie release year
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>

#include "movies_by_year.h"

/* STRUCT */
// Description: movies, with the variables of "Title", "Year", "Languages", and "Rating Value"
// Note: code from assignment #1
struct movie {
    char *title;
    int year;
    char *languages;
    float rating;
    struct movie *next;
};

/* FUNCTION */
// Description: list_freer frees all allocated memory
// Note: code from assignment #1
void list_freer(struct movie *list){
    
    struct movie *current = list;
    struct movie *next = current;

    while (current != NULL){
        next = current->next;
        free(current->title);
        free(current->languages);
        free(current);
        current = next;
    }
}

/* FUNCTION */
// Description: tokenizes the current line based on the struct "movie" and its variables
// Note: code from assignment #1
struct movie *create_movie(char *currLine){
    struct movie *current_movie = malloc(sizeof(struct movie));

    // For use with strtok_r
    char *saveptr;

    // The first token is the title
    char *token = strtok_r(currLine, ",", &saveptr);
    current_movie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(current_movie->title, token);
    // printf("title: %s   ", current_movie->title);

    // The next token is the year
    token = strtok_r(NULL, ",", &saveptr);
    current_movie->year = atoi(token);
    // printf("title: %i   ", current_movie->year);

    // The next token is the languages
    token = strtok_r(NULL, ",", &saveptr);
    current_movie->languages = calloc(strlen(token) + 1, sizeof(char));
    strcpy(current_movie->languages, token);
    // printf("title: %s   ", current_movie->languages);

    // The last token is the rating
    token = strtok_r(NULL, "\n", &saveptr);
    current_movie->rating = strtod(token, NULL);
    // printf("title: %f   \n", current_movie->rating);

    // Set the next node to NULL in the newly created movie entry
    current_movie->next = NULL;

    return current_movie;
}

/* FUNCTION */
// Description: creates a linked list of movies from the file input
// Note: code from assignment #1
struct movie *process_file(char *file, int *number){
    // Open the specified file for reading only
    FILE *movie_list = fopen(file, "r");

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;

    // Read the file line by line
    while ((nread = getline(&line, &len, movie_list)) != -1)
    {
        // Get a new movie node corresponding to the current line
        struct movie *newNode = create_movie(line);
        *number = *number + 1;

        // Is this the first node in the linked list?
        if (head == NULL)
        {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
    }

    free(line);
    fclose(movie_list);
    return head;
}

/* FUNCTION */
// Description: : prints out a single movie struct's variable(s)
// Note: code from assignment #1
void print_movies(struct movie* current){
  printf("%s, %i, %s, %f\n", current->title,
               current->year,
               current->languages,
               current->rating);
}

/* FUNCTION */
// Description: prints out all of the movie structs
// Note: code from assignment #1
void print_movie_list(struct movie *list){
    while (list != NULL)
    {
        print_movies(list);
        list = list->next;
    }
}

/* FUNCTION */
// Description: Gets the initial input to determine whether to explore files or exit program
int first_input_selector(){
    int selection = 1;

    while(selection){
        // Printing first set of selection options
        printf("1. Select file to process \n");
        printf("2. Exit the program \n \n");
        printf("Enter a choice 1 or 2: ");

        // Scanning input to choose from
        scanf("%i", &selection);

        // Determining input validity
        if(selection != 1 && selection != 2){
            printf("Error: please select either 1 or 2. Try again. \n \n");
            selection = 1;
        } else{
            printf(" \n");
            return selection;
        }
    }

    return 0;
}

/* FUNCTION */
// Description: Gets the second input to determine whether to find smallest, largest, or specific file
int second_input_selector(){
    int selection = 1;

    while(selection){
        // Printing first set of selection options
        printf("Which file you want to process? \n");
        printf("Enter 1 to pick the largest file \n");
        printf("Enter 2 to pick the smallest file \n");
        printf("Enter 3 to specify the name of a file \n \n");
        printf("Enter a choice from 1 to 3: ");

        // Scanning input to choose from
        scanf("%i", &selection);

        // Determining input validity
        if(selection != 1 && selection != 2 && selection != 3){
            printf("Error: please select a choice from 1 to 3. Try again. \n \n");
            selection = 1;
        } else{
            return selection;
        }
    }

    return 0;
}

/* FUNCTION */
// Description: Creates a random directory
void random_name(char *file_name){
    // Randomly generating a number
    int rand_int = rand() % 100000;
    char rand_str[10];
    sprintf(rand_str, "%d", rand_int);

    // Creating string for ONID
    char onid[30] = "rameshsa.movies.";
    
    // Assigning file name to passed in argument
    strcpy(file_name, onid);
    strcat(file_name, rand_str);

    // Creating directory based on name
    int create = mkdir(file_name, 0750);
}

/* FUNCTION */
// Description: splits the movie titles by year and places them into seperate files based on year
// Note: code from assignment #1
void year_split(struct movie *list, char *file_name){
    int year = 2021;
    char current_file[20];
    // current_file = file_name;
    char str_year[5];
    char year_file[40];
    int number = 0;
    struct movie *current = list;
    FILE *fp;
    
    // Looping through each year
    while(year >= 1900){
        // Finding the highest ranking movie per year by traversing the list and storing the list element
        while (current != NULL){
            if(year == current->year){
                // Printing within a year within the directory provided
                strcpy(year_file, file_name);
                strcat(year_file, "/");
                sprintf(str_year, "%d", year);
                strcat(year_file, str_year);
                strcat(year_file, ".txt");

                // Opening file and placing title within 
                fp = fopen(year_file, "a");
                fputs(current->title, fp);
                fputs("\n", fp);
                chmod(year_file, 0640);
                fclose(fp);
            }

            current = current->next;
        }

        // Setting up the variables to search through the next year
        current = list;
        year = year - 1;
    }

    printf("\n");
}

/* FUNCTION */
// Description: Picks the largest file
void option_one(){
    long long int file_size = 0;
    char file_process[30];                       // will hold the file name to be processed
    char file_name[30];
    int number = -1;

    // Opening file
    // References Week #3 Exploration #5 Code

    DIR* curr_dir = opendir(".");
    struct dirent *dir_info;
    struct stat dir_stat;

    // Finding Largest File
    while((dir_info = readdir(curr_dir)) != NULL){
        if(strstr(dir_info->d_name,  ".csv") && strncmp("movies_", dir_info->d_name, strlen("movies_")) == 0){
            if (stat(dir_info->d_name, &dir_stat) == 0){
                if(dir_stat.st_size > file_size){

                    // Storing largest file name
                    strcpy(file_process, dir_info->d_name);
                    file_size = dir_stat.st_size;
                }
            }
        }
    }

    // Getting a random file name
    random_name(file_name);

    printf("Now processing the chosen file named %s \n", file_process);
    printf("Created directory with name %s \n \n", file_name);

    // Creating new files within directory for each
    struct movie *list = process_file(file_process, &number);
    year_split(list, file_name);
    list_freer(list);
    closedir(curr_dir);
}

/* FUNCTION */
// Description: Picks the smallest file
void option_two(){
    long long int file_size = LLONG_MAX;
    char file_process[1024];                       // will hold the file name to be processed
    char file_name[30];
    int number = -1;

    // Opening file
    // References Week #3 Exploration #5 Code
    DIR* curr_dir = opendir(".");
    struct dirent *dir_info;
    struct stat dir_stat;

    // Finding Smallest File
    while((dir_info = readdir(curr_dir)) != NULL){
        if(strstr(dir_info->d_name,  ".csv") && strncmp("movies_", dir_info->d_name, strlen("movies_")) == 0){
            if (stat(dir_info->d_name, &dir_stat) == 0){
                if(dir_stat.st_size < file_size){

                    // Storing smallest file name
                    strcpy(file_process, dir_info->d_name);
                    file_size = dir_stat.st_size;
                }
            }
        }
    }

    // Getting a random file name
    random_name(file_name);

    printf("Now processing the chosen file named %s \n", file_process);
    printf("Created directory with name %s \n \n", file_name);

    // Creating new files within directory for each
    struct movie *list = process_file(file_process, &number);
    year_split(list, file_name);
    list_freer(list);
    closedir(curr_dir);
}

/* FUNCTION */
// Description: Picks a specific file based on user input
void option_three(){
    long long int file_size = LLONG_MAX;
    char file_process[1024];                       // will hold the file name to be processed
    char file_name[30];
    int found = 0;
    int number = -1;

    printf("Enter the complete file name: ");
    scanf("%s", file_process);

    // Opening file
    // References Week #3 Exploration #5 Code
    DIR* curr_dir = opendir(".");
    struct dirent *dir_info;
    struct stat dir_stat;

    // Finding Specific File
    while((dir_info = readdir(curr_dir)) != NULL){
        if(strncmp(file_process, dir_info->d_name, strlen("movies_")) == 0){
            found = 1;
        }
    }

    if(found){
        // Getting a random file name
        random_name(file_name);

        printf("Now processing the chosen file named %s \n", file_process);
        printf("Created directory with name %s \n \n", file_name);

        // Creating new files within directory for each
        strcpy(file_process, "movies_1.csv");
        printf("string: %s \n", file_process);
        struct movie *list = process_file(file_process, &number);
        year_split(list, file_name);
        list_freer(list);
        closedir(curr_dir);
    } else{
        printf("The file %s was not found. Try again. \n \n", file_process);
        second_main();
    }
}

/* FUNCTION */
// Description: this is where the user determines what they want to do with the file search
void second_main(){
    int second_input = second_input_selector();

    // switch-statement that determines which program functionality to use
    switch(second_input){
        case 1:
            option_one();
            break;
        case 2:
            option_two();
            break;
        case 3:
            option_three();
            break;
    }
}

int main(){
    srand(time(NULL));
    int run = 1;

    // Repeats program until user wants to exist
    while(run){
        // Determines if user wants to (1) browse files or (2) exit program
        int first_input = first_input_selector();

        if (first_input == 1){
            // Determines if user wants to search for (1) largest (2) smallest or (3) specific file
            second_main();
        } else{
            // Exits program
            run = 0;
        }
    }
    return 0;
}