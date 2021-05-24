#include "ApoyoTareas.h"
#include "string.h"

#define RED "\e[1;31m"
#define BLUE "\e[1;34m"
#define MAG "\e[1;35m"
#define RESET "\e[0m"

// 'r', 'b', 'm', '0'
void color_print(char* str, char* color){

    
    if(strcmp(color, "r")==0){
        printf(RED "%s" RESET, str);
    }
    

}


int main(){

    color_print("Hola que tal", "r");    

}