#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <ctype.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "awget.h"

#define BUF_SIZE 400
char buffer_global [200];
struct chainfile chain_file;
char *buffer;
char *stream_data;
int random_number;
int max_size;
char *file_name;
int accept1;

void random_SS()
{
	srand(time(NULL));
	int no;
	no=chain_file.number_of_SS;
	random_number = (rand() % no)+1;
}

void read_no_SS(char *FILENAME_1)
{
	FILE *fh;
	fh= fopen(FILENAME_1, "r");
	buffer = (char*) malloc(400 * sizeof(char *));
	fgets(buffer, BUF_SIZE, fh);
	max_size= atoi(buffer);
	fclose(fh);
}

char** get_list_from_chainfile(char *FILENAME_2)
{
	FILE *fh;
	fh= fopen(FILENAME_2, "r");
	int no_of_SS, no_of_SS_1, max_line_size = 0;
	int temp_file=0;
	int i=0;
	no_of_SS = max_size;
	no_of_SS_1 = no_of_SS;
	no_of_SS++;

	fseek(fh, 0, SEEK_SET);
	do {
		temp_file = fgetc(fh);
		if (temp_file == '\n') {
			if (i > max_line_size)
			{
				max_line_size = i;

			} else if (max_line_size >= i)
			{
				/*DO NOTHING*/
			}
			i = 0;

		} else if (temp_file != '\n') {
			i++;
		}
	} while (temp_file != EOF);

	chain_file.SS_ip_port = (char**) malloc(sizeof(char*) * no_of_SS+5);
	for (i = 0; i < no_of_SS+5; i++)
	{
		chain_file.SS_ip_port[i] = (char*) malloc(sizeof(char) * max_line_size+5);
	}
	int j=0;
	fseek(fh, 0, SEEK_SET);
	while (!feof(fh))
	{
		fgets(chain_file.SS_ip_port[j], max_line_size+5, fh);
		j++;
		if (fgetc(fh) == EOF)
		{
			break;
		}
		else
		{
			fseek(fh, -1, SEEK_CUR);
		}
	}
	chain_file.number_of_SS=no_of_SS_1;
	return (chain_file.SS_ip_port);
}

void get_ip_port(char *str)
{
	char **token  = NULL;
	char *delimiter;
	strcpy(buffer_global, str);
	delimiter=(char*) malloc(300 * sizeof(char *));
	delimiter= strtok (buffer_global, ",");
	int token_size = 0;

	while (delimiter)
	{
		token = realloc (token, sizeof (char*) * ++token_size);

		if (token == NULL)
		{
			exit (-1);
		}
		token[token_size-1] = delimiter;
		delimiter = strtok (NULL, ",");
	}

	token = realloc (token, sizeof (char*) * (token_size+1));
	token[token_size] = 0;

	strcpy(chain_file.SSaddr,token[0]);
	chain_file.SSport=atoi(token[1]);
	free (token);
}

void remove_entry(int number_of_entry)
{
	int i;
	char *c;
	for (i = 1; i < chain_file.number_of_SS; i++)
	{
		if (i >= number_of_entry)
		{
			strcpy(chain_file.SS_ip_port[i],chain_file.SS_ip_port[i+1]);
		}
	}
	chain_file.number_of_SS--;

	printf("new number of SS after remove_entry: %d\n", chain_file.number_of_SS);

	c = (char*) malloc(255 * sizeof(char *));
	sprintf(c, "%d", chain_file.number_of_SS);
	strcat(c, "\n");
	strcpy(chain_file.SS_ip_port[0], c);
	strcpy(chain_file.SS_ip_port[chain_file.number_of_SS+1], "\0");

}

void make_stream()
{
	int size_of_stream, i=0;
	size_of_stream = chain_file.number_of_SS * 255 + chain_file.number_of_SS + sizeof(chain_file.URL)+ 20;

	stream_data = (char *)malloc(size_of_stream * sizeof(char *));

	strcpy(stream_data,chain_file.URL);
	strcat(stream_data, "\n");

	for(i=0; i< chain_file.number_of_SS+1; i++)
	{
		strcat(stream_data, chain_file.SS_ip_port[i]);
	}
}

