#include <stdio.h> 
#include <stdlib.h> 
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include "server.h"


#define SERVER_PORT 9998
#define SERVER_IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 4092

void sendfile(int sockfd, FILE *fp);

int main(){

    // reading the text file we created 


    char *text; // pointer to the text. holds an address. 
   
    text = readFile();
    int TEXT_LENGTH  = strlen(text); // the length of the text file
    printf("text length is %d\n", TEXT_LENGTH);
    int decision=1; 
    
    // create a socket for communicating with receiver using TCP protocol 
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        printf("Could not create socket : %d", errno);
        return -1; 
    }

    // ************* explain ***************// 
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address)); 

    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(SERVER_PORT); 

    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &server_address.sin_addr);

    if(rval<=0){
        printf("inet_pton() faild"); 
        return -1; 
    }

    
    // Make a connnection to the Receiver server 
    int connectResult = connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));
    if(connectResult == -1){
        printf("connect() failed with error code %d\n", errno); 
        //clean up the socket 
        close(sock); 
        return -1; 
    }
    // If pathd until here the connection has established
    printf("Connected to server\n"); 

    // Send the text to the reciever. 

 // put a whiile loop


    while(decision==1){

        setsockopt(sock,IPPROTO_TCP,TCP_CONGESTION,"cubic",5);

        int byteSent = send(sock, text, TEXT_LENGTH/2, 0); 

        if(byteSent == -1){
            printf("send() faild with an error %d", errno); 
        } else if(byteSent == 0){
            printf("peer has closed the TCP connection prior to send(). \n"); 
        } else if(byteSent < 1000000){
            printf("sent only %d bytes from the requierd %d\n", byteSent, 1000000); 
        } else{
            printf("message was succussfuly sent\n"); 
        }


        //  check for authentication 
        
        printf("waiting for receiver\n");

        char authenticationReply[10]; 
        int byteRecieved  = recv(sock, authenticationReply, 10, 0);

        if(byteRecieved == -1){
            printf("recv() failed withe error code : %d" , errno); 
        } else if(byteRecieved == 0){
            printf("peer has closed the TCP connection prior to recv() \n"); 
        } else{
            printf("recieved %d bytes from server : %s\n", byteRecieved, authenticationReply); 
        }

        int authintication = atoi(authenticationReply);
        printf("%d\n", authintication);
        if(authintication == 4402){
            printf("connection is authenticated\n");
        }
        else{
            printf("authentication faild\n");
            close(sock);
            return -1; 
        }

        // Change CC algorithm

        setsockopt(sock,IPPROTO_TCP,TCP_CONGESTION,"reno",4);

        //  Send the second part

//        sendfile(sock, fp);
        byteSent = send(sock, text + (TEXT_LENGTH/2), TEXT_LENGTH/2 + 1, 0); // check if starts where it ends

        if(byteSent == -1){
            printf("send() faild with an error %d", errno); 
        } else if(byteSent == 0){
            printf("peer has closed the TCP connection prior to send(). \n"); 
        } else if(byteSent < TEXT_LENGTH){
            printf("sent only %d bytes from the requierd %d\n", byteSent, TEXT_LENGTH); 
        } else{
            printf("message was succussfuly sent\n"); 
        }

        // User dicision

        printf("Enter '1' for sending the file again, or enter anything else in order to exit\n"); 
        scanf("%d", &decision);


    }
    setsockopt(sock,IPPROTO_TCP,TCP_CONGESTION,"cubic",5);

   // char exitMessage[20] = "Bye bye receiver";
    int byteSent = send(sock, "Bye bye receiver", sizeof("Bye bye receiver"), 0); 

    if(byteSent == -1){
        printf("send() faild with an error %d", errno); 
    } else if(byteSent == 0){
        printf("peer has closed the TCP connection prior to send(). \n"); 
    } else if(byteSent < (int)sizeof("Bye bye receiver")){
        printf("sent only %d bytes from the requierd %d\n", byteSent, (int)sizeof("Bye bye receiver")); 
    } else{
        printf("message was succussfuly sent\n"); 
    }   

	close(sock);

	return 0; 
}