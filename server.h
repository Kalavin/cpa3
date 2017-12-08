#ifndef _SERVER_H_
#define _SERVER_H_

//flag enum
typedef enum bool {True, False} flag;

typedef struct bank_account {
        char account_name[100];
        float balance;
        flag in_session;
} bank_account;
bank_account bank[20];
pthread_mutex_t bank_lock;
pthread_mutex_t account_locks[20];
int num_accounts = 0;


//function declarations
void* client_session_thread(void* arg);
int open(char* acc_name); //returns int representing index in bank
int start(char* acc_name); //returns int representing index in bank
float deposit(int accountid, char* amount_str); //returns the amount deposited as a float
float withdraw(int accountid, char* amount_str); //returns the amount deposited as a float
float balance(int accountid);
void finish(int* accountid);
int claim_port(const char* port);
int main(int argc, char** argv);

#endif /*_SERVER_H_*/
