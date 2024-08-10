#ifndef MOVIES_BY_YEAR_H
#define MOVIES_BY_YEAR_H

// Structures
struct movie;

// Functions
void list_freer(struct movie *list);
struct movie *create_movie(char *currLine);
struct movie *process_file(char *file, int *number);
void print_movies(struct movie* current);
void print_movie_list(struct movie *list);
int first_input_selector();
int second_input_selector();
void random_name(char *file_name);
void year_split(struct movie *list, char *file_name);
void option_one();
void option_two();
void option_three();
void second_main();

#endif