void create_connection_and_send()
{
	int sock, p_no;
	p_no=chain_file.SSport;
	struct sockaddr_in server2;
	struct hostent *serv;
	serv = gethostbyname(chain_file.SSaddr);

	sock=socket(AF_INET, SOCK_STREAM, 0);
	if (sock==-1)
	{
		printf("Error in creating socket.\n");
		exit(1);
	}
	else
		printf("\nConnecting to a Server...\n");

	bzero(&server2 ,sizeof(server2));

	server2.sin_family=AF_INET;
	bcopy((char *)serv->h_addr, (char *)&server2.sin_addr.s_addr, serv->h_length);
	server2.sin_port=htons(p_no);

	if (connect(sock,(struct sockaddr *) &server2, sizeof(server2)) < 0)
	{
		printf("\nConnection to server failed. Kindly run ./chat -h for help.\n");
		exit(0);
	}
	else
		printf("\nConnected!\nConnected to a Friend! You send first.\n");

	send(sock, stream_data, strlen(stream_data), 0);
}

void file_in_chunk_send()
{
	unsigned long filesize;

	FILE *fd;
	fd = fopen(file_name, "rb");

	if (fd == NULL)
	{
		printf("File not found!\n");
	}
	else
	{
		fseek(fd, 0, SEEK_END);
		filesize = ftell(fd);
		fseek(fd, 0L, SEEK_SET);
		printf("Relaying File ...\n");
	}
	char *entire_file;
	entire_file = (char *)malloc((filesize * (sizeof(char))) * sizeof(char *));
	fread(entire_file, sizeof(char), filesize, fd);
	fclose(fd);
	int chunk, write1, i=0;
	unsigned long file_counter = 0;
	char* file_size;
	file_size = (char *)malloc(20 * sizeof(char *));

	sprintf(file_size, "%ld", filesize);


	write1 = write(accept1, file_size, 20);
	chunk = 2000;
	if (write1 < 0)
	{
		printf("Error in writing socket during sending of file\n");
	}

	for(i = 0; i < filesize+1; i=i+chunk)
	{
		write1 = write(accept1, entire_file, chunk);

		if (write1 <= 0)
		{
			printf("ERROR writing to socket\n");
		}
		entire_file = entire_file + chunk;
		file_counter = file_counter + chunk ;

		if((file_counter + chunk) >= filesize)
		{
			int leftover = 0;
			leftover = filesize % chunk;
			write1 = write(accept1, entire_file, leftover+1);

			if (write1 <= 0)
			{
				printf("ERROR writing to socket\n");
			}
			break;
		}
	}
}




void get_filename_from_url()
{
	//char *file_name;
	char *token;
	char *delimiter;
	char *temp;
	int url_length,filenamelen;
	char *temp_buffer;
	int n = 0, i=0;
	url_length = strlen(chain_file.URL);
	temp = (char *)malloc((url_length+10) * sizeof(char *));
	temp_buffer = (char *) malloc((url_length + 10) * sizeof(char *));
	bzero(temp_buffer, url_length + 10);
	strcpy(temp_buffer, chain_file.URL);
	strcpy(temp, chain_file.URL);

	while((temp = strchr(temp, '/')) != NULL){
		i++;
		temp++;
	}

	delimiter = "/";
	token = strtok(temp_buffer, delimiter);

	if(i < 3)
	{
		file_name = (char *)malloc(15 * sizeof(char *));
		file_name = "index.html";
	}
	else
	{
		while(n < i-1)
		{
			token = strtok(NULL, delimiter);
			n++;
		}
		filenamelen = strlen(token);
		file_name = (char *)malloc((filenamelen+10) * sizeof(char *));
		strcpy(file_name, token);
	}
}

