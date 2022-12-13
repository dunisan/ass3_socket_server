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
#define TEXT_LENGTH 1728037

long timeOfReceivingFiles[100]; 
int indexOfReveivingFiles = 0; 

int main(){


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

    // Make socket listening; 

    int listenResult = listen(listeningSocket, 4); 
    if(listenResult == -1){
        printf("listen() failed with error code : %d", errno);
        //close the socket
        close(listeningSocket); 
    }

    printf("Waiting for incoming TCP-connection...\n");
    struct sockaddr_in clientAddress; 
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


        int totalByteReceive = 0; 
        int byteRecieved;
        //receive a message from client
        char buffer[BUFFER_SIZE]; 

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
            }else{
                totalByteReceive += byteRecieved;
                printf("%s\n",buffer);
                if(totalByteReceive == TEXT_LENGTH/2){
                    clock_gettime(CLOCK_MONOTONIC, &after);
                    timeOfReceivingFiles[indexOfReveivingFiles++] = after.tv_nsec - before.tv_nsec;
                    break;
                }
                if(byteRecieved == 17){
                    goto end;
                }
            }
        }
           
                
           
     
        printf("\n\n\n\nrecieved %d bytes\n\n\n", totalByteReceive);


       // int authentication = 4599 ^ 790;
        char authenticationMessage[4] = "4402";

        int messageLen = sizeof(authenticationMessage);

        int byteSent = send(clientSocket, authenticationMessage, messageLen, 0);
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

        //receive a message from client
         
        memset(buffer, 0, BUFFER_SIZE);
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
                printf("\n\n%d  ------- %d\n\n", totalByteReceive, byteRecieved);
                printf("%s",buffer);
                if(totalByteReceive == TEXT_LENGTH){
                    clock_gettime(CLOCK_MONOTONIC, &after);
                    timeOfReceivingFiles[indexOfReveivingFiles++] = after.tv_nsec - before.tv_nsec;;
                    break;
                }
            }
        }


    }

    end: 

        long sumOfTimes=0;
        for(int i=0;i<indexOfReveivingFiles;i++){
            printf("\ntime %d: %ld", i, timeOfReceivingFiles[i]);
            sumOfTimes +=timeOfReceivingFiles[i];
        }
        printf("\n");
        long avarageOftimes = sumOfTimes/indexOfReveivingFiles;
        printf("The Avarage time for receving all packets is %ld\n", avarageOftimes);

        

        


    close(listeningSocket);

}