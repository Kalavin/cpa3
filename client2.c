#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>

// Miniature client to exercise getaddrinfo(2).

int
repeated_connect( const char * server, struct addrinfo * rp )
{
	int			sd;
	char			message[256];

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
	int			sd;
	char			message[256];
	char			string[512];
	char			buffer[512];
	char			prompt[] = "Enter a string>>";
	char			output[512];
	int			len;
	int			ignore;
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
		fprintf( stderr, "\x1b[1;31mMust specify port numbe as integerr on command line.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
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
			printf( "Connected to server %s\n", argv[1] );
			while ( write( 1, prompt, sizeof(prompt) ), (len = read( 0, string, sizeof(string) )) > 0 )
			{
				string[len]= '\0';
				write( sd, string, strlen( string ) + 1 );
				read( sd, buffer, sizeof(buffer) );
				sprintf( output, "Result is >%s<\n", buffer );
				write( 1, output, strlen(output) );
			}
			close( sd );
			return 0;
		}
	}
	return 0;
}
