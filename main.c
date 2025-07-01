#include "minishell.h"


extern char *external_command[200];  
char prompt[25];
extern char input_string[50];
int main() {
    system("clear");
    strcpy(prompt, "Minishell");   
    scan_input(prompt, input_string);
    return 0;
}

