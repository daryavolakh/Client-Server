
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

#define MY_PORT 5223		  // Порт, который слушает сервер
#define TIMEOUT 1000

std::ostringstream buffer;		  // Строковый буфер

std::vector<HANDLE> arrHandle;

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
unsigned __stdcall Client(LPVOID info);

// Глобальная переменная – количество
// активных пользователей 
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

	char tmp[1000];			  // Буфер для различных нужд
	char buff[1024];		  // Буфер для различных нужд

	printf("TCP SERVER \n");

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
	// Деиницилизация библиотеки Winsock
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

