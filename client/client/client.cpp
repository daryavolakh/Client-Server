#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning (disable : 4996)
using namespace std;

#define _WINSOCK_DEPRECATED_NO_WARNINGS

double function(int x);
void logToFile(std::ofstream &logFile, char* message);
std::ofstream logFile("logClient.txt", std::ios::out);

int __cdecl main(int argc, char** argv)
{
	char buff[1024];

	HANDLE hStdout;

	const char* granted = "GRANTED";
	const char* request = "REQUEST";
	const char* answer = "ANSWER";

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_INTENSITY);

	printf("TCP CLIENT\n");

	/*char* serverAddr = argv[1];
	int PORT = atoi(argv[2]);
	int begin = atoi(argv[3]);
	int end = atoi(argv[4]);
	int step = atoi(argv[5]);*/
	char* serverAddr = "127.0.0.1";
	int PORT = 5223;
	int begin = 10;
	int end = 17;
	int step = 1;

	if (WSAStartup(0x202, (WSADATA *)buff))
	{
		printf("WSAStart error %d\n", WSAGetLastError());
		logToFile(logFile, "ERROR WSAStartup");
		return -1;
	}

	else {
		logToFile(logFile, "Initialization of winSock library");
	}

	SOCKET my_sock;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	logToFile(logFile, "Create socket");

	if (my_sock < 0)
	{
		printf("Socket() error %d\n", WSAGetLastError());
		logToFile(logFile, "ERROR socket");
		return -1;
	}

	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst = NULL;

	if (inet_addr(serverAddr) != INADDR_NONE) {
		dest_addr.sin_addr.s_addr = inet_addr(serverAddr);
		logToFile(logFile, "Valid address");
	}
	else
		if (hst = gethostbyname(serverAddr)) {
			((unsigned long *)&dest_addr.sin_addr)[0] = ((unsigned long **)hst->h_addr_list)[0][0];
			logToFile(logFile, "Name of host -> null");
		}
		else
		{
			printf("Invalid address %s\n", serverAddr);
			logToFile(logFile, "Invalid address");
			closesocket(my_sock);
			logToFile(logFile, "Close socket");
			WSACleanup();
			return -1;
		}


	if (connect(my_sock, (sockaddr *)&dest_addr,
		sizeof(dest_addr)))
	{
		printf("Connect error %d\n", WSAGetLastError());
		logToFile(logFile, "Connect ERROR");
		return -1;
	}

	else {
		logToFile(logFile, "Connect");
	}

	int x = begin;
	int nsize;
	send(my_sock, request, strlen(request), 0);
	logToFile(logFile, "Send request to server");
	while ((nsize = recv(my_sock, buff, sizeof(buff), 0)) != SOCKET_ERROR && x <= end)
	{
		buff[nsize] = 0;
		if (strcmp(buff, granted) == 0) {
			logToFile(logFile, "Resieve granted from server");
			printf("%s\n", buff);

			double funcValue = function(x);
			sprintf(buff, "%f\n", funcValue);

			send(my_sock, buff, strlen(buff), 0);
			logToFile(logFile, "Send function value to server");
			x += step;
		}

		if (strcmp(buff, answer) == 0 && x <= end) {
			logToFile(logFile, "Resieve answer from server");

			send(my_sock, request, strlen(request), 0);
			logToFile(logFile, "Send request to server");
		}
		if (strcmp(buff, answer) == 0 && x > end) {
			logToFile(logFile, "Resieve answer from server");
			closesocket(my_sock);
			logToFile(logFile, "Close socket");
			WSACleanup();
			return 0;
		}
	}

	printf("Recv error %d\n", WSAGetLastError());
	logToFile(logFile, "Recv ERROR");
	closesocket(my_sock);
	WSACleanup();
	return 0;
}

double function(int x) {
	double fx = pow(x, 2) + pow(x, 3) + pow((x + 3), 4);

	return fx;
}

void logToFile(std::ofstream &logFile, char* message) {
	char bufferForTime[80];
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	char* format = "%A, %B %d, %Y %H:%M:%S";
	strftime(bufferForTime, 80, format, timeinfo);
	logFile << bufferForTime << " " << message << std::endl;
}

