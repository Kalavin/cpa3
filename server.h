#ifndef _SERVER_H_
#define _SERVER_H_

//flag enum
typedef enum bool {True, False} flag;

typedef struct bank_account {
        char account_name[100];
        float balance;
        flag in_session;
} bank_account;


//function declarations
void* client_session_thread(void* arg);
bank_account* open(char* acc_name);
int claim_port(const char* port);
int main(int argc, char** argv);

#endif /*_SERVER_H_*/
