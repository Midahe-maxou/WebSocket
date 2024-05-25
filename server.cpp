#include <iostream>
#include <string>
#include <tchar.h>
#include <limits>


#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <WS2tcpip.h>

#include <Windows.h>
#define NOMINMAX

using std::string;

#define DEFAULTPORT 5535
#define BUFFERSIZE 24

int main(int argc, char** argv)
{
	
	int port = (argc > 1) ? std::stoi(argv[1]) : DEFAULTPORT;

	std::cout << argc << std::endl;
	SetConsoleOutputCP(1258);


	WSADATA WData;

	int wsaerr = WSAStartup(MAKEWORD(2, 2), &WData);
	if (wsaerr != 0)
	{
		std::cout << "Winsock dll introuvable" << std::endl;
		WSACleanup();
		return -1;
	}
	std::cout << "Winsock dll chargée: " << WData.szSystemStatus << std::endl;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		std::cout << "Socket Erreur: " << sock << std::endl;
		std::cout << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}
	std::cout << "Socket créé: " << sock << std::endl;

	sockaddr_in server;
	server.sin_family = AF_INET;
	InetPton(AF_INET, _T("127.0.0.1"), &server.sin_addr.S_un.S_addr);
	server.sin_port = htons(port);
	
	if (bind(sock, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		std::cout << "Bind Erreur: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	std::cout << "Binding effectué, port " << port << std::endl;
	

	/* ----- for TCP only ----- */

	/*if (listen(sock, 1) == SOCKET_ERROR)
	{
		std::cout << "Erreur lors de l'écoute du socket " << sock << std::endl;
		std::cout << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	std::cout << "Écoute sur le socket " << sock << std::endl;


	/*SOCKET acceptSock = accept(sock, NULL, NULL);

	if (acceptSock == INVALID_SOCKET)
	{
		std::cout << "L'acceptation à échouée " << std::endl;
		closesocket(acceptSock);
		closesocket(sock);
		WSACleanup();
		return -1;
	}*/


	/*<>*/ SOCKET acceptSock = sock; //UPD purposes only

	std::cout << "Connection au client avec le socket " << acceptSock << std::endl;

	char header[BUFFERSIZE + 1];
	recv(acceptSock, header, BUFFERSIZE, NULL);
	
	int len = 0;

	for (int i = 0; i < 8; i++)
	{
		std::cout << header[i] << std::endl;
		len += header[i] << (8 * i); // parse the 8Bytes header into the lenght of the data
	}

	int nb_packets = (len / BUFFERSIZE) + 1;

	std::cout << "Taille des données à recevoir: " << len << std::endl;
	std::cout << "Nombre de packets à recevoir: " << nb_packets << std::endl;

	std::string all_data;
	char data[BUFFERSIZE + 1];
	std::fill(std::begin(data), std::end(data), '\0'); //reset data
	short fail_count = 0;

	int recverr = recv(acceptSock, data, BUFFERSIZE, NULL);

	while (string(data) != "EOF" && fail_count < 5) {

		if (string(data) == "EOF") break;

		if (recverr == SOCKET_ERROR)
		{
			std::cout << "le message n'a pas été reçu" << std::endl;
			std::cout << "Erreur: " << WSAGetLastError() << std::endl;
			all_data.append(BUFFERSIZE, '?');
			fail_count++;
		}
		else
		{
			std::cout << "voici le message: \"" << data << '"' << std::endl;
			all_data.append(string(data));
		}

		std::fill(std::begin(data), std::end(data), '\0'); //reset data
		recverr = recv(acceptSock, data, BUFFERSIZE, NULL);
	}

	std::cout << "Données reçus: " << all_data << std::endl;

	closesocket(sock);
	WSACleanup();

	return 0x0;
}


char fromCharToUchar(char uc)
{
	int code = (int)uc;
	if (code < 0) code += 127;
	return char(code);
}