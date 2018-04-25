/***************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/23/2018
* File:         external_app.c
* Description:  Source file containing the functionality and implementation
*               of external application
***************************************************************************/

#include "external_app.h"

int main(void)
{
    int client_sock;
    struct sockaddr_in serv_addr;
    
    char buffer[BUFF_SIZE];
    
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT_NUM);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    struct _ext_app_req_msg_struct_ ext_app_req_msg;
    memset(&ext_app_req_msg, '\0', sizeof(ext_app_req_msg));
	int user_option = 0;
	
	/* Loop for getting input from the user for different operation */
	while(1)
	{
		/* Menu for the user */
		printf("\n/********************************************************************/\n");
		printf("You can enter the option to perform the following operations using this application:\n");
		printf("Enter (1) to get step count data.\n");
		printf("Enter (2) to get humidity data.\n");
		printf("Enter (3) to exit the external application.\n");
		printf("/********************************************************************/\n");

		printf("\nEnter the option number you want to select:\n");
		scanf("%d",&user_option);
		
		if((user_option > 0) || (user_option < 25)){
            memset(buffer, '\0', sizeof(buffer));
			
			if(user_option == 1){
				
				strcpy(ext_app_req_msg.req_api_msg, "get_step_count_data");
				ext_app_req_msg.req_recipient = REQ_RECP_PEDOMETER_TASK;
				ext_app_req_msg.params = -1;

				printf("Sending %s request to socket task\n", ext_app_req_msg.req_api_msg);
				ssize_t num_sent_bytes = send(client_sock, &ext_app_req_msg, 
						sizeof(struct _ext_app_req_msg_struct_), 0);
				if (num_sent_bytes < 0)
				{
					perror("send failed");
				}
				else
				{
					/* Receiving message from parent process */
					size_t num_read_bytes = read(client_sock, buffer, sizeof(buffer));
					printf("Message received in external app : %s\n", buffer);
				}
				
			}
			else if(user_option == 2){
				
				strcpy(ext_app_req_msg.req_api_msg, "get_humidity_data");
				ext_app_req_msg.req_recipient = REQ_RECP_HUMIDITY_TASK;
				ext_app_req_msg.params = -1;

				printf("Sending %s request to socket task\n", ext_app_req_msg.req_api_msg);
				ssize_t num_sent_bytes = send(client_sock, &ext_app_req_msg, 
						sizeof(struct _ext_app_req_msg_struct_), 0);
				if (num_sent_bytes < 0)
				{
					perror("send failed");
				}
				else
				{
					/* Receiving message from parent process */
					size_t num_read_bytes = read(client_sock, buffer, sizeof(buffer));
					printf("Message received in external app : %s\n", buffer);
				}
				
			}
			else if(user_option == 3){
				
				exit(0);
			}
		}
		else{
			printf("Invalid option selected, please select the correct option.\n");
		}
		
	}
	
	return 0;	
}
