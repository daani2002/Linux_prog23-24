#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <poll.h>

void child(int* pipe1_fds, int* pipe2_fds)
{  
  while(1){
  //redirect stdin to pipe1 read side
  dup2(pipe1_fds[0], STDIN_FILENO);
  //redirect stdout to pipe2 write side
  dup2(pipe2_fds[1], STDOUT_FILENO);
  
  //cat command without parameters
  //uses the redirected stdin/out, connects pipe1-pipe2
  execlp("cat", "cat", NULL);
  }
}

int main(int argc, char* argv[])
{
  int buf[64];
  int len;
  
  //file descriptors for the 2 pipes
  int pipe1_fds[2];
  int pipe2_fds[2];
  
  pipe(pipe1_fds);
  pipe(pipe2_fds);
  
  int fdix;
  struct pollfd pollset[2];

  //we poll for input from stdin and pipe2
  pollset[0].fd = STDIN_FILENO;
  pollset[0].events = POLLIN;
  pollset[1].fd = pipe2_fds[0];
  pollset[1].events = POLLIN;
    
  //create new process
  int pid = fork();
  if(pid < 0)
  {
    printf("Hiba: fork()\n");
    return -1;
  }
  //it is child
  if(pid == 0)
  {
    child(pipe1_fds, pipe2_fds);
  }
  //it is parent
  while(1){
    if(poll(pollset, 2, -1) < 0)
    {
      perror("poll");
      return EXIT_FAILURE;
    }
    
    for(fdix = 0; fdix < 2; fdix++)
    {
      if(pollset[fdix].revents & POLLHUP)
      {
        printf("A pipe%d lezarult!\n", fdix + 1);
        return EXIT_SUCCESS;
      }
      
      if(pollset[fdix].revents & (POLLERR | POLLNVAL))
      {
        printf("Hiba!\n");
        return EXIT_FAILURE;
      }
      
      if(pollset[fdix].revents & POLLIN)
      {
        len = read(pollset[fdix].fd, buf, sizeof(buf));
        if(len > 0)
        {
          //data from stdin
          if(fdix == 0)
            //send to child on pipe1
            write(pipe1_fds[1], buf, len);
            
          //data from child on pipe2
          if(fdix == 1)
            //write to stdout
            write(STDOUT_FILENO, buf, len);
        }
      }
    }
   
    
  }
  
  return 0;
}
