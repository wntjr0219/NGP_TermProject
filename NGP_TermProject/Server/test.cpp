#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include "protocol.h"
#include "Common.h"
//------------------------Common.h�� �ִ°͵�
void err_quit(const char* s) { }
#define SERVERPORT 4500
//-------------------------�Լ�����
void writeRankInfoFile(const char* filename, const RankedInfo* rankInfo);
void readRankInfoFile(const char* filename, RankedInfo*& rankInfo);
void initPlayer();
void initObstacle();
void initGamePlayer();
bool isDead();
void OverGame();
void RecvProcess(SOCKET& sock);
void setRankedInfo(int meter, char* Inintial, SOCKET sock);
void moveCharacter(int keytype);
void reStart();
//------------------------��������(�����ڿ�����)
int numOfClient = 0;
const char* filename;
SCRankingPacket rankingPacket;
POSXYZ playerPos;
POSXYZ cubePos[5];
POSXYZ normalCubePos[5];
POSXYZ hardCubePos[5];
POSXYZ hardCube2Pos[3];
RankedInfo rankInfo;
float player_hp;

//------------------------Thread ����

//���⼭ �� ó���� ���ΰ�
DWORD WINAPI ClientThread(LPVOID arg) {

	SOCKET sock = (SOCKET)arg;

	while (1) {

		RecvProcess(sock);

		/*bool deadflag = isDead();
		if (deadflag == true) { OverGame(); }*/
		if (isDead()) { OverGame(); }

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


void moveCharacter(int keytype) 
{
	//Collsion Detection
}



void setRankedInfo(int meter, char* Initial, SOCKET sock)
{
	//place aggregation
	filename = "rankingFile.bin";
	RankedInfo* rankInfo = new RankedInfo[RANKERS];
	readRankInfoFile(filename, rankInfo);

	rankingPacket.type = SCRANKINGPACKET;
	for (int i = 0; i < RANKERS; ++i) {
		rankingPacket.rankings[i] = rankInfo[i];
	}
	
	send(sock, (char*)&rankingPacket, sizeof(rankingPacket), MSG_WAITALL);
	delete[] rankInfo;
}
void reStart() 
{
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
		setRankedInfo(Initial.meter, Initial.nameInitial, sock);
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

void writeRankInfoFile(const char* filename, const RankedInfo* rankInfo)
{
	std::ofstream file(filename, std::ios::binary | std::ios::app);
	if (!file.is_open()) {
		std::cerr << "file write open error" << std::endl;
		return;
	}
	file.write(reinterpret_cast<const char*>(&rankInfo), sizeof(RankedInfo));
	file.close();
}

void readRankInfoFile(const char* filename, RankedInfo*& rankInfo)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "file read open error" << std::endl;
		return;
	}

	std::vector<RankedInfo> allRankings;
	while (file.peek() != EOF) {
		RankedInfo temp;
		file.read(reinterpret_cast<char*>(&temp), sizeof(RankedInfo));
		allRankings.push_back(temp);
	}

	std::sort(allRankings.begin(), allRankings.end(), [](const RankedInfo& a, const RankedInfo& b) {
		return a.meter > b.meter;
		});

	int numCopyElements = min(RANKERS, static_cast<int>(allRankings.size()));

	for (int i = 0; i < numCopyElements; ++i) {
		rankInfo[i] = allRankings[i];
	}

	file.close();
}

void initPlayer()
{
	// �÷��̾� ��ġ ����� �ʱ�ȭ (���� ���߾� ��ġ)
	playerPos.posX = 0.0;
	playerPos.posY = 0.4;
	playerPos.posZ = 3.0;
}

void initObstacle()
{
	// ��ֹ� ��ġ ������������ �ʱ�ȭ
	for (int i = 0; i < 5; i++) {
		cubePos[i].posX = (float)(rand() % 7);
		cubePos[i].posY = (float)(rand() % 2);
		cubePos[i].posZ = -(float)(rand() % 100 + 29);
	}
	for (int i = 0; i < 5; i++) {
		normalCubePos[i].posX = (float)(rand() % 7);
		normalCubePos[i].posY = (float)(rand() % 2);
		normalCubePos[i].posZ = -(float)(rand() % 100 + 40);
	}
	for (int i = 0; i < 5; i++) {
		hardCubePos[i].posX = (float)(rand() % 7);
		hardCubePos[i].posY = (float)(rand() % 2);
		hardCubePos[i].posZ = -(float)(rand() % 100 + 40);
	}
	for (int i = 0; i < 3; i++) {
		hardCube2Pos[i].posX = 3.0;
		hardCube2Pos[i].posY = (float)(rand() % 4);
		hardCube2Pos[i].posZ = -(float)(rand() % 200 + 40);
	}
}

void initGamePlayer()
{
	//���� �ʱ�ȭ�� �ʿ��� �Լ� �߰�
	initPlayer();
	initObstacle();
}

bool isDead() {
	return player_hp > 2.0;
}

void OverGame() {

}