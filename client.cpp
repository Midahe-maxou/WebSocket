#include <iostream>
#include <string>
#include <tchar.h>
#include <stdlib.h>
#include <algorithm>

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <WS2tcpip.h>

#include <Windows.h>
#define NOMINMAX

using std::string;

#define DEFAULTPORT 5535
#define BUFFERSIZE 24
#define LAST_DEFINED_CHAR (MAXCHAR - 1)

void sendData(const SOCKET& sock, const std::string& data);
char fromUcharToChar(unsigned char uc);


int main(int argc, char** argv)
{
	SetConsoleOutputCP(1258);

	int port = (argc > 1) ? std::stoi(argv[1]) : DEFAULTPORT;

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

	sockaddr_in client;
	client.sin_family = AF_INET;
	InetPton(AF_INET, _T("46.193.4.138"), &client.sin_addr.S_un.S_addr);
	client.sin_port = htons(port);
	
	if (connect(sock, (sockaddr*)&client, sizeof(client)) == SOCKET_ERROR)
	{
		std::cout << "connect Erreur: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	std::cout << "connection effectué, port " << port << std::endl;

	std::string data;

	std::cout << "Message à envoyer: ";
	std::getline(std::cin, data);

	std::cout << "sending " << data << "..." << std::endl;

	sendData(sock, data);


	closesocket(sock);
	WSACleanup();

	return 0x0;
}

void sendData(const SOCKET& sock, const std::string& data) // abcdefghijklmnopqrstuvwxyz
{
	size_t len = data.length();

	std::cout << "Bytes à envoyer: " << len << ", nombre de packets: " << (len / BUFFERSIZE) + 1 << std::endl;


	/** creating the header package **/
	char header[BUFFERSIZE];
	std::fill(std::begin(header), std::end(header), '\0');
	
	for (int i = 0; i < 8; i++)
	{
		header[i] = fromUcharToChar((len >> (8 * i)) & 0xFF); //split len into 8 packets of 8 bits each
	}

	std::cout << '"';
	std::for_each(std::begin(header), std::end(header), [](const char& c) { std::cout << c; });
	std::cout << '"' << std::endl;

	short headererr = 0;
	while (send(sock, header, BUFFERSIZE, NULL) == 0 && headererr < 5) headererr++;
	if (headererr == 5)
	{
		std::cout << "Erreur lors de l'envoi du header" << '\n';
		std::cout << "Annulation de l'envoi du message" << std::endl;
		std::cout << "Erreur: " << WSAGetLastError() << std::endl;
		return;
	}


	/** sending data */
	std::string::const_iterator it_beg = std::cbegin(data);
	std::string::const_iterator it_end = std::cend(data);
	for (size_t i = 0; i < (len / BUFFERSIZE) + 1; i++)
	{
		std::string::const_iterator beg = it_beg + (BUFFERSIZE*i);
		std::string::const_iterator end = ( ((i + 1) * BUFFERSIZE < len) ) ? (beg + BUFFERSIZE) : (beg + len % BUFFERSIZE); // prevent segfault while accessing the last packet
		std::string splitted_data = { beg, end };

		int sentbyte = send(sock, splitted_data.c_str(), BUFFERSIZE, NULL);

		if (sentbyte == SOCKET_ERROR)
		{
			std::cout << "Packet " << i+1 << ": donnée non transmise \"" << splitted_data << '"' << std::endl;
		}
		else
		{
			std::cout << "Packet " << i+1 << ": donnée transmise: \"" << splitted_data << '"' << std::endl;
		}
	}

	/** sending EOF */
	short fail_count = 0;
	while (send(sock, "EOF", BUFFERSIZE, NULL) == SOCKET_ERROR && fail_count < 5)
	{ 
		std::cout << "Échec EOF" << std::endl;
		fail_count++;
	}

	std::cout << "Fin de transmission" << std::endl;
}

char fromUcharToChar(unsigned char uc)
{
	int code = (int)uc;
	if (code > 127) code -= 256;
	return char(code);
}