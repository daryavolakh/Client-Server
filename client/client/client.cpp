#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning (disable : 4996)
using namespace std;

#define PORT 5223			  
#define SERVERADDR "127.0.0.1" 
#define _WINSOCK_DEPRECATED_NO_WARNINGS

double function();

int main(int argc, char* argv[])
{
	char buff[1024];
	HANDLE hStdout;

	const char* granted = "GRANTED";
	const char* request = "REQUEST";
	const char* answer = "ANSWER";

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_INTENSITY);

	printf("TCP CLIENT\n");


	if (WSAStartup(0x202, (WSADATA *)buff))
	{
		printf("WSAStart error %d\n", WSAGetLastError());
		return -1;
	}

	SOCKET my_sock;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0)
	{
		printf("Socket() error %d\n", WSAGetLastError());
		return -1;
	}

	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst = NULL;


	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else

		if (hst = gethostbyname(SERVERADDR))
			((unsigned long *)&dest_addr.sin_addr)[0] =
			((unsigned long **)hst->h_addr_list)[0][0];
		else
		{
			printf("Invalid address %s\n", SERVERADDR);
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}


	if (connect(my_sock, (sockaddr *)&dest_addr,
		sizeof(dest_addr)))
	{
		printf("Connect error %d\n", WSAGetLastError());
		return -1;
	}

	int nsize;

	send(my_sock, request, strlen(request), 0);

	while ((nsize = recv(my_sock, buff, sizeof(buff), 0)) != SOCKET_ERROR)
	{
		buff[nsize] = 0;
		if (strcmp(buff, granted) == 0) {
			printf("%s\n", buff);
			double func = function();
			
			char funcValue[100];
			sprintf(funcValue, "%f", func);

			strcpy_s(buff, funcValue);
			printf(funcValue);
			send(my_sock, buff, strlen(buff), 0);
		}

		if (strcmp(buff, answer) == 0) {
			closesocket(my_sock);
			WSACleanup();
			return 0;
		}
	}

	printf("Recv error %d\n", WSAGetLastError());
	closesocket(my_sock);
	WSACleanup();
	return 0;
}

double function() {
	int x = rand() % 100 + 1;
	double fx = pow(x, 2) + pow(x, 3) + pow((x + 3), 4);

	return fx;
}