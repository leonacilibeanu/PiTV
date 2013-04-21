#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include "util.h"
#include "debug.h"
#include "sock_util.h"
#include "w_epoll.h"

/* server socket file descriptor */
static int listenfd;

/* epoll file descriptor */
static int epollfd;

enum connection_state {
	STATE_DATA_RECEIVED,
	STATE_DATA_SENT,
	STATE_CONNECTION_CLOSED
};

/* structure acting as a connection handler */
struct connection {
	int sockfd;
	/* buffers used for receiving messages and then echoing them back */
	char recv_buffer[BUFSIZ];
	size_t recv_len;
	char send_buffer[BUFSIZ];
	size_t send_len;
	enum connection_state state;
        int new;
        int streamno;
};
 

typedef struct m_part {
    int fd;
    long size;
} m_part;

#define MAX_CONN 	10
#define MAX_PARTS 90
#define NAME1 "movie1"
#define NAME2 "movie2"
#define PORT 10000
#define MAX_BUF 2000
#define MIN(a,b) ((a) < (b) ? (a):(b))


struct connection *connections[MAX_CONN];
int num_conn = 0;
int f_ind;
m_part c_file[2];
int val = 0;

int send_file(int fd2, int fd1, int size) {

        char buf[MAX_BUF];
        int trans = 0, bytes;
        
        DIE(lseek(fd1, 0, SEEK_SET) < 0, "seek");
        
        while (trans < size) {
                
                bytes = read(fd1, buf, MAX_BUF);
                if (bytes < 0)
                        return -1;
                if (send(fd2, buf, bytes, 0) != bytes)
                        return -1;
                        
                trans += bytes;
        }
        
        return trans;
}

static void file_switch() {

        char name[BUFSIZ];
        int fd, i;
        struct stat buff;
        
        f_ind++;
        if (f_ind > MAX_PARTS) f_ind = 1;
        
        sprintf(name, "%s-%i.wmv", NAME1, f_ind);
        printf("%s\n", name);
        

        
        fd = open(name, O_RDONLY);
        DIE(fd < 0, "Deschidere fisier");
        DIE(fstat(fd, &buff) < 0, "Eroare stat");
        c_file[0].fd = fd;
        c_file[0].size = buff.st_size;
        
        sprintf(name, "%s-%i.wmv", NAME2, f_ind);
        printf("%s\n", name);
        

        
        fd = open(name, O_RDONLY);
        DIE(fd < 0, "Deschidere fisier");
        DIE(fstat(fd, &buff) < 0, "Eroare stat");
        c_file[1].fd = fd;
        c_file[1].size = buff.st_size;

        
        for (i = 0; i < num_conn; i++) {
                if (connections[i]->streamno == 0)
                {
                        printf("Intru pe zero\n");
                        send(connections[i]->sockfd, &(c_file[0].size), sizeof(c_file[0].size), 0);
                        send_file(connections[i]->sockfd, c_file[0].fd, c_file[0].size);
                } else
                if (connections[i]->streamno == 1)
                {
                        printf("Intru pe unu\n");
                        send(connections[i]->sockfd, &(c_file[1].size), sizeof(c_file[1].size), 0);
                        send_file(connections[i]->sockfd, c_file[1].fd, c_file[1].size);
                }
        }
}

/*
 * Initialize connection structure on given socket.
 */

static struct connection *connection_create(int sockfd)
{
	struct connection *conn = malloc(sizeof(*conn));
	DIE(conn == NULL, "malloc");

	conn->sockfd = sockfd;
        conn->new = 1;
	memset(conn->recv_buffer, 0, BUFSIZ);
	memset(conn->send_buffer, 0, BUFSIZ);
	connections[num_conn] = conn;
	num_conn++;

	return conn;
}

/*
 * Copy receive buffer to send buffer (echo).
 */

static void connection_reply(struct connection *conn) {
        
}

/*
 * Remove connection handler.
 */

static void connection_remove(struct connection *conn)
{

	int i, j;
	close(conn->sockfd);
	conn->state = STATE_CONNECTION_CLOSED;
	for (i = 0; i < num_conn; i++)
		if (connections[i] == conn)
			for (j = i; j < num_conn - 1; j++)
				connections[j] = connections[j + 1];
        num_conn--;
	free(conn);
}

