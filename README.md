# TFTP

A tftp client written for the UNIX operating system and works with any TFTP server that implements the RFC 1350 protocol.

The compilation steps are :

1. Navigate to the folder where you downloaded the client.c file in your terminal
2. Run in terminal

    <b>gcc client.c -o tftp_client</b>
    
This should create a tftp_client file in that folder.

Now to run the application you use the following command :

    ./tftp_client IP_YOU_WANT_TO_CONNECT
  
  If now ip address is provided it takes localhost as the default ip address.
  
  You can access the help by typing help in the prompt.
  
  The commands you can use are :
  
   1. put filename :- uploads the file to the server.
   2. get filename :- downloads the file from the server.
   3. quit :- quits the TFTP shell
   4. help :- shows the help for the applicaton
