/*
Author:         Santosh Ramesh
Email:          rameshsa@oregonstate.edu
Date:           1-9-21
Description:    This program takes in an input of movies as a ".csv" file and allows user to view simple organizations of the movies
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "movies.h"

/* STRUCT: movies, with the variables of "Title", "Year", "Languages", and "Rating Value" */
struct movie {
    char *title;
    int year;
    char *languages;
    float rating;
    struct movie *next;
};

/* FUNCTION: tokenizes the current line based on the struct "movie" and its variables */
// note: a lot of this code has been taken from the example resource provided in the assignment description
struct movie *create_movie(char *currLine) {
    struct movie *current_movie = malloc(sizeof(struct movie));

    // For use with strtok_r
    char *saveptr;

    // The first token is the title
    char *token = strtok_r(currLine, ",", &saveptr);
    current_movie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(current_movie->title, token);

    // The next token is the year
    token = strtok_r(NULL, ",", &saveptr);
    current_movie->year = atoi(token);

    // The next token is the languages
    // token = strtok_r(NULL, "[", &saveptr);              // removes the "["
    //getc(saveptr);
    token = strtok_r(NULL, ",", &saveptr);
    current_movie->languages = calloc(strlen(token) + 1, sizeof(char));
    strcpy(current_movie->languages, token);

    // The last token is the rating
    token = strtok_r(NULL, "\n", &saveptr);
    current_movie->rating = strtod(token, NULL);

    // Set the next node to NULL in the newly created movie entry
    current_movie->next = NULL;

    return current_movie;
}

/* FUNCTION: creates a linked list of movies from the file input */
// note: a lot of this code has been taken from the example resource provided in the assignment description
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

/* FUNCTION: prints out a single movie struct's variable(s) */
// note: a lot of this code has been taken from the example resource provided in the assignment description
void print_movies(struct movie* current){
  //printf("%s, %i, %s, %s\n", current->title,
            //    current->year,
            //    current->languages,
            //    current->rating);
    printf("%s \n", current->languages);
}

/* FUNCTION: prints out all of the movie structs */
// note: a lot of this code has been taken from the example resource provided in the assignment description
void print_movie_list(struct movie *list){
    while (list != NULL)
    {
        print_movies(list);
        list = list->next;
    }
}

/* FUNCTION: has the user choose a mode of operation */
void choice_select(char *choice){
    char error;
    int check = 1;

    while(check){
        // Printing options to select from
        printf("1. Show movies released in the specified year \n");
        printf("2. Show highest rated movie for each year \n");
        printf("3. Show the title and year of release of all movies in a specific language \n");
        printf("4. Exit from the program \n \n");    

        // Prompting user to select a choice
        printf("Enter a choice from 1 to 4: ");
        scanf("%c", choice);

        // Checking to see if the first character input is valid
        if (*choice == '1' ||  *choice == '2' || *choice == '3' || *choice == '4'){
            // Checking to see if more than one character exists in the buffer
            scanf("%c", &error);                  // if there's a 2nd character the input is automatically invalid
            if (error == '\n'){                    
                check = 0;
            }
        } 
        if (check == 1){
            printf("You entered an incorrect choice. Try again. \n \n");
            error = '\n';
        }
    }
}

// FUNCTION: option_one prints all of the movies in a given year
void option_one(struct movie *list){
    int year;
    int number = 0;
    struct movie *current = list;

    // obtains user input for a specific year
    printf("Enter the year for which you want to see movies: ");
    scanf("%i", &year);
    while((getchar()) != '\n');

    // traverses through list of movies
    while (current != NULL){
        if(year == current->year){
            // only prints titles matching the user input
            printf("%s \n", current->title);
            number = 1;
        }
        current = current->next;
    }

    // prints a special message if no movies were found for a year
    if(number == 0){
        printf("No data about movies released in the year %i", year);
    }

    printf("\n \n");
}

// FUNCTION: option_two prints the highest ranking movie per year
void option_two(struct movie *list){
    int year = 2021;
    int number = 0;
    float rating = 0.0;
    struct movie *current = list;
    struct movie *highest = NULL;

    // looping through each year
    while(year >= 1900){
        // finding the highest ranking movie per year by traversing the list and storing the list element
        while (current != NULL){
            if(year == current->year){
                // replacing the highest rated movie if one exists
                if(current->rating > rating){
                    rating = current->rating;
                    highest = current;
                }
            }
            current = current->next;
        }

        if (highest != NULL){
            printf("%i %.1f %s \n", highest->year, highest->rating, highest->title);
        }

        // setting up the variables to search through the next year
        current = list;
        highest = NULL;
        rating = 0.0;
        year = year - 1;
    }

    printf("\n");
}

// FUNCTION: option_three prints all the movies that are available in a specific language
void option_three(struct movie *list){
    char language[20];
    char *existence;
    int number = 0;
    struct movie *current = list;

    // obtains user input for a specific language
    printf("Enter the language for which you want to see movies: ");
    scanf("%s", language);
    while((getchar()) != '\n');

    // traverses through list of movies
    while (current != NULL){
        existence = strstr(current->languages, language);
        if (existence != NULL){
            printf("%i %s \n", current->year, current->title);
            number = 1;
        }

        current = current->next;
        existence = NULL;
    }

    // prints a special message if no movies were found for a year
    if(number == 0){
        printf("No data about movies released in %s", language);
    }

    printf("\n \n");
}

// FUNCTION: list_freer frees all allocated memory
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

int main(int argc, char *argv[]) {
    // Variable Declarations
    char choice;
    int number = -1;
    int run = 1;
    struct movie *list;

    // Opening The "movies.csv" file
    if (argc < 2){
        // If no file has been inputed
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./movies movie_sample_1.csv\n");
        return 1;
    } else{
        // Loading file if correctly inputed
        list = process_file(argv[1], &number);
        printf("Processed file %s and parsed data for %i movies \n \n", argv[1], number);
    }
    
    while(run){
        choice_select(&choice);

        // switch-statement that chooses which option to use based on user input
        switch(choice){
            case '1':
                option_one(list);
                break;
            case '2':
                option_two(list);
                break;
            case '3':
                option_three(list);
                break;
            case '4':
                run = 0;
        }
    }    

    list_freer(list);

    return 0;
}