#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include    <pthread.h>
#include	<sys/socket.h>
#include	<netdb.h>

char 	dcmsg[150];
char    new_msg[2048];
// Miniature client to exercise getaddrinfo(2).
void* writing_messages(void* arg) 
{
	int sd;
	sd = *(int *)arg;
	char	prompt[128] = "How may we help you? >>";
	write( 1, prompt, sizeof(prompt));
	//writing variables
	//Gets your message and sends it to everyone, then zeros out message.
	while (1)
	{
		read( 0, new_msg, sizeof(new_msg));
        new_msg[strlen(new_msg)] = '\0';
		write(sd, new_msg, strlen(new_msg) + 1);
        if(strcmp(new_msg, "exit\n") == 0)
        {
        	strcat(dcmsg,"DISCONNECTED");
        	//write(sd,dcmsg,strlen(dcmsg));
            printf("Banking session ended.\n");
            pthread_exit(0);
            return 0;
        }
		strcpy(new_msg, "");
		bzero(new_msg, sizeof(new_msg));
	}
}

void* reading_messages(void* arg) 
{
	int sd;
	sd = *(int *)arg;
	char	prompt[128] = "How may we help you? >>";
	//reading variable
	char	got_message[50000];
	//constantly reading in messages
	while(1)
	{
		//shutting down reading function if exit is typed
        if(strcmp(new_msg, "exit\n") == 0)
        {
        	pthread_exit(0);
            return 0;
		}
		read(sd, got_message, sizeof(got_message));
		printf("%s\n", got_message);
		bzero(got_message, sizeof(got_message));
		sleep(1);
		write( 1, prompt, sizeof(prompt));
	}
}

int
repeated_connect( const char * server, struct addrinfo * rp )
{
	int 		sd;
	char		message[256];
	do {
		if ( errno = 0, (sd = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol )) == -1 )
		{
			return -1;
		}
		else if ( errno = 0, connect( sd, rp->ai_addr, rp->ai_addrlen ) == -1 )
		{
			close( sd );
			sleep( 1 );
			write( 1, message, sprintf( message, "\x1b[2;33mConnecting to server %s ...\x1b[0m\n", server ) );
		}
		else
		{
			return sd;		// connect() succeeded
		}
	} while ( errno == ECONNREFUSED );
	return -1;				// failure to conenct
}

int
main( int argc, char ** argv )
{
	int 			sd;
	int *	SDptr;
	char			message[256];
	int				ignore;
	struct addrinfo		addrinfo;
	struct addrinfo *	result;
	struct addrinfo *	rp;

	addrinfo.ai_flags = 0;			// AI_PASSIVE for bind()
	addrinfo.ai_family = AF_INET;		// IPv4 only
	addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
	addrinfo.ai_protocol = 0;		// Any protocol
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;

	if ( argc < 3 )
	{
		fprintf( stderr, "\x1b[1;31mMust specify server host name and port number on command line.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	else if ( sscanf( argv[2], "%d", &ignore ) == 0 )
	{
		fprintf( stderr, "\x1b[1;31mMust specify port number as integerr on command line.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	else if ( getaddrinfo( argv[1], argv[2], &addrinfo, &result ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed errno is %s.  File %s line %d.\x1b[0m\n", argv[1], strerror( errno ), __FILE__, __LINE__ );
		exit( 1 );
	}
	else
	{
		for ( rp = result ; rp != 0 ; rp = rp->ai_next )
		{
			if ( (sd = repeated_connect( argv[1], rp )) == -1 )
			{
				continue;		// failure
			}
			else
			{
				write( 1, message, sprintf( message,  "\x1b[1;32mSUCCESS : Connected to server %s\x1b[0m\n", argv[ 1] ) );
				break;
			}
		}
		freeaddrinfo( result );
		if ( sd == -1 )
		{
			write( 1, message, sprintf( message,  "\x1b[1;31mCould not connect to server %s errno %s\x1b[0m\n", argv[1], strerror( errno ) ) );
			return 1;
		}
		else
		{
			printf( "Welcome to JC & AW Bank on %s\n", argv[1] );
			//Creating writing and reading threads
			pthread_t write;
			pthread_t read;
			SDptr = (int *)malloc(sizeof(int));
			*SDptr = sd;
			if (pthread_create(&write, NULL, writing_messages, SDptr) != 0)
			{
				printf("\x1b[1;31mProblem creating thread\x1b[0m\n");
			}
			pthread_detach(write);
			if (pthread_create(&read, NULL, reading_messages, SDptr) != 0)
			{
				printf("\x1b[1;31mProblem creating thread\x1b[0m\n");
			}
			pthread_detach(read);
		}
        while (strcmp(dcmsg,"DISCONNECTED") != 0)
       	{
       		sleep(1);
       	}
       	close(sd);
		return 0;
	}
}
