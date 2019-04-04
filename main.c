#include "sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char** loadFile(char* name, int* count);

char* getWord(FILE* stream);

int main(){
    setSortThreads(10);
    int count = 0;
    char** words = loadFile("poem.txt", &count);
    printf("Loaded in words\n");
    printf("Unsorted\n");
    for(int i = 0; i < count-1; i++){
        printf("%s\n",words[i]); 
    }
    sortThreaded(words,count);
}

char* getWord(FILE* stream){
    char c;
    size_t size = 10;
    int pos = 0;
    char* str = malloc(sizeof(char)*size);
    while((c = fgetc(stream)) != EOF && !isspace(c)){
        if(pos >= size-2){
            size *= 2;
            str = realloc(str,size*sizeof(char));
        }
        str[pos++] = c;
    }
    str[pos] = '\0';
    if(pos == 0 && c == EOF){
        return NULL;
    }
    return str;
}

char** loadFile(char* name, int* count){
    FILE* f = fopen(name,"r");
    size_t size = 1000;
    int pos = 0;
    char** words = malloc(sizeof(char*)*size);
    char* word;
    while((word = getWord(f)) != NULL){
        if(pos >= size-2){
            size *= 2;
            words = realloc(words,size*sizeof(char*));
        }
        if(word[0] != '\0'){
            words[pos++] = word;
        }
    }
    *count = pos-1;
    return words;
}

