#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<pthread.h>
#include	<malloc.h>
#include	<sys/ioctl.h>
#include	<netinet/in.h>
#include	<sys/types.h>
#include 	<sys/stat.h>
#include 	<netdb.h>
#include "server.h"


/*I created the thread at the end of the program, which is the client_session thread right below.
  The code is going to need to be tweaked because we need to create an array of structs to store
  the bank account info. 
*/
void * client_session_thread(void * arg)
{
	int 	SD;
	char 	request[128] = "";
    char* delim = " \0\n";

	SD = *(int *)arg;
	free (arg);
	pthread_detach(pthread_self());
    int size = 0;
    int accountid = -1;
	while ( (size = read( SD, request, sizeof(request) )) > 0)
	{
        printf("%d\n", size);
        char* token = strtok(request, delim);
        if (strcmp(token, "open") == 0) {
            token = strtok(NULL, delim);
            if (accountid >= 0 && accountid <= 19) {
                char errmess[] = "Error: you can not open an account while in a session.";
                write(SD, errmess, sizeof(errmess));
                continue;
            }
            int ret_val = open(token); //distinguish accounts by index in bank array
            if (ret_val == -1) { //too many accounts
                char errmess[] = "Error: not enough room in bank. \"exit\" to quit";
                write(SD, errmess, sizeof(errmess));
            } else if (ret_val == -2) { //account name too long
                char errmess[] = "Error: account name too long. Must be less than 100 characters.";
                write(SD, errmess, sizeof(errmess));
            } else if (ret_val == -3) {
                char errmess[] = "Error: account with that name already exists.";
                write(SD, errmess, sizeof(errmess));
            } else  {
                char message[] = "Account successfully opened!";
                write(SD, message, sizeof(message));
            }
        } else if (strcmp(token, "start") == 0) {
            token = strtok(NULL, delim);
            accountid = start(token);
            if (accountid == -1) {
                char errmess[] = "Error: no account with that name exists.";
                write(SD, errmess, sizeof(errmess));
            } else {
                char message[] = "Welcome back! Your session has begun successfully.";
                write(SD, message, sizeof(message));
            }
        } else if (strcmp(token, "deposit") == 0) {
            token = strtok(NULL, delim);
            if (accountid >= 0 && accountid <= 19) {
                float amount = deposit(accountid, token);
                char message[100] = "";
                sprintf(message, ">>%f<< deposited successfully.", amount);
                write(SD, message, sizeof(message));
            } else {
                char errmess[] = "Error: you are not currently in a session.";
                write(SD, errmess, sizeof(errmess));
            }
        } else if (strcmp(token, "withdraw") == 0) {
            token = strtok(NULL, delim);
            if (accountid >= 0 && accountid <= 19) {
                float amount = withdraw(accountid, token);
                if (amount == -1) { 
                    char errmess[] = "Error: You can't withdraw more than you have.";
                    write(SD, errmess, sizeof(errmess));
                    continue;
                }
                char message[100] = "";
                sprintf(message, ">>%f<< withdrawn successfully.", amount);
                write(SD, message, sizeof(message));
            } else {
                char errmess[] = "Error: you are not currently in a session.";
                write(SD, errmess, sizeof(errmess));
                continue;
            }
        } else if (strncmp(token, "balance", strlen("balance")) == 0) {
            if (accountid >= 0 && accountid <= 19) {
                float curr_bal = balance(accountid);
                char message[100] = "";
                sprintf(message, "Your current balance is >>%f<<", curr_bal);
                write(SD, message, sizeof(message));
            } else {
                char errmess[] = "Error: you are not currently in a session.";
                write(SD, errmess, sizeof(errmess));
            }
        } else if (strncmp(token, "finish", strlen("finish")) == 0) {
            if (accountid >= 0 && accountid <= 19) {
                finish(&accountid);
                char message[] = "Session closed.";
                write(SD, message, sizeof(message));
            } else {
                char* errmess = "Error: you are not currently in a session.";
                write(SD, errmess, sizeof(errmess));
            }
        } else if (strncmp(token, "exit", strlen("exit")) == 0) {
            if (accountid >= 0 && accountid <= 19) {
                finish(&accountid);
            }
            char message[] = "Thanks for using Jeff and Alex's bank. Have a nice day!";
            write(SD, message, sizeof(message));
        } else {
            char message[] = "You have entered an incorrect command! Please try again using open, start, deposit, withdraw, balance, finish or exit.";        	
            write(SD, message, sizeof(message));
        }
	}
    pthread_exit(0);
}

int open(char* acc_name) { //returns -1 for too many accounts, -2 for name too long, -3 for account with that name already exists
    pthread_mutex_lock(&bank_lock);

    if (strlen(acc_name) > 100) {
        return -2;
    } else if (num_accounts >= 20) {
        return -1;
    }
    //check if an account with that name already exists
    int i;
    for (i = 0; i < num_accounts; i++) {
        if (strcmp(bank[i].account_name, acc_name) == 0) {
           return -3;
        } 
    }
    pthread_mutex_lock(&account_locks[num_accounts]);

    int accountid = num_accounts;
    num_accounts += 1;

    pthread_mutex_unlock(&bank_lock);

    //update all the information
    strcpy(bank[accountid].account_name, acc_name);
    bank[accountid].balance = 0;
    bank[accountid].in_session = False;

    pthread_mutex_unlock(&account_locks[accountid]);

    return accountid;
}

int start(char* acc_name) {
    pthread_mutex_lock(&bank_lock);
    int i;
    int accountid = -1;
    for (i = 0; i < num_accounts; i++) {
        pthread_mutex_lock(&account_locks[i]);
        if (strcmp(bank[i].account_name, acc_name) == 0) {
            accountid = i;
            bank[accountid].in_session = True;
            break;
        }
        pthread_mutex_unlock(&account_locks[i]);
    }
    pthread_mutex_unlock(&bank_lock);
    pthread_mutex_unlock(&account_locks[i]);
    return accountid;
}

float deposit(int accountid, char* amount_str) {
    float amount = atof(amount_str);
    pthread_mutex_lock(&account_locks[accountid]);
    bank[accountid].balance += amount;
    pthread_mutex_unlock(&account_locks[accountid]);
    return amount;
}

float withdraw(int accountid, char* amount_str) {
    float amount = atof(amount_str);
    pthread_mutex_lock(&account_locks[accountid]);
    if ((bank[accountid].balance - amount) < 0) {
        pthread_mutex_unlock(&account_locks[accountid]);
        return -1;
    }
    bank[accountid].balance -= amount;
    pthread_mutex_unlock(&account_locks[accountid]);
    return amount;
}

float balance(int accountid) {
    return bank[accountid].balance;
}

void finish(int* accountid) {
    pthread_mutex_lock(&account_locks[*accountid]);
    bank[*accountid].in_session = False;
    pthread_mutex_unlock(&account_locks[*accountid]);    
    *accountid = -1;
}

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
	int * FDptr;
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
