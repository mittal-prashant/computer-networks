Name: Prashant Mittal
Entry No: 2020CSB1113

"TASK 1"
A. Open a terminal and run the server.c file using command "gcc server.c -o server".
B. Then run the server using command "./server".
C. Run the 5 clients using command "gcc client1.c -o client1".
D. Then run the client1 by using commanf "./client1".
E. Similarly for all other 4 clients changing 1 to 2,3,4,5 in file names.
F. Give input after running each client file through terminal.
G. Reversed input will be displayed by each client file.


"TASK 2"
A. Open a terminal and run the server.c file using command "gcc -pthread server.c -o server".
B. Then run the server using command "./server".
C. Enter the credentials where username = "server" and passowrd = "server".
D. Run the 5 clients using command "gcc client1.c -o client1".
E. Then run the client1 by using commanf "./client1".
F. Similarly for all other 4 clients changing 1 to 2,3,4,5 in file names.
G. Give input after running each client file through terminal.
H. Enter the type of each ticket and then it will ask for number of tickets.
I. Give number of tickets as input.
J. Enter the details again if you want to book ticket again.
K. Enter 0 if you want to close the connection.
L. Each connection is created in a different thread, server closes after 5 connections closed.
M. The tickets booked will be displayed by clients.
N. All the tickets with type, number and username is displayed in server_records.txt.