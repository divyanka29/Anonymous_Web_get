#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include "awget.h"

int in_port;
char* self_port;
char file_buffer[3500];
char *stream_data;
struct chainfile chain_file;
char host[NI_MAXHOST];
char buffer_global[2000];
int accept1;
int random_number_SS;
int chunk;
int flg=0;

pthread_t thread;

void *thread_call(void *);
void stream_to_structure();
void own_ip();
void create_server(int );
void call_wget(char *, int);
void file_in_chunk_send(char *, int);
void remove_self_entry(char *, char *);
void make_stream_in_SS();
void create_connection(char *);
char* get_file_name();
void random_num();

void own_ip()
{
	struct ifaddrs *ifaddr, *ifa;
	int s;

	if (getifaddrs(&ifaddr) == -1) {
		printf("\nSome error in getting IP address. \n");
		exit(0);
	}

	for (ifa = ifaddr; ifa != NULL ; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL )
			continue;

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
				NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		if ((strcmp(ifa->ifa_name, "em1") == 0)
				&& (ifa->ifa_addr->sa_family == AF_INET)) {
			if (s != 0) {
				printf("getnameinfo() failed: \n");
				exit(0);
			}
		}
	}
	freeifaddrs(ifaddr);
}

void create_server(int p_no)
{

	printf("\nWelcome to the SS program!\n");

	int sock1, bind1, *sock;
	struct sockaddr_in server, client;
	socklen_t client_len;

	sock1 = socket(AF_INET, SOCK_STREAM, 0);
	if(sock1 < 0)
	{
		printf("Error in creating socket");
	}

	bzero((char *) &server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(p_no);

	bind1 = bind(sock1, (struct sockaddr *) &server,
			sizeof(server));

	if(bind1 < 0)
	{
		printf("Bind Error");
	}

	own_ip();

	while(1)
	{

		printf("SS listening on IP %s and port %d\n",host,p_no);

		listen(sock1, 2);

		client_len = sizeof(client);



		accept1 = accept(sock1,(struct sockaddr *) &client,&client_len);	//Accept the connection
		printf("Connected!\n");

		if(accept1 < 0)
		{
			printf("Some error in accepting the connection from the client.\n ");

		}



		sock = malloc(1);
		*sock = accept1;
		pthread_create(&thread, NULL, thread_call, (void*)sock);

	}
}

void *thread_call(void* soc)
{
	int read1;
	int sock = *(int*)soc;
	char *filename;
	filename = (char *)malloc((100)* sizeof(char *));
	bzero(buffer_global,sizeof(buffer_global));
	read1 = 0;
	read1 = read(sock,buffer_global,1000);
	if (read1 < 0)
	{
		printf("Error in reading\n");
	}

	stream_data = (char *)malloc(2000 * sizeof(char *));
	strcpy(stream_data, buffer_global);

	free(soc);
	stream_to_structure();
	filename = get_file_name();
	own_ip();


	chain_file.number_of_SS--;

	if(chain_file.number_of_SS == 0)
	{
		printf("Chain list is empty\n");
		call_wget(filename, sock);
		printf("Goodbye!\n");


		int i,size;
		char* delete;
		size = strlen(filename);
		delete = (char *)malloc(size + 10);
		bzero(chain_file.URL,strlen(chain_file.URL));
		for(i = 0; i < chain_file.number_of_SS+1; i++)
		{
			bzero(chain_file.SS_ip_port[i],strlen(chain_file.SS_ip_port[i]));
		}
		chain_file.number_of_SS = 0;
		bzero(stream_data,strlen(stream_data));
		strcpy(delete, "rm ");
		strcat(delete, filename);
		system(delete);

		printf("\nWaiting for next connection\n");
	}
	else
	{
		int sock2;
		sock2= sock;
		chain_file.number_of_SS++;
		char* stream_self_ip;
		stream_self_ip = (char *)malloc(25 * sizeof(char *));
		strcpy(stream_self_ip,host);
		strcat(stream_self_ip, ",");
		strcat(stream_self_ip, self_port);
		strcat(stream_self_ip, "\n");
		remove_self_entry(host, self_port);

		make_stream_in_SS();

		random_num();

		create_connection(filename);
		file_in_chunk_send(filename, sock2);
		printf("Goodbye!\n");

		int i,size;
		char* delete;
		size = strlen(filename);
		delete = (char *)malloc(size + 10);
		bzero(chain_file.URL,strlen(chain_file.URL));
		for(i = 0; i < chain_file.number_of_SS+1; i++)
		{
			bzero(chain_file.SS_ip_port[i],strlen(chain_file.SS_ip_port[i]));
		}
		chain_file.number_of_SS = 0;
		bzero(stream_data,strlen(stream_data));
		strcpy(delete, "rm ");
		strcat(delete, filename);
		system(delete);

		printf("\nWaiting for next connection\n");
	}

	return(0);
}

void stream_to_structure()
{

	int length_streamdata, length_url, i;
	int j=1;
	length_streamdata = strlen(stream_data);

	char *token;
	char buffer[length_streamdata+10];
	char* temp_buff;
	char* delimiter;

	temp_buff = (char *)malloc((length_streamdata+10) * sizeof(char *));
	strcpy(buffer, stream_data);
	strcpy(temp_buff,stream_data);
	delimiter = "\n";

	token = strtok(buffer, delimiter);
	length_url = strlen(token);
	chain_file.URL = (char *)malloc((length_url+10) * sizeof(char *));

	strcpy(chain_file.URL, token);

	int point = 0;
	while((temp_buff = strchr(temp_buff, '\n')) != NULL) {
		point++;
		temp_buff++;
	}

	chain_file.SS_ip_port = (char**) malloc(sizeof(char*) * point);
	for (i = 0; i < point; i++) {
		chain_file.SS_ip_port[i] = (char*) malloc(sizeof(char) * 30);
	}

	token = strtok(NULL, delimiter);
	strcpy(chain_file.SS_ip_port[0], token);
	chain_file.number_of_SS = atoi(token);
	strcat(chain_file.SS_ip_port[0], "\n");

	for(j=1; j <= chain_file.number_of_SS; j++)
	{
		token = strtok(NULL, delimiter);
		strcpy(chain_file.SS_ip_port[j], token);
		strcat(chain_file.SS_ip_port[j], "\n");
	}

	printf("Request: %s\n",chain_file.URL);
	printf("Chainlist is: \n");
	for(i=0;i<chain_file.number_of_SS+1;i++)
	{
		printf("%s\n",chain_file.SS_ip_port[i]);
	}
}



char* get_file_name()
{
	char *file_name;
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
	printf("filename:%s\n", file_name);
	return(file_name);
}

void remove_self_entry(char *host, char *port_no)
{
	int i, j;
	char *c;
	char *buffer;

	buffer = (char *) malloc(30 * sizeof(char *));
	strcpy(buffer, host);
	strcat(buffer, ",");
	strcat(buffer, port_no);
	strcat(buffer, "\n");

	for (i = 1; i < chain_file.number_of_SS+1; i++)
	{
		if ( strcmp(chain_file.SS_ip_port[i], buffer) == 0 )
		{

			int k =0;

			for(k = i; k<chain_file.number_of_SS; k++)
			{
				strcpy(chain_file.SS_ip_port[k],chain_file.SS_ip_port[k+1]);
			}

		}
	}

	chain_file.number_of_SS--;


	c = (char*) malloc(255 * sizeof(char *));
	sprintf(c, "%d", chain_file.number_of_SS);
	strcat(c, "\n");
	strcpy(chain_file.SS_ip_port[0], c);
	strcpy(chain_file.SS_ip_port[chain_file.number_of_SS+1], "\0");

	for (j=0; j<=chain_file.number_of_SS+1;j++)
	{
		printf("\nnew chainfile:%s\n:", chain_file.SS_ip_port[j]);
	}
}



void call_wget(char *file_name, int sock)
{
	char* wget_command;
	int len = 0;

	len = strlen(chain_file.URL);
	wget_command = (char *)malloc((len + 10) * sizeof(char *));
	strcpy(wget_command, "wget ");
	strcat(wget_command, chain_file.URL);

	printf("Issuing wget for the file %s\n", file_name);
	printf("...\n");
	system(wget_command);
	printf("\nFile received!\n");

	file_in_chunk_send(file_name, sock);
}

void file_in_chunk_send(char *file_name, int accept1)
{
	unsigned long filesize;

	FILE *fd;
	printf("file_name: %s\n", file_name);
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


void make_stream_in_SS()
{
	int size_of_stream, i=0;
	size_of_stream = chain_file.number_of_SS * 255 + chain_file.number_of_SS + sizeof(chain_file.URL)+ 20;

	stream_data = (char *)malloc(size_of_stream * sizeof(char *));

	strcpy(stream_data,chain_file.URL);
	strcat(stream_data, "\n");

	for(i=0; i< chain_file.number_of_SS+1; i++)
	{
		strcat(stream_data, chain_file.SS_ip_port[i]);
		strcat(stream_data, "\n");
	}


}



void create_connection(char *file_name)
{
	chunk = 2000;
	int p_no, n;
	flg++;
	int a1, a2, a3, a4;
	int client_soc;

	struct sockaddr_in serv_addr;
	struct hostent *server;
	p_no = chain_file.SSport;
	server = gethostbyname(chain_file.SSaddr);

	printf("Next SS is %s, %d\n", chain_file.SSaddr, chain_file.SSport);

	client_soc = socket(AF_INET,SOCK_STREAM,0);
	if(client_soc < 0)
	{
		printf("Client socket not created successfully.\n");
		exit(0);
	}
	if(flg == 1)
	{
		a1 = client_soc;
		printf("\nConnecting to the SS ...\n");


		bzero(&serv_addr,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(p_no);


		if (connect(a1,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		{
			printf("error in connection\n");
			exit(0);
		}

		printf("Connected!\n");

		int sent = 0;

		sent = send(a1,stream_data,strlen(stream_data),0);
		if(sent < 0)
		{
			printf("Error in sending\n");
			exit(0);
		}
		get_file_name();
		char* write_command;
		write_command = (char *)malloc(20 * sizeof(char *));
		strcpy(write_command, "touch ");
		strcat(write_command, file_name);
		system(write_command);
		FILE *fp;
		fp = fopen(file_name, "wb");
		if (fp == NULL)
		{
			printf("File not found!\n");

		}

		printf("Waiting for file...\n");
		printf("...\n");

		char* in_file_size;
		char* receive;
		int file_reader = 0, in_file_size_int = 0;
		in_file_size = (char *)malloc(20 * sizeof(char *));
		n = read(a1,in_file_size,20);
		in_file_size_int = atoi(in_file_size);
		receive= (char *)malloc(in_file_size_int * sizeof(char *));

		while (file_reader < in_file_size_int+1)
		{
			bzero(receive,sizeof(receive));

			n = read(a1,receive,chunk);
			if (n < 0) printf("ERROR reading from socket\n");

			n = fwrite(receive, chunk, 1, fp);
			if (n < 0) printf("ERROR writing in file\n");
			file_reader = file_reader+chunk;

			if((file_reader + chunk) >= in_file_size_int)
			{
				char *temp_buff;
				int difference = 0;
				difference = in_file_size_int - file_reader;
				temp_buff = (char *)malloc(difference * sizeof(char *));
				n = read(a1,temp_buff,difference);
				if (n <= 0)
				{
					printf("\nERROR reading from socket\n");
				}
				n = fwrite(temp_buff, difference, 1, fp);
				if (n < 0)
				{
					printf("ERROR writing in file\n");
				}
				break;
			}
		}
		printf("Relaying File...\n");
		fclose(fp);
		close(a1);
	}
	else if(flg == 2)
	{
		a2 = client_soc;
		printf("\nConnecting to the SS ...\n");


		bzero(&serv_addr,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(p_no);


		if (connect(a2,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		{
			printf("error in connection\n");
			exit(0);
		}

		printf("Connected!\n");

		int sent = 0;

		sent = send(a2,stream_data,strlen(stream_data),0);
		if(sent < 0)
		{
			printf("Error in sending\n");
			exit(0);
		}
		get_file_name();
		char* write_command;
		write_command = (char *)malloc(20 * sizeof(char *));
		strcpy(write_command, "touch ");
		strcat(write_command, file_name);
		system(write_command);
		FILE *fp;
		fp = fopen(file_name, "wb");
		if (fp == NULL)
		{
			printf("File not found!\n");

		}

		printf("Waiting for file...\n");
		printf("...\n");

		char* in_file_size;
		char* receive;
		int file_reader = 0, in_file_size_int = 0;
		in_file_size = (char *)malloc(20 * sizeof(char *));
		n = read(a2,in_file_size,20);
		in_file_size_int = atoi(in_file_size);
		receive= (char *)malloc(in_file_size_int * sizeof(char *));

		while (file_reader < in_file_size_int+1)
		{
			bzero(receive,sizeof(receive));

			n = read(a2,receive,chunk);
			if (n < 0) printf("ERROR reading from socket\n");

			n = fwrite(receive, chunk, 1, fp);
			if (n < 0) printf("ERROR writing in file\n");
			file_reader = file_reader+chunk;

			if((file_reader + chunk) >= in_file_size_int)
			{
				char *temp_buff;
				int difference = 0;
				difference = in_file_size_int - file_reader;
				temp_buff = (char *)malloc(difference * sizeof(char *));
				n = read(a2,temp_buff,difference);
				if (n <= 0)
				{
					printf("\nERROR reading from socket\n");
				}
				n = fwrite(temp_buff, difference, 1, fp);
				if (n < 0)
				{
					printf("ERROR writing in file");
				}
				break;
			}
		}
		printf("Relaying File...\n");
		fclose(fp);
		close(a2);
	}
	else if(flg == 3)
	{
		a3 = client_soc;
		printf("\nConnecting to the SS ...\n");


		bzero(&serv_addr,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(p_no);


		if (connect(a3,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		{
			printf("error in connection\n");
			exit(0);
		}

		printf("Connected!\n");

		int sent = 0;

		sent = send(a3,stream_data,strlen(stream_data),0);
		if(sent < 0)
		{
			printf("Error in sending\n");
			exit(0);
		}
		get_file_name();
		char* write_command;
		write_command = (char *)malloc(20 * sizeof(char *));
		strcpy(write_command, "touch ");
		strcat(write_command, file_name);
		system(write_command);
		FILE *fp;
		fp = fopen(file_name, "wb");
		if (fp == NULL)
		{
			printf("File not found!\n");

		}

		printf("Waiting for file...\n");
		printf("...\n");

		char* in_file_size;
		char* receive;
		int file_reader = 0, in_file_size_int = 0;
		in_file_size = (char *)malloc(20 * sizeof(char *));
		n = read(a3,in_file_size,20);
		in_file_size_int = atoi(in_file_size);
		receive= (char *)malloc(in_file_size_int * sizeof(char *));

		while (file_reader < in_file_size_int+1)
		{
			bzero(receive,sizeof(receive));

			n = read(a3,receive,chunk);
			if (n < 0) printf("ERROR reading from socket");

			n = fwrite(receive, chunk, 1, fp);
			if (n < 0) printf("ERROR writing in file");
			file_reader = file_reader+chunk;

			if((file_reader + chunk) >= in_file_size_int)
			{
				char *temp_buff;
				int difference = 0;
				difference = in_file_size_int - file_reader;
				temp_buff = (char *)malloc(difference * sizeof(char *));
				n = read(a3,temp_buff,difference);
				if (n <= 0)
				{
					printf("\nERROR reading from socket\n");
				}
				n = fwrite(temp_buff, difference, 1, fp);
				if (n < 0)
				{
					printf("ERROR writing in file");
				}
				break;
			}
		}
		printf("Relaying File...\n");
		fclose(fp);
		close(a3);
	}
	else
	{
		a4 = client_soc;
		printf("\nConnecting to the SS ...\n");


		bzero(&serv_addr,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(p_no);


		if (connect(a4,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		{
			printf("error in connection\n");
			exit(0);
		}

		printf("Connected!\n");

		int sent = 0;

		sent = send(a4,stream_data,strlen(stream_data),0);
		if(sent < 0)
		{
			printf("Error in sending\n");
			exit(0);
		}
		get_file_name();
		char* write_command;
		write_command = (char *)malloc(20 * sizeof(char *));
		strcpy(write_command, "touch ");
		strcat(write_command, file_name);
		system(write_command);
		FILE *fp;
		fp = fopen(file_name, "wb");
		if (fp == NULL)
		{
			printf("File not found!\n");

		}

		printf("Waiting for file...\n");
		printf("...\n");

		char* in_file_size;
		char* receive;
		int file_reader = 0, in_file_size_int = 0;
		in_file_size = (char *)malloc(20 * sizeof(char *));
		n = read(a4,in_file_size,20);
		in_file_size_int = atoi(in_file_size);
		receive= (char *)malloc(in_file_size_int * sizeof(char *));

		while (file_reader < in_file_size_int+1)
		{
			bzero(receive,sizeof(receive));

			n = read(a4,receive,chunk);
			if (n < 0) printf("ERROR reading from socket");

			n = fwrite(receive, chunk, 1, fp);
			if (n < 0) printf("ERROR writing in file");
			file_reader = file_reader+chunk;

			if((file_reader + chunk) >= in_file_size_int)
			{
				char *temp_buff;
				int difference = 0;
				difference = in_file_size_int - file_reader;
				temp_buff = (char *)malloc(difference * sizeof(char *));
				n = read(a4,temp_buff,difference);
				if (n <= 0)
				{
					printf("\nERROR reading from socket\n");
				}
				n = fwrite(temp_buff, difference, 1, fp);
				if (n < 0)
				{
					printf("ERROR writing in file");
				}
				break;
			}
		}
		printf("Relaying File...\n");
		fclose(fp);
		close(a4);
	}
}


void random_num()
{
	srand(time(NULL));
	int no;
	no=chain_file.number_of_SS;
	random_number_SS = (rand() % no)+1;
	char *SS_chosen;
	SS_chosen = (char*) malloc(sizeof(char) * 256);
	SS_chosen = chain_file.SS_ip_port[random_number_SS];
	char **token  = NULL;
	char *delimiter;
	strcpy(buffer_global, SS_chosen);
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




int main(int argc, char *argv[])
{
	int p_len = 0;
	if(argc < 2)
	{
		printf("Inappropriate argument usage.\n");
		printf("The correct usage is : \n");
		printf("./ss <port number>\n");
		exit(0);
	}
	p_len = strlen(argv[1]);
	self_port = (char *)malloc(p_len * sizeof(char *));
	strcpy(self_port, argv[1]);
	in_port = atoi(argv[1]);
	create_server(in_port);
	return(0);
}
