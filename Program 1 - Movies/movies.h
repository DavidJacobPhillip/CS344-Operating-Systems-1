#ifndef MOVIES_H
#define MOVIES_H

// Structures
struct movie;

// Functions
struct movie *create_movie(char *currLine);
struct movie *process_file(char *file, int *number);
void print_movies(struct movie* current);
void print_movie_list(struct movie *list);
void choice_select(char *choice);
void option_one(struct movie *list);
void option_two(struct movie *list);
void option_three(struct movie *list);
void list_freer(struct movie *list);

#endif