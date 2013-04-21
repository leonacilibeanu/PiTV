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
	
      int portno, serverfd, pid, streamno, f1;
      char buf[BUFLEN], buf1[BUFLEN];
      struct sockaddr_in serv_addr;
      int i, k, n, fdmax;
      long dim;
      char *args[4];
      fd_set read_fds;
      fd_set tmp_fds;
      
      int f_num = 0;
      char *name[2] = {"test1.wav", "test2.wav"};
      
      if (argc < 3) {
        fprintf(stderr,"Usage : %s addr_server port\n", argv[0]);
        exit(1);
      }
         
	    portno = atoi(argv[2]);
      streamno = atoi(argv[3]);
      if (streamno != 0 && streamno != 1) 
        streamno = 0;
      
      pid = fork();
      
      if (pid < 0)
        exit(1);
      
      if (pid == 0) {
      
        args[0] = "cp";
        args[1] = "Intro.wmv";
        args[2] = "test1.wav";
        args[3] = NULL;
        execvp("cp", args);
        exit(1);
      
      }
      
      pid = fork();
      
      if (pid < 0)
        exit(1);
      
      if (pid == 0) {
      
        args[0] = "cp";
        args[1] = "Intro.wmv";
        args[2] = "test2.wav";
        args[3] = NULL;
        execvp("cp", args);
        exit(1);
      
      }

	    //conectare la server
	    serverfd = socket(AF_INET, SOCK_STREAM, 0);
         	if (serverfd < 0) 
            	exit(1);
	    memset((char *) &serv_addr, 0, sizeof(serv_addr));
	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_port = htons(portno);
	    inet_aton(argv[1], &serv_addr.sin_addr);
	    if (connect(serverfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
		    exit(1);
        
      //set FD_SET
      FD_ZERO(&read_fds);
      FD_ZERO(&tmp_fds);
      FD_SET(0, &read_fds);
      FD_SET(serverfd, &read_fds);
      fdmax = serverfd;
      
      send(serverfd, &streamno, sizeof(int), 0);
      
      f_num = (f_num + 1) % 2;
      
      pid = fork();
      
      if (pid < 0)
        exit(1);
        
      tmp_fds = read_fds;
      
      if (pid == 0) {
      
        args[0] = "/bin/bash";
        args[1] = "play.sh";
        args[2] = "&";
        args[3] = NULL;
        execvp("/bin/bash", args);
        exit(1);
      
      }
	    
	    while (1) {
          
	      //receive file from server
        if ((k = recv(serverfd, buf, sizeof(long), 0)) <= 0) {
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
