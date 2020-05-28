#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

int main()
{

	const unsigned short port = 54000;

	// intialize winsocket
	WORD version = MAKEWORD(2, 2); // word is unsigned short / two bytes
	WSADATA wsaData;
	int wsstatus = WSAStartup(version, &wsaData); // returns 0 for success
	if (wsstatus != 0) {
		std::cerr << "Winsocket startup failed" << std::endl;
		return 1;
	}

	// open socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		std::cerr << "Cannot open socket" << std::endl;
		return 1;
	}

	// Bind ip and port to socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port); // host to network - returns unsigned short
	hint.sin_addr.s_addr = INADDR_ANY; // s_addr is a macro for S_un.S_addr

	bind(listening, (sockaddr*)&hint, sizeof(hint));
	listen(listening, SOMAXCONN); // second argument is for backlog: The maximum length of the queue of pending connections. 

	std::cout << "Listening on port 54000" << std::endl;

	// client socket
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize); // client is optional and receives data on connection
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Client socket cannot be openedd" << std::endl;
		return 1;
	}

	// allocate memory for client data
	char host[NI_MAXHOST]; // string for client host name 
	char service[NI_MAXHOST]; // client port
	ZeroMemory(host, NI_MAXHOST); // Windows ... - memset(
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, clientSize, host, NI_MAXHOST, service, NI_MAXHOST, 0)) {
		std::cout << host << "connected on port " << service << std::endl;
	}
	else { // cannot get named info
		inet_ntop(AF_INET, &(client.sin_addr), host, NI_MAXHOST);
		std::cout << host << "ip connected on port " << ntohs(client.sin_port) << std::endl;
	}

	closesocket(listening);

	const int chunkSize = 4096;
	char buf[chunkSize];
	const char response[] = "thanks";

	while (true) {
		ZeroMemory(buf, chunkSize);
		int bytesReceived = recv(clientSocket, buf, chunkSize, 0);
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "Error receiving client data" << std::endl;
			break;
		}
		if (bytesReceived == 0) { // gracefully closed
			std::cout << "Connection closed" << std::endl;
			break;
		}

		if (buf[0] != 0) {
			std::cout << "Received: " << buf << std::endl;
			send(clientSocket, response, strlen(response) + 1, 0);
		}
	}

	closesocket(clientSocket);

	WSACleanup();
}