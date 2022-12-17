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



#define SERVER_PORT 9998  // The port to connect server 
#define SERVER_IP_ADDRESS "127.0.0.1" // Te server IP adderss
#define BUFFER_SIZE 4096 // Buffer size for sending the text 
#define TEXT_LENGTH 8640185


int authentication(int sockfd);

// readFile() Read file from text save it in a strtin and returns a pointer to the text. 
char* readFile();

int main(){

  

    FILE *fp; 
    char *filename = "/home/daniel/Desktop/Computer_Networking/assignment3/text.txt";
    char *text; // pointer to the text. 
   /*
    text = readFile(); // read the file 
    if(text == NULL){
        printf("faild to read text\n");
        return -1; 
    }
*/
   
    printf("readFile() Succeeded\n");
 
  
    
    // create a socket for communicating with receiver using TCP protocol 

    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        printf("Could not create socket : %d", errno);
        return -1; 
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address)); 

    server_address.sin_family = AF_INET; // Ipv4 
    server_address.sin_port = htons(SERVER_PORT);  

    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &server_address.sin_addr);

    if(rval<=0){
        printf("inet_pton() faild"); 
        return -1; 
    }

    
    // Make a connnection to the Receiver server 
    int connectResult = connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));
    if(connectResult == -1){  // connection faild - print a message and close socket
        printf("connect() failed with error code %d\n", errno);  
        close(sock); 
        return -1; 
    }

    // If we are here - connection with receiver has established - 

    printf("Connected to server\n"); 

    //start sending the first part of the file. 

    int decision=1; // While condition is "dicision". it gives the user the option to send the file again.   

    

    while(decision==1){
        
        setsockopt(sock,IPPROTO_TCP,TCP_CONGESTION,"cubic",5); // send the file with "cubic" cc algorithm 
                           

      //sendfile(sock,text, start);
        fp = fopen(filename, "r");
        if(fp == NULL){
            perror("ERROR in reading file\n");
            exit(1);
        }
        int textLength; 

        fseek(fp, 0L, SEEK_END); 
        textLength = ftell(fp);
        fseek(fp, 0L, SEEK_SET);


        int n; 
        char data[BUFFER_SIZE] = {0};
        int byteSent=0;
        int byteToSend = textLength/2 + (BUFFER_SIZE - (textLength/2)%BUFFER_SIZE);


        send(sock, &byteToSend, sizeof(byteToSend), 0);


        while (fgets(data, BUFFER_SIZE, fp) != NULL){
            if((n = send(sock, data, sizeof(data),0))==-1){
                perror("Error in sending file"); 
            }
            byteSent += n; 
            if(byteSent == byteToSend){
                break; 
            }
        }

        printf("SEND %d and BREAK\n", byteSent);



     //   int byteSent = 0;
     //   byteSent = send(sock, text, TEXT_LENGTH/2, 0); // send the file with send()
     //   if(byteSent == -1){ // 
     //       printf("send() faild with an error %d", errno); 
     //   } else if(byteSent == 0){ // 
     //       printf("peer has closed the TCP connection prior to send(). \n"); 
     //   } else if(byteSent < TEXT_LENGTH/2){
     //       printf("sent only %d bytes from the requierd %d\n", byteSent, TEXT_LENGTH/2); 
     //   } else{
     //       printf("message was succussfuly sent, sent %d bytes\n", byteSent); 
     //   }
                
     
        //  Authentication - get a authentication message from receiver. 

        authentication(sock);
        
        // Change the CC algorithm to "reno"

        setsockopt(sock,IPPROTO_TCP,TCP_CONGESTION,"reno",4);

        memset(&server_address, 0, sizeof(server_address));
       
        //  Send the second part
       //sendfile(sock, text + TEXT_LENGTH/2 , start);

        fseek(fp, 0L, SEEK_SET+byteToSend);
        byteToSend = textLength - byteToSend; // the number of bytes of the second part of the file. 
        printf("-- byte to sen d %d\n", byteToSend);
        byteSent = 0, n=0;
        
        send(sock, &byteToSend, sizeof(byteToSend), 0);

        while (fgets(data, BUFFER_SIZE, fp) != NULL){
            if((n = send(sock, data, sizeof(data),0))==-1){
                perror("Error in sending file"); 
            }
            byteSent += n; 
           
            if(byteSent == byteToSend){
                break; 
            }     
        }

        printf("SEND %d and BREAK\n", byteSent);
      /* 
        byteSent = 0;
        byteSent = send(sock, text + (TEXT_LENGTH/2), TEXT_LENGTH/2 + 1, 0); // send the file with send()
    
        printf("%d\n",byteSent);
        if(byteSent == -1){ // 
            printf("send() faild with an error %d", errno); 
        } else if(byteSent == 0){ // 
            printf("peer has closed the TCP connection prior to send(). \n"); 
        } else if(byteSent < TEXT_LENGTH/2){
            printf("sent only %d bytes from the requierd %d\n", byteSent, TEXT_LENGTH/2); 
        } else{
            printf("message was succussfuly sent, sent %d bytes\n", byteSent); 
        }
*/

        // User dicision if to send the file again or to exit. 
        // for sending again enter 1. Any other key for exiting. 

        printf("Enter '1' for sending the file again, or enter anything else in order to exit\n"); 
        scanf("%d", &decision); // The while condition is "decision == 1"


    }

    int byteSent = send(sock, "Bye bye receiver", sizeof("Bye bye receiver"), 0);  // Send a bye message to receiver

    if(byteSent == -1){
        printf("send \"bye bye\" faild with an error %d", errno); 
    } else if(byteSent == 0){
        printf("peer has closed the TCP connection prior to send(). \n"); 
    } else if(byteSent < (int)sizeof("Bye bye receiver")){
        printf("sent only %d bytes from the requierd %d\n", byteSent, (int)sizeof("Bye bye receiver")); 
    } else{
        printf(" \"bye bye\" was succussfuly sent\n"); 
    }   

	close(sock); // finshed all. close socket. 

	return 0; 
}



 // readFile() reads a file into a char pointer.
 // return the pointer to the string. 

char* readFile(){

    FILE *ptr; // file pointer
    long textLength, readLength;
    char *text = NULL; 
	
	ptr = fopen("/home/daniel/Desktop/Computer_Networking/assignment3/text.txt","r");
	if(ptr == NULL)
        return NULL; 

    // get the length of the file using fseek() and ftell() 

    fseek(ptr, 0L, SEEK_END); 
    textLength = ftell(ptr);
    fseek(ptr, 0L, SEEK_SET);


    text = (char*)malloc((textLength + 1) * sizeof(char) ); 

    if(text == NULL)
        return NULL; 

    readLength =fread(text, sizeof(char), textLength, ptr);
    text[textLength] = '\0';

    if(textLength != readLength){
        printf("something went worng while reading\n");
        free(text);
        text = NULL;
    }
	fclose(ptr);


  
    

	return text;
}



    

    int authentication(int sockfd){
        int authenticationReply; 
        int byteRecieved  = recv(sockfd, &authenticationReply, 10, 0);

        if(byteRecieved == -1){
            printf("recv() failed withe error code : %d" , errno); 
            return -1; 
        } 
        
        printf("recieved %d bytes from server :\n", byteRecieved); 
        

      //  int authentication = atoi(authenticationReply);
        
        if(authenticationReply != (197^4599)){
            printf("authentication faild\n");
            close(sockfd);
            return -1; 
        }
   
        printf("Authentication succeeded\n");
        return 1; 
    }