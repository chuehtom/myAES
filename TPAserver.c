# include <stdlib.h>
# include <stdio.h>
# include <string.h>    
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h> 
# include "myTPA.h"
# include "myAESstorage.h" 

int main(void)
{
	int server_socket, storage_socket, client_socket, c ,read_size ,server_loop = 1 ,socket_loop = 1, file_count;
	struct sockaddr_in TPAserver, Storageserver,client;
	char client_message[500],token[33],storage_message[100],reply[32],de_message[100];
	char *type,*command,*user_name,*password,*filename,*encryptedfilename,*decryptedfilename,*key,*salt,*num;

	//Create socket for client
	server_socket = socket(AF_INET , SOCK_STREAM , 0);
	if (server_socket == -1){
		printf("Could not create socket");
	}
	     
	//Prepare the sockaddr_in structure
	TPAserver.sin_family = AF_INET;
	TPAserver.sin_addr.s_addr = INADDR_ANY;
	TPAserver.sin_port = htons( 1231 );
	     
	//Bind
	if( bind(server_socket,(struct sockaddr *)&TPAserver , sizeof(TPAserver)) < 0){
		//print the error message
		perror("Error, failed to bind");
		return 1;
	}
	puts("Server create successfully");

	//Listen to max of 5 clients
	listen(server_socket , 5);
	c = sizeof(struct sockaddr_in);
     
	myTPA_load_account();
	puts("Load account list successfully");
	while(server_loop){//
		//Accept and incoming connection
		puts("Waiting for incoming connections...");
		  
		//accept connection from an incoming client
		client_socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_socket < 0){
			perror("Error, failed to accept");
			return 1;
		}
		puts("Connection between TPAserver and client accepted");
		    
	    	//Receive a message from client
		while(socket_loop){//check if the user is authenticated or not
	 		memset(client_message,0,sizeof(client_message));//clear buffer
			if((read_size = recv(client_socket , client_message , sizeof(client_message) , 0)) > 0 ){
				//printf("in:%s\n",client_message);
				//split string into smaller parts
				char *copy = malloc(sizeof(client_message));
				memset(copy,0,sizeof(copy));
				strcpy(copy,client_message);
				type = strsep(&copy,",");
				user_name = strsep(&copy,",");
				printf("User name:%s\n",user_name);
				memset(client_message,0,sizeof(client_message));
				if(!strcmp(type,"1")){
					password = strsep(&copy,",");
					printf("Password:%s\n",password);
					free(copy);
					if(myTPA_authentication(user_name,password,token)){
						printf("Token generated by TPA:%s\n",token);

						//create socket for new connection
						storage_socket = socket(AF_INET , SOCK_STREAM , 0);
						if (storage_socket == -1){
							printf("Could not create socket");
						}

						//setup info about Storageserver
						Storageserver.sin_addr.s_addr = inet_addr("127.0.0.1");
						Storageserver.sin_family = AF_INET;
						Storageserver.sin_port = htons( 1233 );

						//Connect to Storageserver
						if (connect(storage_socket , (struct sockaddr *)&Storageserver , sizeof(Storageserver)) < 0)
							perror("Error, Established connection with Storageserver");
						else 
							puts("Established connection with Storageserver");

						//send message to Storageserver to register the user
						memset(storage_message,0,sizeof(storage_message));
						strcat(storage_message,"0");
						strcat(storage_message,",");
						strcat(storage_message,user_name);
						strcat(storage_message,",");
						token[32] = ',';
						strcat(storage_message,token);
						write(storage_socket , storage_message, strlen(storage_message));
		
						memset(reply,0,sizeof(reply));
						memcpy(reply,token,32);
						//wait for respond from Storageserver
						memset(storage_message,0,sizeof(storage_message));
						while((read_size = recv(storage_socket , storage_message , sizeof(storage_message) , 0)) < 0);
						if(!strcmp(storage_message,"1")){//register successed
							printf("User %s registered successfully with token ",user_name);
							for(int i=0;i<32;i++)
								printf("%c",token[i]);
							puts("");
						}else//register failed
							puts("Error, failed to register");
						
						write(client_socket , reply, sizeof(reply));//send respond to client
					}else{
						memset(reply,0,sizeof(reply));
						write(client_socket ,reply, sizeof(reply));//send respond to client, if token is empty means user is not authenticated
					}
					
					for(int i=0;i<50;i++)printf("-");
					puts("\n");
				}else if(!strcmp(type,"2")){
					filename = strsep(&copy,",");
					//printf("filename:%s\n",filename);
					encryptedfilename = strsep(&copy,",");
					//printf("encryptedfilename:%s\n",encryptedfilename);
					decryptedfilename = strsep(&copy,",");
					//printf("decryptedfilename:%s\n",decryptedfilename);
					key = strsep(&copy,",");
					//printf("key:%s\n",key);
					salt = strsep(&copy,",");
					//printf("salt:%s\n",salt);
					num = strsep(&copy,",");
					//printf("num:%s\n",num);
					file_count = atoi(num);
					myAESStorage_set_root(myAESStorage_insert_node(myAESStorage_get_root(),filename,encryptedfilename,decryptedfilename,key,key,key,KEY_SIZE,salt,file_count));
					printf("Key %s of file %s stored successfully\n",key,filename);
					free(copy);
					write(client_socket ,"1",1);
					myAESStorage_print_storage();
					puts("");
					
				}else if(!strcmp(type,"3")){
					filename = strsep(&copy,",");
					struct myAES_decryptblock *myAESCrypt = myAESStorage_search_node(filename);
					memset(client_message,0,sizeof(client_message));
					if(myAESCrypt != NULL){
						file_count = myAESCrypt->file_count;
						sprintf(num,"%d",myAESCrypt->file_count);
						strcat(client_message,myAESCrypt->key);
						strcat(client_message,",");
						strcat(client_message,myAESCrypt->salt);
						strcat(client_message,",");
						strcat(client_message,num);
						strcat(client_message,",");
						
					}else{
	 					printf("Error, cant find key of file %s\n",filename);
					}
					printf("Key %s of file %s passed to client successfully\n",key,filename);
					send(client_socket ,client_message, sizeof(client_message),0);
					myAESStorage_print_storage();
					puts("");	
				}
				
			}else
				//puts("Error, failed to receive message from client.");
				socket_loop=0;
		}	
	}
	//close socket
   	close(storage_socket);
	close(client_socket);
	close(server_socket);
	puts("TPAserver closed");
        return 0;
}
