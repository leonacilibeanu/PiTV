#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFLEN	2000
#define MIN(a,b) ((a) < (b) ? (a):(b))

int main(int argc, char *argv[])
{
	
      int sockfd, newsockfd, portno, cont = 1, serverfd, pid;
      char nume[50], buf[BUFLEN], buf1[BUFLEN];
      struct sockaddr_in serv_addr, cli_addr, my_addr;
      int n, i, j, k, dim_fis, ok;
      long dim;
      char *args[3];
      
      int f, f1, f2;
      int fdmax;
      int f_num = 0;
      char *name[2] = {"test1.wav", "test2.wav"};
      
      if (argc < 2) {
        fprintf(stderr,"Usage : %s addr_server port\n", argv[0]);
        exit(1);
      }
         
	    portno = atoi(argv[2]);

	    //conectare la server
	    serverfd = socket(AF_INET, SOCK_STREAM, 0);
         	if (serverfd < 0) 
            	error("Nu s-a putut deschide soketul");
	    memset((char *) &serv_addr, 0, sizeof(serv_addr));
	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_port = htons(atoi(argv[2]));
	    inet_aton(argv[1], &serv_addr.sin_addr);
	    if (connect(serverfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
		    error("Eroare la conectare");
		
	   /* if ((k = recv(serverfd, buf, BUFLEN, 0)) <= 0)
		      printf("S-a inchis serverul vietii!!!!\n");

		  i = 0;
		  
		  printf("Got %d bytes and Got dim %ld\n", k, ((long *) buf)[0]);

      //write initial file		
		  dim = ((long *) buf)[0];

		  f1 = open(name[f_num], O_CREAT | O_TRUNC | O_WRONLY, 0666);
		  if (f1 < 0)
		    exit(1);
		  
		  while (i < dim) {
		    k = recv(serverfd, buf1, MIN(BUFLEN, dim - i), 0);
        printf("Got %d and total is %d and min is %i\n", k, i + k, MIN(BUFLEN, dim - i));
		    if (k <= 0)
		      exit(1);
				
        
			  n = write(f1, buf1, k);
			  if (n < 0)
				  exit(1);
				  
				i += k;
		  }
			
		  close(f1);*/
      f_num = (f_num + 1) % 2;
      
      pid = fork();
      
      if (pid < 0)
        exit(1);
      
      if (pid == 0) {
      
        printf("forked\n");
        args[0] = "/bin/bash";
        args[1] = "play.sh";
        args[2] = NULL;
        execvp("/bin/bash", args);
        printf("err\n");
        exit(1);
      
      }
	    
	    while (1) {
	    
	      //receive file from server
        if ((k = recv(serverfd, buf, sizeof(long), 0)) <= 0) {
		      printf("S-a inchis serverul vietii!!!!\n");
          exit(1);
        }
          
		    dim = ((long *) buf)[0];
        printf("Loop: Got %d bytes and Got dim %ld\n", k, ((long *) buf)[0]);

		    f1 = open(name[f_num], O_CREAT | O_TRUNC | O_WRONLY, 0666);
		    if (f1 < 0)
		      exit(1);
		    
        i = 0;
        
		    while (i < dim) {
		      k = recv(serverfd, buf1, BUFLEN, 0);
          
		      if (k <= 0)
		        exit(1);
				
			    n = write(f1, buf1, k);
          
			    if (n < 0)
				    exit(1);
				    
				  i += k;
		    }
      
        printf("done\n");
        close(f1);
        f_num = (f_num + 1) % 2;
    }
      close(serverfd);
	
	return 0;
}
