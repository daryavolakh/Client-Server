#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning (disable : 4996)
using namespace std;

#define PORT 5223			  //���� �� �������� ������� ������
#define SERVERADDR "127.0.0.1" //IP ��  ������� ��������� ������
#define _WINSOCK_DEPRECATED_NO_WARNINGS

double function();

int main(int argc, char* argv[])
{
	char buff[1024];		  //��������� ����� ��� ��������� ����
	HANDLE hStdout;		  //��������� ������ ������ �������

	//SetConsoleTitle("Echo - ������");
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN);

	printf("TCP CLIENT\n");

	//������������� ���������� Winsock
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		printf("WSAStart error %d\n", WSAGetLastError());
		return -1;
	}

	// �������� ������
	SOCKET my_sock;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0)
	{
		printf("Socket() error %d\n", WSAGetLastError());
		return -1;
	}

	//��������� ����������

	// ���������� ��������� sockaddr_in.
	// �������� ������ � ����� �������.
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst = NULL;

	// �������������� IP ������ �� ����������� �
	// ������� ������.
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
		// ������� �������� IP ����� �� ���������
		// ����� �������

		if (hst = gethostbyname(SERVERADDR))
			// hst->h_addr_list �������� �� ������ 
			//�������, � ������ ���������� �� ������
			((unsigned long *)&dest_addr.sin_addr)[0] =
			((unsigned long **)hst->h_addr_list)[0][0];
		else
		{
			printf("Invalid address %s\n", SERVERADDR);
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}

	// ����� ������� ������� � ��������
	// ���������� ���������� 
	if (connect(my_sock, (sockaddr *)&dest_addr,
		sizeof(dest_addr)))
	{
		printf("Connect error %d\n", WSAGetLastError());
		return -1;
	}
	// ����� ������� connect() � ��������� 
	// ������ (A) ������� ������� � ����� 
	// ��������� ������������ 
	// sleep(randomNumber), ������ randomNumber 
	// ��������� � ��������� 2..10 ������. 
	// ����� ����� �� ��������� [A..B] -> 
	// long randomNumber = A + rand() % (B+1-A);
	long randomNumber;

	randomNumber = 2 + rand() % 5;

	Sleep(1000 * randomNumber);

	// �������� ������ ������� �������
	int nsize;
	double func = function();
	
	char funcValue[100];
	sprintf(funcValue, "%f", func);
	printf(funcValue);

	strcpy_s(buff, funcValue);
	buff[strlen(funcValue)] = 0;
	send(my_sock, &buff[0], sizeof(buff) - 1, 0);

	if ((nsize = recv(my_sock, &buff[0],
		sizeof(buff) - 1, 0))
		!= SOCKET_ERROR)
	{
		// ����� ������ ������ �� �������, ������ 
		// ����������� ��������� ����� ����� 
		// � ��������������.
		// ��� �������� ����� ������� �� 5 �� 20 ���. 
		randomNumber = 5 + rand() % 16;
		Sleep(1000 * randomNumber);
		printf("Exit...");
		closesocket(my_sock);
		WSACleanup();
		return 0;
	}
	printf("Recv error \n", WSAGetLastError());
	closesocket(my_sock);
	WSACleanup();
	return -1;
}

double function() {
	int x = 0;
	printf("Enter x : ");
	scanf("%d",&x);
	double fx = pow(x,2) + pow(x, 3) + pow((x + 3),4);
	
	return fx;
}

