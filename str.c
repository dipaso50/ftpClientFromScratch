
#include <stdlib.h>
#include <stdio.h>
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
