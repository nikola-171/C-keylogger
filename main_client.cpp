#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

void client(BYTE b1, BYTE b2, BYTE b3, BYTE b4, USHORT port) {
	const WORD WSA_VERSION = 0x202;
	WSADATA wsa;
	WSAStartup(WSA_VERSION, &wsa);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock == INVALID_SOCKET) {
		throw "SOCKET COULD NOT BE INITIALIZED! EXIT!";
		exit(1);
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.S_un.S_un_b = { b1, b2, b3, b4 };

	

	int res = connect(sock, (sockaddr*)&address, sizeof(address));

	/*ODAVDE KRECE*/
	int iResult = 0;

	BOOL bOptVal = FALSE;
	int bOptLen = sizeof(BOOL);

	int iOptVal = 0;
	int iOptLen = sizeof(int);
	bOptVal = TRUE;

	iResult = getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"getsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
	}
	else
		wprintf(L"SO_KEEPALIVE Value: %ld\n", iOptVal);

	iResult = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"setsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
	}
	else
		wprintf(L"Set SO_KEEPALIVE: ON\n");

	iResult = getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"getsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
	}
	else
		wprintf(L"SO_KEEPALIVE Value: %ld\n", iOptVal);

	/*DO OVDEE*/

	if (res != 0) {
		throw "COULD NOT MAKE CONNECTION, EXITING!";
		exit(1);
		
	}
	
	int n = 125;
	std::vector<char> buff(n);

	while (1) {
		int bytes_read = recv(sock, buff.data(), n, 0);
		if (bytes_read == 0) {
			break;
		}

		for (int i = 0; i < bytes_read; i++) {
			std::cout << buff.at(i);
		}
		std::cout << '\n';
		
	}
	int close = closesocket(sock);
	if (close != 0) {
		throw "ERROR WHILE CLOSING SOCKET!";
		exit(1);
	}
	
}

int main() {

	std::cout << "ENTER IP ADDRESS AND PORT SEPARATED VIA SPACE:" << '\n';
	USHORT b1, b2, b3, b4, port;
	std::cin >> b1 >> b2 >> b3 >> b4 >> port;

	std::cout << b1 << "," << b2 << "," << b3 << "," << b4 << "," << port << '\n';


	try {
		client((BYTE)b1, (BYTE)b2, (BYTE)b3, (BYTE)b4, port);
	}catch (const char* exception) {
		std::cout << "excpetion : " << exception << '\n';
	}
	catch (...) {
		std::cout << "error " << '\n';
	}
	
	return 0;
}