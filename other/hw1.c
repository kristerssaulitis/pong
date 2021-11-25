#include <stdio.h>
#include "hw1.h"

int str_length(char* mystring){
  int len = 0;
  while(mystring[len]!='\0') len++;
  return len;
}

void str_copy(char * source, char * destination){
  int pos=0;
  while(pos<256 && source[pos]!='\0'){ /* We don't want to run forever if there is no \0 so end after 256 characters are copied */
    destination[pos] = source[pos];
    pos++;
  }
  destination[pos] = '\0';
}

int is_first_substring(char* needle, char* offset_haystack){
  /* This function checks if needle matches the first N characters of offset_haystack */
  int i = 0;
  while(needle[i] == offset_haystack[i]){
    if(needle[i] == '\0') return 1; /* Check if both strings ended at the same place and all characters matched */
    i++;
  }
  if(needle[i] == '\0') return 1; /* Check if needle ended with all characters matching up to that point */
  return 0;
}

int str_find(char* needle, char* haystack){
  int len_needle, len_haystack = 0;
  int candidate_location = 0;

  len_needle = str_length(needle);

  if(len_needle == 0) {
    printf("ERROR: Tried looking for empty string! Undefined behaviour!");
    return -1;
  }

  len_haystack = str_length(haystack);

  if(len_needle>len_haystack){
    /* needle longer than haystack - not possible to find */
    return -1;
  }

  while(candidate_location+len_needle<=len_haystack){
    /* check if needle is at candidate location */
    if(is_first_substring(needle, haystack+candidate_location*sizeof(char))) return candidate_location;
    candidate_location++;
  }
  return -1;
}

int get_unnamed_argument(int index, int argc, char** argv, char* result){
  int i = 0;
  int unnamed_i = -1;

  for(i=0;i<argc;i++){ /* startimng from first argument check if unnamed */
    if(str_length(argv[i]) == 2 && str_find("--", argv[i]) == 0) return -1; /* check if argument is exactly -- */
    if(str_find("=", argv[i])==-1){  /* otherwise if it contains = it is named and not counted */
      unnamed_i++;
      if(unnamed_i == index){
        str_copy(argv[i], result);
        return str_length(result);
      }
    }
  }

  return -1;
}

int get_named_argument(int index, int argc, char** argv, char* result){
  int i = 0;
  int named_i = -1;

  for(i=0;i<argc;i++){ /* startimng from first argument check if named */
    if(str_length(argv[i]) == 2 && str_find("--", argv[i]) == 0) return -1; /* check if argument is exactly -- */
    if(str_find("=", argv[i])>=0){  /* otherwise if it does not contain = it is unnamed and not counted */
      named_i++;
      if(named_i == index){
        str_copy(argv[i], result);
        return str_length(result);
      }
    }
  }

  return -1;
}
