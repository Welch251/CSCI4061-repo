#include <stdio.h>

main(){
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  while(1){
    printf("@>");
    char str[MAX_LINE];
    char *success;
    success = fgets(str, MAX_LINE, stdio);
    if(success == NULL){
      printf("End of input");
      break;
    }
    
  }
}
