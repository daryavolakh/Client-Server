
#include <winsock2.h> 
#include <windows.h>
#include <winbase.h>

#pragma warning (disable : 4786) 
#pragma warning (disable : 4996)
#include <fstream>
#include <sstream>
#include <vector>
#include <direct.h>
#include <process.h> 
#include <ctime>
#pragma comment(lib, "Ws2_32.lib")

#define MY_PORT 5223		  // ����, ������� ������� ������
#define TIMEOUT 1000

std::ostringstream buffer;		  // ��������� �����

std::vector<HANDLE> arrHandle;

struct tagINF {
	sockaddr_in* caddr;
	SOCKET* soc;
	HANDLE hID;
};

#define PRINTUSERS if (numOfClients) printf("%d user on-line\n",numOfClients);\
else printf("No User on line\n");

CRITICAL_SECTION  critSect;

// �������� �������, �������������
// �������������� �������������
unsigned __stdcall Client(LPVOID info);

// ���������� ���������� � ����������
// �������� ������������� 
int numOfClients = 0;
void deinitialize();
HWND hwnd;

const char* granted = "GRANTED";
const char* request = "REQUEST";
const char* answer = "ANSWER";


int main(int argc, char* argv[])
{
	HANDLE hStdout;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_INTENSITY);

	char tmp[1000];			  // ����� ��� ��������� ����
	char buff[1024];		  // ����� ��� ��������� ����

	printf("TCP SERVER \n");

	// ������������� ���������� �������
	// �.�. ������������ �������� ����������
	// �� ������������ �� ���������� ��������� 
	// �� ������� �����, �������������
	// � ���������  �� ��������� WSADATA.
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// ������!
		printf("Error WSAStartup %d\n", WSAGetLastError());
		return -1;
	}
	InitializeCriticalSection(&critSect);
	// �������� ������
	SOCKET mysocket;
	// AF_INET     - ����� ���������
	// SOCK_STREAM  - ��������� ����� 
	//(� ���������� ����������)
	// 0 - �� ��������� ���������� TCP ��������
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// ������!
		printf("Error socket %d\n", WSAGetLastError());
	}

	// ���������� ������ � ��������� �������
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	// �� �������� � ������� �������!!!
	local_addr.sin_port = htons(MY_PORT);
	// ������ ��������� �����������
	// �� ��� IP-������
	local_addr.sin_addr.s_addr = 0;

	// �������� bind ��� ����������
	if (bind(mysocket, (sockaddr *)&local_addr,
		sizeof(local_addr)))
	{
		// ������!
		printf("Error bind %d\n", WSAGetLastError());
		// ��������� �����!
		closesocket(mysocket);

	}

	// �������� �����������
	// ������ ������� � 0x100
	if (listen(mysocket, 0x100))
	{
		// ������!
		printf("Error listen %d\n", WSAGetLastError());
		closesocket(mysocket);
	}

	strcpy_s(tmp, "�������� �����������...\n");
	AnsiToOem(tmp, tmp);
	puts(tmp);

	//��������� ��������� �� �������
	// ����� ��� �������

	SOCKET client_socket;
	// ����� �������
	// (����������� ��������)
	sockaddr_in client_addr;

	// ������� accept ���������� �������� ������
	// ���������
	int client_addr_size = sizeof(client_addr);

	while ((client_socket = accept(mysocket, (sockaddr *)
		&client_addr, &client_addr_size)))
	{
		// ����������� �������
		// �������������� ��������
		numOfClients++;

		// �������� �������� ��� �����
		HOSTENT *hst;
		hst = gethostbyaddr((char *)
			&client_addr.sin_addr.s_addr, 4, AF_INET);

		PRINTUSERS
			unsigned  thID;
		tagINF info;

		info.caddr = &client_addr;
		info.soc = &client_socket;

		HANDLE hID = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)&Client, &info, 0, &thID);
		arrHandle.push_back(hID);

	}

	deinitialize();

	return 0;
}

void deinitialize() {
	// �������������� ���������� Winsock
	WSACleanup();
	std::vector<HANDLE>::iterator it;
	for (it = arrHandle.begin(); it != arrHandle.end(); it++)
		CloseHandle(*it);
	DeleteCriticalSection(&critSect);
}

unsigned __stdcall Client(LPVOID info)
{
	tagINF inf;
	SOCKET my_sock;
	sockaddr_in my_addr;

	inf = ((tagINF *)info)[0];
	my_sock = *(inf.soc);
	my_addr = *(inf.caddr);

	char buff[1024];
	int bytes_recv;

	while ((bytes_recv = recv(my_sock, buff, sizeof(buff), 0))
		&& bytes_recv != SOCKET_ERROR) {
		EnterCriticalSection(&critSect);
		buff[bytes_recv] = 0;

		printf("client %d,  recive string: %s\n", my_sock, buff);
		if (strcmp(buff, request) == 0) {
			send(my_sock, granted, strlen(granted), 0);
		}
		else
		{
			std::ofstream out("file.txt", std::ios::app);
			if (out.is_open())
			{
				out << my_sock << " " << buff << " ";

				char bufferForTime[80];
				time_t seconds = time(NULL);
				tm* timeinfo = localtime(&seconds);
				char* format = "%H:%M:%S";
				strftime(bufferForTime, 80, format, timeinfo);
				out << bufferForTime << std::endl;
				out.close();
			}
			Sleep(1000 * 5);
			send(my_sock, answer, strlen(answer), 0);
		}

		LeaveCriticalSection(&critSect);
	}


	closesocket(my_sock);
	ExitThread(0);
	return 0;
}

