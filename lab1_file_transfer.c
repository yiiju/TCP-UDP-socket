/* A simple TCP and UDP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <math.h>

void error(const char *msg)
{
	perror(msg);
	exit(1);
}


/*
 * UDP server
 */
void udp_echo_ser(int sockfd, char* msg)
{
	char buffer[1024];
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int n;

	peerlen = sizeof(peeraddr);
	memset(buffer, 0, sizeof(buffer));
	n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
			(struct sockaddr *)&peeraddr, &peerlen);
	bzero(buffer,1024);
	strcpy(buffer, msg);
	// send the img
	if(buffer[0] == '/' || buffer[0] == '.') {
		// img
		// send "img" for client to tell the type
		n = sendto(sockfd, "img", 3, 0, (struct sockaddr *)&peeraddr, peerlen);
		if (n < 0) error("ERROR writing to socket");
		char file_name[1024];  
		bzero(file_name, sizeof(file_name));  
		strcpy(file_name, buffer);  
		// open the file
		FILE *fp = fopen(file_name, "r");  
		if (fp == NULL) {  
			printf("File:\t%s Not Found!\n", file_name);  
		}  
		else {
			// store the size of fp
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			// print total size
			printf("Total size of file: %d\n",size);
			// use part to get size of every 5%
			int part = size/20;

			bzero(buffer, sizeof(buffer));  
			int file_block_length = 0;  
			int count = 0;
			int count_size = 0;
			// read the file
			while( (file_block_length = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {  
				// send buffer to client
				if (sendto(sockfd, buffer, file_block_length, 0, (struct sockaddr *)&peeraddr, peerlen) < 0) {
					printf("Send File:\t%s Failed!\n", file_name);  
					break;  
				}  
				// clear the buffer
				bzero(buffer, sizeof(buffer));  

				// print log
				count_size = count_size + sizeof(buffer);
				while(count_size >= part) {
					count_size = count_size - part;
					++count;
					// get current time
					time_t rawtime;
					struct tm *timeinfo;
					time ( &rawtime );
					timeinfo = localtime ( &rawtime );

					if(count < 20) {
						printf("%d%% %d/%d/%d %d:%d:%d\n", count*5, 
								timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
								timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
					}
				} 
			}  
			// if the final size is small than part
			if(count != 20) {
				time_t rawtime;
				struct tm *timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );

				printf("%d%% %d/%d/%d %d:%d:%d\n", 100, 
						timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
						timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);		
			}
			// close file
			fclose(fp);  
			printf("File:\t%s Transfer Finished!\n", file_name);  
			// send finish
			sendto(sockfd, NULL, 0, 0, (struct sockaddr *)&peeraddr, peerlen);
		}  
	}
	else {
		// message
		sendto(sockfd, "msg", 3, 0, (struct sockaddr *)&peeraddr, peerlen);
		int size = strlen(buffer);
		int part = round(size/20);
		// set the smallest part
		if(part == 0) part = 1;

		int count = 0;
		int count_size = 0;
		// record length of pointer p
		int str_p = 0;
		// use pointer p to send one part of message
		char *p = buffer;
		while(1) {
			// send buffer to client 
			n = sendto(sockfd, p, part, 0, (struct sockaddr *)&peeraddr, peerlen);
			if(n < 0) {  
				printf("Send Failed!\n");  
				break;  
			}
			if(n == 0) break;
			if(str_p >= strlen(buffer)) break;
			// count total send length
			str_p += n;
			// move to next site
			p += n;
			// print log
			time_t rawtime;
			struct tm *timeinfo;
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );

			++count;
			if(count < 20) {
				printf("%d%% %d/%d/%d %d:%d:%d\n", count*5, 
						timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
						timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			}
		}
		// send finish
		n = sendto(sockfd, NULL, 0, 0, (struct sockaddr *)&peeraddr, peerlen);
		// print final log
		time_t rawtime;
		struct tm *timeinfo;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		while(count < 19) {
			++count;
			printf("%d%% %d/%d/%d %d:%d:%d\n", count*5,
					timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
					timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		}
		printf("100%% %d/%d/%d %d:%d:%d\n", 
				timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
				timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		printf("I send a message: %s\n",buffer);
	}
	close(sockfd);
}

/*
 * TCP server
 */
void tcp_echo_ser(int sockfd, char* msg)
{
	socklen_t clilen;
	struct sockaddr_in cli_addr;
	char buffer[1024];
	int newsockfd, n;

	clilen = sizeof(cli_addr);
	// accept
	newsockfd = accept(sockfd,
			(struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) error("ERROR on accept");
	bzero(buffer,1024);
	strcpy(buffer, msg);
	// send img
	if(buffer[0] == '/' || buffer[0] == '.') {
		// img
		n = write(newsockfd, "img", 3);
		if (n < 0) error("ERROR writing to socket");
		char file_name[1024];  
		bzero(file_name, sizeof(file_name));  
		strcpy(file_name, buffer);  
		// open file
		FILE *fp = fopen(file_name, "r");  
		if (fp == NULL) {  
			printf("File:\t%s Not Found!\n", file_name);  
		}  
		else {
			// store the size of fp
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			// print total size
			printf("Total size of file: %d\n",size);
			// use part to get size of every 5%
			int part = size/20;

			bzero(buffer, sizeof(buffer));  
			int file_block_length = 0;  
			int count = 0;
			int count_size = 0;
			// read file
			while( (file_block_length = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {  
				// send buffer to client 
				if (send(newsockfd, buffer, file_block_length, 0) < 0) {  
					printf("Send File:\t%s Failed!\n", file_name);  
					break;  
				}  
				bzero(buffer, sizeof(buffer));  

				// print log
				count_size = count_size + sizeof(buffer);
				while(count_size >= part) {
					count_size = count_size - part;
					++count;
					time_t rawtime;
					struct tm *timeinfo;
					time ( &rawtime );
					timeinfo = localtime ( &rawtime );

					if(count < 20) {
						printf("%d%% %d/%d/%d %d:%d:%d\n", count*5, 
								timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
								timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
					}
				} 
			}  
			// if the final size is small than part
			if(count != 20) {
				time_t rawtime;
				struct tm *timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );

				printf("%d%% %d/%d/%d %d:%d:%d\n", 100, 
						timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
						timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);		
			}
			// close file
			fclose(fp);  
			printf("File:\t%s Transfer Finished!\n", file_name);  
		}  
	}
	else {
		// message
		n = send(newsockfd, "msg", 3, 0); 
		if(n < 0) {  
			printf("Send Failed!\n");   
		}
		int size = strlen(buffer);
		int part = round(size/20);
		// set the smallest part
		if(part == 0) part = 1;

		int count = 0;
		int count_size = 0;
		// record length of pointer p
		int str_p = 0;
		// use pointer p to send one part of message
		char *p = buffer;
		while(1) {
			// send buffer to client 
			n = send(newsockfd, p, part, 0); 
			if(n < 0) {  
				printf("Send Failed!\n");  
				break;  
			}
			if(n == 0) break;
			if(str_p >= strlen(buffer)) break;
			str_p += n;
			p += n;
			// print log
			time_t rawtime;
			struct tm *timeinfo;
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );

			++count;
			if(count < 20) {
				printf("%d%% %d/%d/%d %d:%d:%d\n", count*5, 
						timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
						timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			}
		}

		// print final log
		time_t rawtime;
		struct tm *timeinfo;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		while(count < 19) {
			++count;
			printf("%d%% %d/%d/%d %d:%d:%d\n", count*5,
					timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
					timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		}
		printf("100%% %d/%d/%d %d:%d:%d\n", 
				timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
				timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		printf("I send a message: %s\n",buffer);
	}
	close(newsockfd);
	close(sockfd);
}

/*
 * UDP client
 */
void udp_cli(int sockfd, int portno)
{
	int n;
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	char buffer[1024];

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(portno);

	// send something to server
	sendto(sockfd, "udp", 3, 0, 
			(struct sockaddr *)&servaddr, sizeof(servaddr));

	bzero(buffer,1024);
	// get the type from server
	n = recvfrom(sockfd, buffer, 3, 0, NULL, NULL);
	if (n < 0) error("ERROR reading from socket");
	if(!strcmp(buffer, "img")) {
		// img
		bzero(buffer,1024);
		// save in ./recv_picture/udp.png
		char file_name[1024] = "./recv_picture/udp.png";
		// open file
		FILE *fp = fopen(file_name, "w");  
		if (fp == NULL) {  
			printf("File:\t%s Can Not Open To Write!\n", file_name);  
			exit(1);  
		}  
		// get the file from server
		while(n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) { 
			if (n < 0) {  
				printf("Recieve Data From Server Failed!\n");  
				break;  
			}  
			// write into file
			int write_length = fwrite(buffer, sizeof(char), n, fp);  
			if (write_length < n) {  
				printf("File: Write Failed!\n");  
				break;  
			}  
			bzero(buffer, sizeof(buffer));
			// if n is small than sizeof(buffer), it means the final one
			if (n < sizeof(buffer)) break;
		}  
		printf("Recieve File From Server Finished!\n");  
		// close file
		fclose(fp);  
	}
	else {
		// message
		char msg[1024];
		bzero(msg, 1024);
		bzero(buffer, sizeof(buffer));
		// recv from server
		while(n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) {
			if (n < 0) {  
				printf("Recieve Data From Server Failed!\n");  
				break;  
			} 
			// concatenate into msg
			strcat(msg, buffer);
			bzero(buffer, sizeof(buffer));
		}
		printf("The message is: ");
		printf("%s\n", msg);
	}
	close(sockfd);
}

/*
 * TCP server
 */
void tcp_cli(int sockfd)
{
	int n;
	char buffer[1024];
	bzero(buffer,1024);
	// read the type from server
	n = read(sockfd,buffer,3);
	if (n < 0) error("ERROR reading from socket");
	if(!strcmp(buffer, "img")) {
		// img
		bzero(buffer,1024);
		// save in ./recv_picture/tcp.png
		char file_name[1024] = "./recv_picture/tcp.png";
		// open file to write
		FILE *fp = fopen(file_name, "w");  
		if (fp == NULL) {  
			printf("File:\t%s Can Not Open To Write!\n", file_name);  
			exit(1);  
		}  
		// recv from server
		while(n = recv(sockfd, buffer, sizeof(buffer), 0)) {  
			if (n < 0) {  
				printf("Recieve Data From Server Failed!\n");  
				break;  
			}  
			// write into file
			int write_length = fwrite(buffer, sizeof(char), n, fp);  
			if (write_length < n) {  
				printf("File: Write Failed!\n");  
				break;  
			}  
			bzero(buffer, sizeof(buffer));  
		}  
		printf("Recieve File From Server Finished!\n");  
		// close file
		fclose(fp);  
	}
	else {
		// message
		char msg[1024];
		bzero(msg, 1024);
		bzero(buffer, sizeof(buffer));
		while(n = recv(sockfd, buffer, sizeof(buffer), 0)) {
			if (n < 0) {  
				printf("Recieve Data From Server Failed!\n");  
				break;  
			} 
			// concatenate into msg
			strcat(msg, buffer);
			bzero(buffer, sizeof(buffer));
		}
		printf("The message is: ");
		printf("%s\n", msg);
	}
	close(sockfd);
}

int main(int argc, char *argv[])
{
	// check the length of argv
	if (argc < 5) {
		fprintf(stderr,"ERROR, wrong input\n");
		exit(1);
	}
	if(!strcmp(argv[2],"send")) {
		// server
		int sockfd, portno;
		struct sockaddr_in serv_addr;
		int tcp_ser = 0;
		int udp_ser = 0;
		// tcp use SOCK_STREAM
		if (!strcmp(argv[1], "tcp")) {
			tcp_ser = 1;
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
		}
		// udp use SOCK_DGRAM
		else if (!strcmp(argv[1], "udp")) {
			udp_ser = 1;
			sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		}
		if (sockfd < 0)
			error("ERROR opening socket");
		bzero((char *) &serv_addr, sizeof(serv_addr));
		// set port
		portno = atoi(argv[4]);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		// bind
		if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			error("ERROR on binding");
		}
		listen(sockfd,5);

		// copy send message
		char buffer[1024];
		strcpy(buffer, argv[5]);

		// tcp server
		if (tcp_ser == 1) {
			tcp_echo_ser(sockfd, buffer);
		}
		// udp server
		else if (udp_ser == 1) {
			udp_echo_ser(sockfd, buffer);
		}
	}
	else if(!strcmp(argv[2], "recv")) {
		// client
		int sockfd, portno;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		// set port
		portno = atoi(argv[4]);
		if(!strcmp(argv[1], "tcp")) {
			// tcp use SOCK_STREAM
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
		}
		else if (!strcmp(argv[1], "udp")) {
			// udp use SOCK_DGRAM
			sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		}
		if (sockfd < 0) {
			error("ERROR opening socket");
		}
		server = gethostbyname(argv[3]);
		if (server == NULL) {
			fprintf(stderr,"ERROR, no such host\n");
			exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(portno);
		if (!strcmp(argv[1], "udp")) {
			udp_cli(sockfd, portno);
		}
		else if (!strcmp(argv[1], "tcp")) {
			// tcp need to connect
			if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
				error("ERROR connecting");
			}
			tcp_cli(sockfd);
		}
	}
	return 0;
}