/*
 * Handle a new connection request on the server socket.
 */

static void handle_new_connection(void)
{
	static int sockfd;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	struct connection *conn;
	int rc;

	/* accept new connection */
	sockfd = accept(listenfd, (SSA *) &addr, &addrlen);
	DIE(sockfd < 0, "accept");

	dlog(LOG_ERR, "Accepted connection from: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	/* instantiate new connection handler */
	conn = connection_create(sockfd);

	/* add socket to epoll */
	rc = w_epoll_add_ptr_in(epollfd, sockfd, conn);
	DIE(rc < 0, "w_epoll_add_in");
}

/*
 * Receive message on socket.
 * Store message in recv_buffer in struct connection.
 */

static enum connection_state receive_message(struct connection *conn)
{
	ssize_t bytes_recv;
	int rc;
	char abuffer[64];

	rc = get_peer_address(conn->sockfd, abuffer, 64);
	if (rc < 0) {
		goto remove_connection;
	}

	bytes_recv = recv(conn->sockfd, conn->recv_buffer, BUFSIZ, 0);
	if (bytes_recv < 0) {		/* error in communication */
		dlog(LOG_ERR, "Error in communication from: %s\n", abuffer);
		goto remove_connection;
	}
        
	if (bytes_recv == 0) {		/* connection closed */
		dlog(LOG_INFO, "Connection closed from: %s\n", abuffer);
		goto remove_connection;
	}

	dlog(LOG_DEBUG, "Received message from: %s\n", abuffer);

        printf("OK\n");
	printf("--\n%d--\n", ((int*)(conn->recv_buffer))[0]);
        conn->streamno = ((int*)(conn->recv_buffer))[0];
        

	conn->recv_len = bytes_recv;
	conn->state = STATE_DATA_RECEIVED;

	return STATE_DATA_RECEIVED;

remove_connection:
        printf("Lost connection\n");
	rc = w_epoll_remove_ptr(epollfd, conn->sockfd, conn);
	DIE(rc < 0, "w_epoll_remove_ptr");

	/* remove current connection */
	connection_remove(conn);

	return STATE_CONNECTION_CLOSED;
}


/*
 * Handle a client request on a client connection.
 */

static void handle_client_request(struct connection *conn)
{
	int rc;
	enum connection_state ret_state;

	ret_state = receive_message(conn);
	if (ret_state == STATE_CONNECTION_CLOSED)
		return;

	connection_reply(conn);

	/* add socket to epoll for out events */
	rc = w_epoll_update_ptr_inout(epollfd, conn->sockfd, conn);
	DIE(rc < 0, "w_epoll_add_ptr_inout");
}

int main(void)
{
	int rc, fd;
        struct sigevent evp = { 0 };
        timer_t id;
        char name[BUFSIZ];
        struct stat buff;

	/* init multiplexing */
	epollfd = w_epoll_create();
	DIE(epollfd < 0, "w_epoll_create");

	/* create server socket */
	listenfd = tcp_create_listener(PORT, DEFAULT_LISTEN_BACKLOG);
	DIE(listenfd < 0, "tcp_create_listener");

	rc = w_epoll_add_fd_in(epollfd, listenfd);
	DIE(rc < 0, "w_epoll_add_fd_in");

	dlog(LOG_INFO, "Server waiting for connections on port %d\n", AWS_LISTEN_PORT);
        
        evp.sigev_notify = SIGEV_THREAD;
        evp.sigev_notify_function = file_switch;
        evp.sigev_value.sival_ptr = NULL;

        struct itimerspec timer = { 3, 0, 3, 0 };


        timer_create(CLOCK_REALTIME, &evp, &id);
        timer_settime(id, 0, &timer, NULL);
        

	/* server main loop */
	while (1) {
		struct epoll_event rev;
		//struct connection *conn;

		rc = w_epoll_wait_infinite(epollfd, &rev);
		DIE(rc < 0, "w_epoll_wait_infinite");


		if (rev.data.fd == listenfd) {
			printf("New connection\n");
			if (rev.events & EPOLLIN)
				handle_new_connection();
		}
		else {
			if (rev.events & EPOLLIN) {
				dlog(LOG_DEBUG, "New message\n");
				handle_client_request(rev.data.ptr);
			}
		}
	}

	return 0;
}
