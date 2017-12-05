#ifndef _SERVER_H_
#define _SERVER_H_

//flag enum
typedef enum bool {True, False} flag;

typedef struct bank_account {
        char account_name[100];
        float balance;
        flag in_session;
} bank_account;

#endif /*_SERVER_H_*/
