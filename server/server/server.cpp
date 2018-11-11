
#include <winsock2.h> 
#include <windows.h>
#include <winbase.h>

#pragma warning (disable : 4786) 
#pragma warning (disable : 4996)
#include <fstream>
#include <sstream>
#include <vector>
#include <direct.h>
#pragma comment(lib, "Ws2_32.lib")

#define MY_PORT 5223		  // ����, ������� ������� ������
#define TIMEOUT 1000

std::ostringstream buffer;		  // ��������� �����

std::vector<HANDLE> arrHandle;

VOID APIENTRY timer_proc(LPVOID IpArgToCompletionRoutine,
	DWORD dwTimerLowValue,
	DWORD dwTimerHighValue
);

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
DWORD WINAPI Client(LPVOID info);

// ���������� ���������� � ����������
// �������� ������������� 
int numOfClients = 0;
void deinitialize();
HWND hwnd;
bool critSectFree = true;
char granted[1024];
char denied[1024];
char request[1024];



int main(int argc, char* argv[])
{
	HANDLE hStdout;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN);

	char tmp[1000];			  // ����� ��� ��������� ����
	char buff[1024];		  // ����� ��� ��������� ����

	printf("TCP SERVER \n");

	strcpy_s(granted, "GRANTED");
	strcpy_s(denied, "DENIED");
	strcpy_s(request, "REQUEST");

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
			DWORD thID;
		tagINF info;

		info.caddr = &client_addr;
		info.soc = &client_socket;
 
		HANDLE hID = CreateThread(NULL, NULL, Client, &info, NULL, &thID);
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

DWORD WINAPI Client(LPVOID info)
{
	tagINF inf;
	SOCKET my_sock;
	sockaddr_in my_addr;
	HANDLE hID;
	inf = ((tagINF *)info)[0];
	my_sock = *(inf.soc);
	my_addr = *(inf.caddr);


	char buff[20 * 1024];
	buff[0] = '\0';

	std::ofstream out("file.txt", std::ios::app);

	int bytes_recv;

	while ((bytes_recv = recv(my_sock, &buff[0], sizeof(buff), 0))
		&& bytes_recv != SOCKET_ERROR) {
		if (critSectFree) {
			critSectFree = false;
			EnterCriticalSection(&critSect);

			send(my_sock, &granted[0], sizeof(buff) - 1, 0);

			DuplicateHandle(
				GetCurrentProcess(),
				GetCurrentThread(),
				GetCurrentProcess(),
				&hID,
				0,
				FALSE,
				DUPLICATE_SAME_ACCESS);
			// ����� � ����� ��� ���� 
			// ��������� ������� ��������� ����� 
			// (buffer) ������ ���� �[%d]: accept new client 
			// %s\n�, ��� %d ����������� ������, 
			// %s � IP-����� �������.  
			//buffer << "[" << hID << "]" << ":accept new client " << inet_ntoa(my_addr.sin_addr) << "\n";
			//printf("[%d]: accept new client %s\n", hID, inet_ntoa(my_addr.sin_addr));

			HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
			LARGE_INTEGER li;
			int nNanosecondsPerSecond = 1000000;
			__int64 qwTimeFromNowInNanoseconds = nNanosecondsPerSecond;

			qwTimeFromNowInNanoseconds = -qwTimeFromNowInNanoseconds,
				li.LowPart = (DWORD)(qwTimeFromNowInNanoseconds & 0xFFFFFFFF),
				li.HighPart = (LONG)(qwTimeFromNowInNanoseconds >> 32);

			SetWaitableTimer(hTimer, &li, TIMEOUT, timer_proc, (LPVOID)hID, FALSE);

			int bytes_recv;
			//����� ������ �� ������� 
			while ((bytes_recv = recv(my_sock, &buff[0], sizeof(buff), 0))
				&& bytes_recv != SOCKET_ERROR) {
				buffer << buff << "\n";
				printf("recive string: %s\n", buff); 
															   
				if (out.is_open())
				{
					out << buff << std::endl;
				}

				LeaveCriticalSection(&critSect);
				critSectFree = true;
			}
		}

		else {
			send(my_sock, &denied[0], sizeof(buff) - 1, 0);
		}

		numOfClients--;
		//��� ����������� ������� ������ ����� 
		//� ����� (buffer) ������ ���� �[%d]: 
		//client %s disconnected\n�, 
		//��� %d � ���������� ������,
		//%s � IP ����� �������, ����� � ������� 
		//���� ����������. 
		EnterCriticalSection(&critSect);
		buffer << "[" << hID << "] client " << inet_ntoa(my_addr.sin_addr) << " disconnected\n";
		printf("[%d]:client %s disconnected\n", hID, inet_ntoa(my_addr.sin_addr));
		PRINTUSERS
			LeaveCriticalSection(&critSect);

		closesocket(my_sock);
		CloseHandle(hID);
		//CancelWaitableTimer(hTimer);
		ExitThread(0);
		return 0;
	}
}

VOID APIENTRY  timer_proc(LPVOID IpArgToCompletionRoutine,
	DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	EnterCriticalSection(&critSect);
	printf("get connection\n", IpArgToCompletionRoutine); //[%d]: idle\n
	buffer << "[" << IpArgToCompletionRoutine << "]: idle\n";
	LeaveCriticalSection(&critSect);
}
