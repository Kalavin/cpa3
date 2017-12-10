# cpa3
Alex Weiner and Jeff Cave Readme
Files included: server.h, server.c, client.c, makefile

To run program:
Step 1: Open 2 or more windows and type "make" in every window you plan on using for the server and for the various clients.
Step 2: In the window you would like to run the server type ./server 6800
Step 2(cont): Where I wrote 6800 you can type any port number over 3000
Step 3: Now in your other windows type ./client clamshell.rutgers.edu 6800
Step 3(cont): Where I wrote 6800, just make sure you use the same port you used for the server
Step 4: Enjoy the bank.

Summary/Difficulties: Connecting the Server to the Client was easy enough. We had some issues connecting the threads in the client to the threads in the server. We had to test some different ways of reading and writing to fix it. Once we solved that problem Jeff had a great idea to use tokens to accept our commands from the client to the server. This way we could check the first word and see which command it was before dealing with the second word which could have been a number value, account name, or blank.
