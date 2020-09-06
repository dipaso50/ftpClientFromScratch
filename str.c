
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "str.h"

char* substr(const char *src, int m, int n){
    int len = n - m;
         
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
         
    for (int i = m; i < n && (*(src + i) != '\0'); i++)
    {
         *dest = *(src + i);
          dest++;
    }
                 
    *dest = '\0';
                    
    return dest - len;
}


int startwith(const char* a, const char*b){
    return strncmp(a, b , strlen(b)) == 0 ? 1 : 0;
}
