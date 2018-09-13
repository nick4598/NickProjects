#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "csapp.h"
#include "debug.h"
#include "server.h"
#include "directory.h"
#include "thread_counter.h"
#include <ctype.h>

static void terminate();

THREAD_COUNTER *thread_counter;

int main(int argc, char* argv[]) {
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    if (argc == 1 || argc == 2)
    {
        fprintf(stderr, "Please enter a port number!\n");
        exit(EXIT_FAILURE);
    }
    char* port;
    char option;
    while ((option = getopt(argc,argv, "+p:")) != -1)
    {
        switch (option)
        {
            case 'p':
            {
                port = optarg;
                for (int i = 0; port[i] != '0'; i++)
                {
                    if (isalpha(port[i]))
                    {
                        fprintf(stderr, "Please enter a valid port number!\n");
                        exit(EXIT_FAILURE);
                    }
                }

            }
        }
    }

    //struct sigaction hupHandler;
    //hupHandler.sa_handler = terminate;
    Signal(SIGHUP, terminate);
    //sigaction(SIGHUP, &hupHandler, NULL);





    // Perform required initializations of the thread counter and directory.
    thread_counter = tcnt_init();
    dir_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function bvd_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    //char client_hostname[MAXLINE], client_port[MAXLINE];
    struct sockaddr_in clientaddr;
    int listenfd = open_listenfd(port);
    if (listenfd < 0)
    {
        fprintf(stderr, "Unable to open server for listening\n");
        exit(EXIT_FAILURE);

    }

    pthread_t thread1;
    while (1)
    {
        int *connfd = malloc(sizeof(int));
        socklen_t clientlen = sizeof(struct sockaddr_storage);
        *connfd = (Accept(listenfd, (SA *) &clientaddr, &clientlen));
        //getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
          //  client_port, MAXLINE, 0);
        //bvd_client_service((void *)(&connfd));

        int success = pthread_create(&thread1, NULL, bvd_client_service,
            (void *)(connfd));
        //close(connfd);
        //sleep(2000);
        if (success != 0)
        {
            fprintf(stderr, "Unable to create thread! Error: %d", success);
        }
        else
        {
            //tcnt_incr(thread_counter);
        }
        //close(*connfd);


    }


    fprintf(stderr, "You have to finish implementing main() "
	    "before the Bavarde server will function.\n");

    terminate();
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int sig) {
    // Shut down the directory.
    // This will trigger the eventual termination of service threads.
    dir_shutdown();

    debug("Waiting for service threads to terminate...");
    tcnt_wait_for_zero(thread_counter);
    debug("All service threads terminated.");

    tcnt_fini(thread_counter);
    dir_fini();
    exit(EXIT_SUCCESS);
}
