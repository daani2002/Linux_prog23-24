#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
// Absolute source of header file
#include "/home/pep/linux_gyak/12-LP-gy/mq/mq.h"

#define PATHNAME "/home/pep/linux_gyak/12-LP-gy/mq/reg.txt"

int main(int argc, char* argv[])
{
  // Create the deamon, std I/O closed
  if(daemon(0, 0) < 0)
  {
    perror("daemon");
    return EXIT_FAILURE;
  }
  
  key_t key;
  int msgid;
  int i;
  struct persmsg msg;

  // Key not depeding on directory
  key=100;
  // Create message queue
  msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
  if(msgid < 0)
  {
    perror("msgget");
    return 1;
  }
  
  // Open log in "/var/log/syslog"
  openlog("MSG queue", LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO, "Log nyitva\n");
  
  // File descriptor for reg file
  int fd;
  // Open or create .txt file for registration data
  fd = open(PATHNAME, O_RDWR | O_CREAT);
  if(fd < 0)
  {
    perror("open");
    syslog(LOG_INFO, "Nincs regfile\n");
    return 1;
  }
  
  syslog(LOG_INFO, "Regisztracio indul\n");
  char buf[50];
  for(i = 0; i < 3; i++)
  {
    msgrcv(msgid, (struct msgbuf*)&msg, sizeof(msg) - sizeof(long),
	   MSG_TYPE, 0);
	sprintf(buf, "%d regisztracio:\n", i);
    write(fd, buf, strlen(buf));
    sprintf(buf, "nev: %s\n", msg.data.nev);
    write(fd, buf, strlen(buf));
    sprintf(buf, "neptun: %s\n", msg.data.neptun);
    write(fd, buf, strlen(buf));
  }
  // remove msg queue
  msgctl(msgid, IPC_RMID, 0);
  // close reg file
  close(fd);
  
  syslog(LOG_INFO, "Vege.\n");
  
  return 0;
}
