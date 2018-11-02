#include "stdafx.h"
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

#define MY_PORT 5223		  // Порт, который слушает сервер
#define TIMEOUT 1000

std::ostringstream buffer;		  // Строковый буфер

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

// Прототип функции, обслуживающий
// подключившихся пользователей
DWORD WINAPI Client(LPVOID info);

// Глобальная переменная – количество
// активных пользователей 
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

	//SetConsoleTitle("Echo - сервер");
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN);

	char tmp[1000];			  // Буфер для различных нужд
	char buff[1024];		  // Буфер для различных нужд

	printf("TCP SERVER \n");

	strcpy_s(granted, "GRANTED");
	strcpy_s(denied, "DENIED");
	strcpy_s(request, "REQUEST");

	// Инициализация Библиотеки Сокетов
	// Т.к. возвращенная функцией информация
	// не используется ей передается указатель 
	// на рабочий буфер, преобразуемый
	// к указателю  на структуру WSADATA.
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// Ошибка!
		printf("Error WSAStartup %d\n", WSAGetLastError());
		return -1;
	}
	// Инициализация критической секции
	InitializeCriticalSection(&critSect);
	// Создание сокета
	SOCKET mysocket;
	// AF_INET     - сокет Интернета
	// SOCK_STREAM  - потоковый сокет 
	//(с установкой соединения)
	// 0 - по умолчанию выбирается TCP протокол
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// Ошибка!
		printf("Error socket %d\n", WSAGetLastError());
	}

	// Связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	// Не забываем о сетевом порядке!!!
	local_addr.sin_port = htons(MY_PORT);
	// Сервер принимает подключения
	// на все IP-адреса
	local_addr.sin_addr.s_addr = 0;

	// Вызываем bind для связывания
	if (bind(mysocket, (sockaddr *)&local_addr,
		sizeof(local_addr)))
	{
		// Ошибка!
		printf("Error bind %d\n", WSAGetLastError());
		// закрываем сокет!
		closesocket(mysocket);

	}

	// Ожидание подключений
	// размер очереди – 0x100
	if (listen(mysocket, 0x100))
	{
		// Ошибка!
		printf("Error listen %d\n", WSAGetLastError());
		closesocket(mysocket);
	}

	strcpy_s(tmp, "Ожидание подключений...\n");
	AnsiToOem(tmp, tmp);
	puts(tmp);

	//Извлекаем сообщение из очереди
	// сокет для клиента

	SOCKET client_socket;
	// адрес клиента
	// (заполняется системой)
	sockaddr_in client_addr;

	// Функции accept необходимо передать размер
	// структуры
	int client_addr_size = sizeof(client_addr);

	while ((client_socket = accept(mysocket, (sockaddr *)
		&client_addr, &client_addr_size)))
	{
		// Увеличиваем счетчик
		// подключившихся клиентов
		numOfClients++;

		// Пытаемся получить имя хоста
		HOSTENT *hst;
		hst = gethostbyaddr((char *)
			&client_addr.sin_addr.s_addr, 4, AF_INET);


		PRINTUSERS
			// Вызов нового потока для обслужвания
			// клиента. Да, для этого рекомендуется
			// использовать _beginthreadex но, 
			// поскольку никаких вызов функций 
			// стандартной Си библиотеки поток не
			// делает, можно обойтись и CreateThread
			DWORD thID;
		tagINF info;

		info.caddr = &client_addr;
		info.soc = &client_socket;
		// При каждом новом коннекте 
		// создается новый серверный поток, 

		HANDLE hID = CreateThread(NULL, NULL, Client,
			&info, NULL, &thID);
		arrHandle.push_back(hID);

	}

	deinitialize();

	return 0;
}

void deinitialize() {
	// Деиницилизация библиотеки Winsock
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

	if (critSectFree) {
		//	strcpy_s(buff, "GRANTED");
		critSectFree = false;
		send(my_sock, &granted[0], sizeof(buff) - 1, 0);

		DuplicateHandle(
			GetCurrentProcess(),
			GetCurrentThread(),
			GetCurrentProcess(),
			&hID,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		std::ofstream out("D:\\hello.txt", std::ios::app);
		// ПишеМ в общий для всех 
		// серверных потоков строковый буфер 
		// (buffer) строку вида “[%d]: accept new client 
		// %s\n”, где %d –дескриптор потока, 
		// %s – IP-адрес клиента.  
		EnterCriticalSection(&critSect);
		//buffer << "[" << hID << "]" << ":accept new client " << inet_ntoa(my_addr.sin_addr) << "\n";
		//printf("[%d]: accept new client %s\n", hID, inet_ntoa(my_addr.sin_addr));
		LeaveCriticalSection(&critSect);

		HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
		LARGE_INTEGER li;
		int nNanosecondsPerSecond = 1000000;
		__int64 qwTimeFromNowInNanoseconds = nNanosecondsPerSecond;

		qwTimeFromNowInNanoseconds = -qwTimeFromNowInNanoseconds,
			li.LowPart = (DWORD)(qwTimeFromNowInNanoseconds & 0xFFFFFFFF),
			li.HighPart = (LONG)(qwTimeFromNowInNanoseconds >> 32);

		SetWaitableTimer(hTimer, &li, TIMEOUT, timer_proc, (LPVOID)hID, FALSE);

		int bytes_recv;
		// Цикл эхо-сервера: прием строки от клиента 
		// и возвращение ее клиенту


		while ((bytes_recv = recv(my_sock, &buff[0], sizeof(buff), 0))
			&& bytes_recv != SOCKET_ERROR) {
			//При получении сигнала INT сервер делает дамп буфера (buffer) во временный файл в каталоге /tmp и сообщает имя файла в stdout, после чего буфер (buffer) обнуляется.

			EnterCriticalSection(&critSect);
			buffer << buff << "\n";
			printf("recive and send string: %s\n", buff);  //вывод строки, которую передал клиент!
			
			//тут запись в файл
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
	//При дисконнекте клиента сервер пишет 
	//в буфер (buffer) строку вида “[%d]: 
	//client %s disconnected\n”, 
	//где %d – дескриптор потока,
	//%s – IP адрес клиента, связь с которым 
	//была прекращена. 
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

VOID APIENTRY  timer_proc(LPVOID IpArgToCompletionRoutine,
	DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	EnterCriticalSection(&critSect);
	printf("get connection\n", IpArgToCompletionRoutine); //[%d]: idle\n
	buffer << "[" << IpArgToCompletionRoutine << "]: idle\n";
	LeaveCriticalSection(&critSect);
};