void create_client_connection()
{
	int port_no, n, client_socket;
	struct sockaddr_in client;
	struct hostent *server;
	port_no = chain_file.SSport;
	server = gethostbyname(chain_file.SSaddr);

	printf("Next SS is %s, %d\n", chain_file.SSaddr, port_no);

	client_socket = socket(AF_INET,SOCK_STREAM,0);	 //Creating socket, connecting, sending and receiving same as server.
	if(client_socket < 0)
	{
		printf("Client socket not created successfully.\n");
		exit(0);
	}

	printf("\nConnecting to the SS ...\n");


	bzero(&client,sizeof(client));
	client.sin_family = AF_INET;

	bcopy((char *)server->h_addr,(char *)&client.sin_addr.s_addr,server->h_length);
	client.sin_port = htons(port_no);


	if (connect(client_socket,(struct sockaddr *) &client,sizeof(client)) < 0)
	{
		printf("Connection attempt failed.. \n");
		exit(0);
	}

	printf("Connected!\n");

	int send1;

	send1 = send(client_socket,stream_data,strlen(stream_data),0);
	if (send1 < 0)
	{
		printf("Error in sending\n");
	}

	char* comm;
	comm = (char *)malloc(20 * sizeof(char *));
	strcpy(comm, "touch ");
	strcat(comm, file_name);
	system(comm);
	FILE *fp;
	fp = fopen(file_name, "wb");
	if (fp == NULL)
	{
		printf("File not found!\n");

	}

	printf("Waiting for file...\n");
	printf("...\n");

	char* in_file_size;
	char* receive_buffer;
	int file_reader = 0, in_file_size_int = 0, chunk;
	in_file_size = (char *)malloc(20 * sizeof(char *));
	n = read(client_socket,in_file_size,20);
	chunk = 2000;
	in_file_size_int = atoi(in_file_size);
	receive_buffer = (char *)malloc(in_file_size_int * sizeof(char *));

	while (file_reader < in_file_size_int+1)
	{
		bzero(receive_buffer,sizeof(receive_buffer));

		n = read(client_socket,receive_buffer,chunk);
		if (n < 0)
		{
			printf("ERROR reading from socket");
		}
		n = fwrite(receive_buffer, chunk, 1, fp);
		if (n < 0)
			printf("ERROR writing in file");
		file_reader = file_reader+chunk;

		if((file_reader + chunk) >= in_file_size_int)
		{
			char *temp_buff;
			int leftover = 0;
			leftover = in_file_size_int % chunk;
			temp_buff = (char *)malloc(leftover * sizeof(char *));
			n = read(client_socket,temp_buff,leftover);
			if (n <= 0)
			{
				printf("\nERROR reading from socket\n");
			}
			n = fwrite(temp_buff, leftover, 1, fp);
			if (n < 0)
			{
				printf("ERROR writing in file");
			}
			break;
		}
	}
	printf("Relaying File...\n");
	fclose(fp);
	close(client_socket);
}

int main(int argc, char* argv[])
{
	char *chainfilename;
	if(argc == 4)
	{
		if(strcmp(argv[2], "-c") == 0)
		{
			chainfilename = (char *)malloc((strlen(argv[3])) * sizeof(char *));
			strcpy(chainfilename, argv[3]);
			read_no_SS(chainfilename);
			get_list_from_chainfile(chainfilename);

		}
		else
		{
			printf("\nInappropriate flag usage.\n");
			printf("The correct usage is :\n");
			printf("./awget <url> -c <chainfile name>\nor\n");
			printf("./awget <url>\n");
			exit(1);
		}
	}
	else if(argc == 2)
	{
		read_no_SS("chaingang.txt");
		get_list_from_chainfile("chaingang.txt");
	}
	else
	{
		printf("\nInappropriate arguments usage.\n");
		printf("The correct usage is :\n");
		printf("./awget <url> -c <chainfile name>\nor\n");
		printf("./awget <url>\n");
		exit(1);
	}
	chain_file.URL = (char*) malloc(200 * sizeof(char *));
	strcpy(chain_file.URL, argv[1]);
	printf("\nRequest : %s\n",chain_file.URL);
	get_filename_from_url();
	random_SS();
	get_ip_port(chain_file.SS_ip_port[random_number]);
	make_stream();
	create_client_connection();
	return(0);
}
