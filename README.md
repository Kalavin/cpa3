# cpa3
#Jeff and Alex

Challenges/Summary:

We didn't have any problems with the initial connection on the server side into the bank. We used claim_port to establish our socket descriptor and listen for connections. In our main we called claim_port and put in error handling to ensure that we received a port number to grab from the command line. At the end of our main we established our client_session_thread to perform all tasks once the connection was established. Here we use a while loop to read in all messages being send over from the client via the socket descriptor. This part we had some trouble with as we were originally getting a segmentation fault because we were passing a char * instead of just a char. The commands were relatively easy although we had to fool around with the mutex locks a bit to ensure that 2 of the same account sessions couldn't be open simultaneously. Jeff had the great idea to use tokens for receiving one word at a time when receiving commands with 2 statements such as deposit where we have to check the first variable sent over and then the second sent over.

The client side was relatively easy. We are constantly keeping the lines of communication for reading and writing messae available with while loops in our writing_messages and reading_messages thread. We successfully exit the threads and the program upon the exit command and we make sure to zero out our variables after every sent aor received message as to prevent any overlap. We have a prompt that will appear before reading from the command line.

PLEASE REFER TO THE TESTCASE document for instructions on running our code.
