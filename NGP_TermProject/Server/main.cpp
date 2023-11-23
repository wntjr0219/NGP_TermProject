#include <iostream>
#include <thread>
#include "protocol.h"
//------------------------Common.h�� �ִ°͵�
void err_quit(const char* s) { }
#define SERVERPORT 4500
//-------------------------�Լ�����
bool isDead();
void OverGame();
void RecvProcess(SOCKET& sock);
void setRankedInfo(int meter, char* Inintial);
void moveCharacter(int keytype);
void reStart();
//------------------------��������(�����ڿ�����)
int numOfClient = 0;


//------------------------Thread ����

//���⼭ �� ó���� ���ΰ�
DWORD WINAPI ClientThread(LPVOID arg) {

	SOCKET sock = (SOCKET)arg;

	while (1) {

		RecvProcess(sock);

		bool deadflag = isDead();
		if (deadflag == true) { OverGame(); }

	}

	return 0;
}

// ���⼭�� �� ó���� ���ΰ�
// Obstacle Thread�� �̱� ��Ƽ�� ��� ���� ���ΰ�? -> ���� ���� : ������ Supend�� �ڽ� ������ ����
DWORD WINAPI MoveThread(LPVOID arg) {

	SOCKET obsSock = (SOCKET)arg;

	// ��ֹ� ��ġ�� ���� �ϴ� ����
	while (1) {
		//send();

	}
	return 0;
}

//------------------------ Main �Լ�

int main(void) {


	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET sersock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sersock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN seraddr;
	memset(&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	seraddr.sin_port = htons(SERVERPORT);

	if (SOCKET_ERROR == bind(sersock, (SOCKADDR*)&seraddr, sizeof(seraddr))) err_quit("bind()");

	if (listen(sersock, SOMAXCONN) == SOCKET_ERROR) err_quit("listen()");

	SOCKET clisock;
	SOCKADDR_IN cliaddr;
	int addrlen = sizeof(cliaddr);



	while (1) {

		clisock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);

		HANDLE MovThread = CreateThread(NULL, 0, MoveThread, 0, 0, NULL);




	}

	closesocket(clisock);
	closesocket(sersock);

	WSACleanup();

}


void moveCharacter(int keytype) {
	//Collsion Detection
}



void setRankedInfo(int meter, char* Initial) {
	//place aggregation
}
void reStart() {
	//game restart
}


void RecvProcess(SOCKET& sock) {
	BYTE type;
	int ret = recv(sock, (char*)type, sizeof(BYTE), MSG_PEEK);
	if (ret == SOCKET_ERROR) { exit(-1); }
	switch (type)
	{
	case CSINITIALPACKET:
		CSInitialPacket Initial;
		recv(sock, (char*)&Initial, sizeof(CSInitialPacket), MSG_WAITALL);
		setRankedInfo(Initial.meter, Initial.nameInitial);
		break;
	case CSKEYPACKET:
		CSKeyPacket Move;
		recv(sock, (char*)&Move, sizeof(CSKeyPacket), MSG_WAITALL);
		moveCharacter(Move.keytype);
		break;
	case CSRESTARTPACKET:
		CSReStartPacket Restart;
		recv(sock, (char*)&Restart, sizeof(CSReStartPacket), MSG_WAITALL);
		reStart();
		break;
	default:
		std::cout << "invalid Packet" << std::endl;
		exit(-1);
		break;
	}

}

bool isDead() {

}

void OverGame() {

}