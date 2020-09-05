#include <stdlib.h>
#include "srch.h"

 int linearSearch(char arr[], int len, char ss){
     for(int i = 0 ; i < len; i++){
        if(arr[i] == ss){ 
            return i;
        }
     }
     return -1;
 }
