#include<stdio.h>
#include<signal.h>
#include<unistd.h>
volatile bool loop = true;
void sig_handler(int signum){

  //Return type of the handler function should be void
  printf("\nInside handler function\n");
  loop = false;
}

int main(){
  signal(SIGTSTP, sig_handler); // Register signal handler
  while (loop) {    //Infinite loop
    printf("Inside main function, press ctrl + Z\n");
    sleep(1);  // Delay for 1 second
  }
  printf("Exiting\n");
  return 0;
}
