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
#include <time.h>

#define SERVER_PORT 9998
#define SERVER_IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 4092
#define TEXT_LENGTH 8640185

long timeOfReceivingFiles[100]; 
int indexOfReveivingFiles = 0; 

void printToFile(char *buffer);

int main(){

    // open a socket for the receiver
    int listeningSocket = -1; 
    listeningSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if(listeningSocket == -1){
        printf("Could not create socket : %d", errno);
        return -1; 
    }

    int enableReuse = 1; 
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)); 
    if(ret<0){
        printf("setsocketpot() failed with error code : %d" , errno); 
        return -1; 
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY; // any IP at this port 
    
    
    int bindResult = bind(listeningSocket, (struct sockaddr *)&server_address, sizeof(server_address)); 
    if(bindResult == -1){
        printf("bind() failed with error code : %d\n" , errno); 
        //close socket
        close(listeningSocket); 
        return -1; 
    }

    

    int listenResult = listen(listeningSocket, 4); 
    if(listenResult == -1){
        printf("listen() failed with error code : %d", errno);
        //close the socket
        close(listeningSocket); 
    }

    printf("Waiting for incoming TCP-connection...\n");

    struct sockaddr_in clientAddress; // A new socket for the tcp connection with client 
    socklen_t clientAddressLen = sizeof(clientAddress); 
    

    memset(&clientAddress, 0, sizeof(clientAddress)); 
    clientAddressLen = sizeof(clientAddress); 
    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if(clientSocket == -1){
        printf("listen() failed with error code : %d" , errno);
        close(listeningSocket); 
        return -1;
    }

    printf("A new client connection accpted\n");
    
    while(1){

       

        int totalByteReceive = 0; // sum the total byte that received, so we know wen to stop to listen
        int byteRecieved; // byte received in each recv() call. 

        //receive the first file part from client

        char buffer[BUFFER_SIZE]; 

        while(1){
            memset(buffer, 0, BUFFER_SIZE);
                
            struct timespec before, after; // calculate the time that takes to receive every part. 
            clock_gettime(CLOCK_MONOTONIC, &before); 

            if((byteRecieved = recv(clientSocket, buffer, BUFFER_SIZE, 0)) < 0){
                printf("recv failed with error code : %d", errno);
                //close socket
                close(listeningSocket); 
                close(clientSocket);
                return -1;
            }else{
                totalByteReceive += byteRecieved;
                
                // The sender sends in the first time exactly half of the text.   
                if(totalByteReceive == TEXT_LENGTH/2){ 
                    clock_gettime(CLOCK_MONOTONIC, &after);
                    timeOfReceivingFiles[indexOfReveivingFiles++] = after.tv_nsec - before.tv_nsec;
                    break;
                }

                if(byteRecieved == 17){ // We got the exit message from the sender. go to the end. 
                    printf("%s\n",buffer);
                    goto end;
                }
            }
        }
            
        printf("Received succussfully part 1\n");    

        // Autentication with the sender. send the xor of 4599 and 0197. 
        int authenticationMessage = 4599 ^ 197;

        int messageLen = sizeof(authenticationMessage);

        int byteSent = send(clientSocket, &authenticationMessage, messageLen, 0);
        if(byteSent == -1){
            printf("send() failed with error code : %d ", errno);
            close(listeningSocket);
            close(clientSocket);
            return -1; 
        } else if(byteSent == 0){
            printf("peer has closed the TCP connection prior to send(). \n"); 
        } else if(byteSent < messageLen){
            printf("sent only %d bytes from the requierd %d", byteSent, messageLen); 
        } else{
            printf("authentication message was succussfuly sent\n"); 
        }
       
        //receive the second part from the client
        

        while(1){
            memset(buffer, 0, BUFFER_SIZE);

            struct timespec before, after; 
            clock_gettime(CLOCK_MONOTONIC, &before); 
            
            if((byteRecieved = recv(clientSocket, buffer, BUFFER_SIZE, 0)) < 0){
                printf("recv failed with error code : %d", errno);
                //close socket
                close(listeningSocket); 
                close(clientSocket);
                return -1;
            }
            else{
                totalByteReceive += byteRecieved;
               

                if(totalByteReceive == TEXT_LENGTH){
                    clock_gettime(CLOCK_MONOTONIC, &after); // finshed recv the second part. save time
                    timeOfReceivingFiles[indexOfReveivingFiles++] = after.tv_nsec - before.tv_nsec;
                    break;
                }
            }
        }

        printf("Received succussfully part 1\n");  
       

    }

    end: 

        long sumOfTimes=0; // sum up all the times. 
        for(int i=0;i<indexOfReveivingFiles;i++){
            if(i%2==0)
                printf("\n\nsend number %d:\n",i/2+1);
            printf("\ntime %d: %ld nano seconds", i, timeOfReceivingFiles[i]);
            sumOfTimes +=timeOfReceivingFiles[i];
        }
        printf("\n");
        long avarageOftimes = sumOfTimes/indexOfReveivingFiles; // calculate th avarge time
        printf("\nThe Avarage time for receving all packets is %ld\n", avarageOftimes);

        

        


    close(listeningSocket);
}

