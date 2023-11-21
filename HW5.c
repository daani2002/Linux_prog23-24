/* tcpkliens.c
 * 
 * Egyszerű TCP kliens példa. A program kapcsolódik a paraméterként
 * kapott szerverhez és elküldi a bemenetén kapott szöveget.
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

int main(int argc, char* argv[])
{
  struct addrinfo hints;
  struct addrinfo* res;
  int err;
  int csock;
  char buf[1024];
  int len;
  
  // Poll-t használunk a standard bemenetre és kliens socket-re
  struct pollfd fds[2];
    
  if(argc != 3)
  {
    printf("Használat: %s <szerver><port>\n", argv[0]);
    return 1;
  }

  /* kitöltjük a hints struktúrát */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  /* végezzük el a névfeloldást */
  err = getaddrinfo(argv[1], argv[2], &hints, &res);
  if(err != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return -1;
  }
  if(res == NULL)
  {
    return -1;
  }
  
  /* létrehozzuk a kliens socketet */
  csock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(csock < 0)
  {
    perror("socket");
    return -1;
  }
    
  /* Kapcsolodunk a szerverhez. */
  if(connect(csock, res->ai_addr, res->ai_addrlen) < 0)
  {
    perror("connect");
    return -1;
  }
  
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = csock;
  fds[1].events = POLLIN;
  
  int pollres = 0;
  /* az STDIN_FILENO-n érkező adatokat elküldjük a socketen keresztül , a socketen érkező adatokat kiírjuk STDOUT_FILENO-ra*/
  while((pollres=poll(fds, 2, -1)) > -1)
  {
    // Bemeneten érkezik adat, továbbítjuk a szervernek
    if(fds[0].revents & POLLIN)
    {
        if((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
            send(csock, buf, len, 0);
    }
    // Szervertől érkezik adat, kiírjuk STDOUT-ra
    if(fds[1].revents & POLLIN)
    {
        if((len = recv(csock, buf, sizeof(buf), 0)) > 0)
            write(STDOUT_FILENO, buf, len);
        if(len == 0)
            break;
    }
   
  }
  if(pollres < 0)
  {
    perror("poll");
    return -1;
  }
  
  
  /* lezárjuk a szerver socketet */
  close(csock);

  /* szabadítsuk fel a láncolt listát */
  freeaddrinfo(res);

  return 0;
}
