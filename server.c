#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<pthread.h>
#include "server.h"

/*I created the thread at the end of the program, which is the client_session thread right below.
  The code is going to need to be tweaked because we need to create an array of structs to store
  the bank account info. 
*/
void * client_session_thread(void * arg)
{
	int 	SD;
	char 	request[2048];
    char* delim = " ";
    char* token;

	SD = *(int *)arg;
	free (arg);

	while ( read( SD, request, sizeof(request) ) > 0 )
	{
		printf( "server receives input:  %s\n", request );
        token = strtok(request, delim);

	}
    pthread_exit(0);
}
//I assume this will be the format of our bank account struct although do we need a flag variable as well?

int
claim_port( const char * port )
{
	struct addrinfo		addrinfo;
	struct addrinfo *	result;
	int			sd;
	char			message[256];
	int			on = 1;

	addrinfo.ai_flags = AI_PASSIVE;		// for bind()
	addrinfo.ai_family = AF_INET;		// IPv4 only
	addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
	addrinfo.ai_protocol = 0;		// Any protocol
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;
	if ( getaddrinfo( 0, port, &addrinfo, &result ) != 0 )		// want port 3000
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed errno is %s.  File %s line %d.\x1b[0m\n", port, strerror( errno ), __FILE__, __LINE__ );
		return -1;
	}
	else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
	{
		write( 1, message, sprintf( message, "socket() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
		freeaddrinfo( result );
		return -1;
	}
	else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
	{
		write( 1, message, sprintf( message, "setsockopt() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
		freeaddrinfo( result );
		close( sd );
		return -1;
	}
	else if ( bind( sd, result->ai_addr, result->ai_addrlen ) == -1 )
	{
		freeaddrinfo( result );
		close( sd );
		write( 1, message, sprintf( message, "\x1b[2;33mBinding to port %s ...\x1b[0m\n", port ) );
		return -1;
	}
	else if ( listen( sd, 100 ) == -1 )
	{
		printf( "listen() failed in file %s line %d\n", __FILE__, __LINE__ );
		close( sd );
		return 0;
	}
	else
	{
		write( 1, message, sprintf( message,  "\x1b[1;32mSUCCESS : Bind to port %s\x1b[0m\n", port ) );
		freeaddrinfo( result );		
		return sd;			// bind() succeeded;
	}
}

int
main( int argc, char ** argv )
{
	int			sd;
	char		message[256];
	socklen_t	ic;
	int			fd;
	struct sockaddr_in      senderAddr;
	char		temp;
	int			i;
	int * FDptr;
	int			limit, size;
	int			ignore;
	pthread_t 	tid;

	if ( argc < 2 )
	{
		fprintf( stderr, "\x1b[1;31mMust specify port number on command line.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	else if ( sscanf( argv[1], "%d", &ignore ) == 0 )
	{
		fprintf( stderr, "\x1b[1;31mMust specify port number as integerr on command line.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	else if ( (sd = claim_port( argv[1] )) == -1 )
	{
		write( 1, message, sprintf( message,  "\x1b[1;31mCould not bind to port %s errno %s\x1b[0m\n", "3000", strerror( errno ) ) );
		return 1;
	}
	else
	{
		ic = sizeof(senderAddr);
		while ( (fd = accept( sd, (struct sockaddr *)&senderAddr, &ic )) != -1 )
		{
			FDptr = (int *)malloc(sizeof(int));
			*FDptr = fd;
			//Creating client_session_thread at top of program
			if (pthread_create(&tid, NULL, client_session_thread, FDptr) != 0)
			{
				printf("\x1b[1;31mProblem creating thread\x1b[0m\n");
			}
			else
			{
				sleep(1);
				continue;
			}
		}
		close(sd);
		return 0;
	}
}
