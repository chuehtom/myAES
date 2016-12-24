# include <stdio.h>
# include <sys/types.h>    
# include <sys/stat.h>    
# include <fcntl.h>
# include <time.h>
# include <stdlib.h>
# include <openssl/evp.h>
# include <openssl/aes.h>
# include <string.h>
# include <unistd.h>  /* Many POSIX functions (but not all, by a large margin) */
# include "myAES.h"


int main(void){
	char command[5],filename[20];
	int loop=1;
	while(loop){
		printf("Input command : ");
		scanf("%s",command);
		if(!strcmp(command,"exit")){
			loop = 0;		
		}else if(!strcmp(command,"en")){
			printf("Input the name of file to encrypt: ");
			scanf("%s",filename);
			printf("file : %s\n",filename);
			if(aes_encrypt(filename)){
				printf("%s\n","Error, failed to encrypt.");
				//exit(-1);
			}else
				printf("%s\n","encryption successed.");
		}else if(!strcmp(command,"de")){
			printf("Input the name of file to decrypt: ");
			scanf("%s",filename);
			printf("file : %s\n",filename);
			if(aes_decrypt(filename)){
				ERR_print_errors_fp(stderr);
				printf("%s\n","Error, failed to decrypt.");
				//exit(-1);
			}else
				printf("%s\n","decryption successed.");
		}else{
			printf("invalid command.\n");
		}
	}
	

	/*
	
	
	
	*/	
	//EVP_CIPHER_CTX_cleanup(en);
	//EVP_CIPHER_CTX_cleanup(de);
	return 0;
}
