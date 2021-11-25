#ifndef MY_HW1_LIB
#define MY_HW1_LIB

int str_length(char*);
void str_copy(char *, char *);
int is_first_substring(char*, char*); /* helper function for str_find so it is shorter and cleaner */
int str_find(char*, char*);
int get_unnamed_argument(int, int, char**, char*);
int get_named_argument(int, int, char**, char*);

#endif