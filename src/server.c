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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>

/* pthread */
#include <pthread.h>

/* readline (pour choper l'input) */
#include <readline/readline.h>
#include <readline/history.h>


#include "sbot.h"


static int launch_server(void)
{
   int port = SBOT_PORT;
   int fd; /* server socket */
   struct sockaddr_in s_addr;

   /* options sur le socket */
   struct timeval timeout = {5, 0};  
   struct linger lng = {1, 5};

   /* creation du fd sur le serveur */
   if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
      err(1,"socket() :");

   s_addr.sin_family = AF_INET;
   s_addr.sin_port = htons(port);
   /* on écoute sur toutes les interfaces */
   s_addr.sin_addr.s_addr = INADDR_ANY;
   memset(&(s_addr.sin_zero), 0, 8);

   /* set les options du socket timout toussa */
   if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(int[]){1},sizeof(int)) == -1)
      err(1,"setsockopt() :");

   setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,&timeout, sizeof(struct timeval));
   setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,&timeout, sizeof(struct timeval));
   setsockopt(fd, SOL_SOCKET, SO_LINGER,&lng, sizeof(struct linger));


   /* On bind le socket sur le port et on écoute */
   if (bind(fd, (struct sockaddr *)&s_addr,sizeof(struct sockaddr)) == -1)
      err(1,"bind() :");
   if(listen(fd,5) < 0)
      err(1,"listen() :");

   return fd;
}


void * sbot_read(void *arg)
{
   int fd = (int)arg;
   char buf[SBOT_MAX];
   ssize_t n;

   for(;;)
   {
      memset(buf, 0, SBOT_MAX);
      n = recv(fd, buf, SBOT_MAX, 0);
      if(n > 0)
	 printf("\033[36m%s\033[37m", buf);
   }

   printf("%% Disconnected\n");
   pthread_exit(NULL);
}

int main(void)
{
   int c_fd; /* client fd */
   int s_fd; /* server fd */
   struct sockaddr_in c_addr;
   socklen_t sin_size;
   pthread_t tid;
   char *input;

   printf(" __   ____  _           _      ___   _   __ \n");
   printf("| _| / ___|| |__   ___ | |_   / _ \\ / | |_ |\n");
   printf("| |  \\___ \\| '_ \\ / _ \\| __| | | | || |  | |\n");
   printf("| |   ___) | |_) | (_) | |_  | |_| || |  | |\n");
   printf("| |  |____/|_.__/ \\___/ \\__|  \\___(_)_|  | |\n");
   printf("|__|                                    |__|\n");
   printf("(exit pour quitter)\n");

   s_fd = launch_server();

   /* on attend que le client se connecte */
   for(;;)
   {
      printf("%%%% Wait for client....\n");
      sin_size = sizeof(struct sockaddr_in);
      if((c_fd = accept(s_fd,(struct sockaddr *)&c_addr,&sin_size)) >= 0)
      {
	 printf("%%%% Connect from %s:%d\n", inet_ntoa(c_addr.sin_addr), htons(c_addr.sin_port));
	 break;
      }
   }

   /* on lance le thread read */
   if(pthread_create(&tid, NULL, sbot_read, (void*)c_fd) != 0)
      err(1,"pthread_create() :");

   for(;;)
   {
      input = readline(">");
      if(input)
	 send(c_fd,input,strlen(input), 0);
      if(!strcmp(input, "exit"))
	 break;
      sleep(1);
   }

   return EXIT_SUCCESS;
}


