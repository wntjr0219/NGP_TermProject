#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include "protocol.h"
#include "Common.h"
#include "OpenGL.h"
#include "GameUtilities.h"
// 12�� 6�� ����˼�
// 12�� 12,13�� �����˼�
//-----------------------�ؾ��� �͵�
// Ŭ���̾�Ʈ�� enemydraw�Լ� �߰�, ������ enemy�� ����� ó�� �߰�
// reStart, Pause �Լ� ó��
// ������� �ľ��ϴ� �� / � ���� �����������(ip�̿�)
// ��Ŷ�ް� ��ŷ ���Ͼ���/�б�/��� �׽�Ʈ
// ���� �׽�Ʈ
//------------------------Common.h�� �ִ°͵�
//void err_quit(const char* s) { }
#define SERVERPORT 4500
//-------------------------�Լ�����
void writeRankInfoFile(const char* filename, const RankedInfo* rankInfo, int meter);
void readRankInfoFile(const char* filename, RankedInfo*& rankInfo, int*& meter);
void initPlayer();
void initObstacle();
void initGamePlayer();
bool isDead();
void OverGame(SOCKET sock);
void RecvProcess(SOCKET sock);
void setRankedInfo(SOCKET sock);
void moveCharacter(int keytype);
void sendObstaclePacket(SOCKET sock);
void sendCharacterPacket(SOCKET sock);
void sendEnemyPacket(SOCKET sock);
void reStart();
//------------------------��������(�����ڿ�����)
int numOfClient = 0;
HANDLE moveEvent;
HANDLE playerEvent;
const char* filename;
SCRankingPacket rankingPacket;
//POSXYZ player0Pos;
//POSXYZ player1Pos;
RankedInfo rankInfo;
SCObstaclePacket Obs;
PLAYER player0, player1;

struct PLAYER {
	POSXYZ Pos;
	bool isCollide;
};

struct savingIP {
	SOCKET sock;
	char ip[STRLEN];
	bool winner = false;
};

savingIP playersINFO[2];

//------------------------Thread ����
// ���⼭�� �� ó���� ���ΰ�
// Obstacle Thread�� �̱� ��Ƽ�� ��� ���� ���ΰ�? -> ���� ���� : ������ Supend�� �ڽ� ������ ����
DWORD WINAPI MoveThread(LPVOID arg) {
	
	//SOCKET obsSock = (SOCKET)arg;
	
	while (1) {
		Character.isCollide = false;
		if (jumpRunning) {
				std::thread([] {
					std::this_thread::sleep_for(std::chrono::milliseconds(30));
					sphere_jump_timer(1);
					}).detach();
			}

			if (hideRunning) {
				std::thread([] {
					std::this_thread::sleep_for(std::chrono::milliseconds(30));
					sphere_hide_timer(1);
					}).detach();
			}
		std::thread([] {
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			cube_move_timer(1);
			}).detach();

			

		Sleep(30);
		SetEvent(moveEvent);
	}
	
	return 0;
}

//���⼭ �� ó���� ���ΰ�
DWORD WINAPI PlayerThread(LPVOID arg) {
	DWORD retval;
	SOCKET sock = (SOCKET)arg;

	if (playersINFO[0].sock == sock) {
		PLAYER player = player0;
		PLAYER Enemy = player1;
	}
	else {
		PLAYER player = player1;
		PLAYER Enemy = player0;
	}

	while (1) {
		retval = WaitForSingleObject(moveEvent, INFINITE);
		
		sendObstaclePacket(sock);

		RecvProcess(sock);

		sendCharacterPacket(sock, player);
		sendEnemyPacket(sock, Enemy);
		
		if (isDead()) { OverGame(sock); }

		Sleep(65);
		SetEvent(playerEvent);
	}

	return 0;
}

void saveIP(SOCKET& clisock){

	sockaddr_in addr;
	int addrLen = sizeof(addr);
	if (getpeername(clisock, (sockaddr*)&addr, &addrLen) == 0) {

		if (playersINFO[0].ip == NULL) {
			inet_ntop(AF_INET, &addr.sin_addr, playersINFO[0].ip, sizeof(addr));

		}
		else {
			inet_ntop(AF_INET, &addr.sin_addr, playersINFO[1].ip, sizeof(addr));
		}
	}
}

//------------------------ Main �Լ�

int main(int argc, char** argv) {
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

	// �̺�Ʈ ����
	moveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	playerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// �÷��̾� ��ġ, ��ֹ� ��ġ �ʱ�ȭ
	glutInit(&argc, argv); // glut �ʱ�ȭ

	while (1) {
		if (playersINFO[0].winner == false and playersINFO[1].winner == false) {
			if (playersINFO[0].sock == NULL) playersINFO[0].sock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);
			else playersINFO[1].sock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);
		}
		else { // ������ �ű�� ip��ġ 0���� ������ : ���������� ���������� ���
			if(playersINFO[0].winner == true) {}
			else {}
		}
		saveIP(clisock);
		initGamePlayer();

		HANDLE hThread[3];
		//movethread�� �ѹ��� �����Ǿ���.
		hThread[0] = CreateThread(NULL, 0, MoveThread, 0, 0, NULL);
		hThread[1] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);
		if (playersINFO[1].ip != NULL) {
			hThread[2] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);
		}

		CloseHandle(hThread[0]);
		CloseHandle(hThread[1]);
		printf("main");
	}

	CloseHandle(moveEvent);
	CloseHandle(playerEvent);

	closesocket(clisock);
	closesocket(sersock);

	WSACleanup();

	glutMainLoop();
}


void moveCharacter(int keytype, POSXYZ& player) 
{
	switch (keytype)
	{
	case KEYUP:
		if (jumpRunning == false) {
			jumped = true;
			jumpRunning = true;
			printf("Keyup\n");
		}
		break;
	case KEYDOWN:
		if (hideRunning == false) {
			hide = true;
			hideRunning = true;
			printf("Keydown\n");
		}
		break;
	case KEYLEFT:
		if (!(player.posX <= -3.0)) {
			player.posX -= 1.0;
			printf("recv LEFT\n");
			printf("charcter posX : %f\n", player.posX);
		}
		break;
	case KEYRIGHT:
		if (!(player.posX >= 3.0)) {
			player.posX += 1.0;
			printf("recv RIGHT\n");
			printf("charcter posX : %f\n", player.posX);
		}
		break;
	default: 
		break;
	}

}

void setRankedInfo(SOCKET sock)
{
	//place aggregation
	filename = "rankingFile.bin";
	RankedInfo* rankInfo = new RankedInfo[RANKERS];
	int* meters = new int[RANKERS];
	readRankInfoFile(filename, rankInfo, meters);

	rankingPacket.type = SCRANKINGPACKET;
	for (int i = 0; i < RANKERS; ++i) {
		rankingPacket.rankings[i] = rankInfo[i];
	}
	
	send(sock, (char*)&rankingPacket, sizeof(rankingPacket), MSG_WAITALL);
	delete[] rankInfo;
}


void RecvProcess(SOCKET sock) {
	
	if(playersINFO[0].sock == sock ){
		POSXYZ* player = player0Pos;
	}
	else {
		POSXYZ* player = player1Pos;
	}
	u_long mode = 1;
	if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("error code : %d\n", WSAGetLastError());
	}
	
	BYTE type = 0;
	int ret = recv(sock, (char*)&type, sizeof(BYTE), MSG_PEEK);

	if (ret > 0) {
		switch (type)
		{
		case CSINITIALPACKET:
			CSInitialPacket Initial;
			recv(sock, (char*)&Initial, sizeof(CSInitialPacket), MSG_WAITALL);
			writeRankInfoFile("rankingFile.bin", (RankedInfo*)Initial.nameInitial, Initial.meter);
			setRankedInfo(sock);
			break;
		case CSKEYPACKET:
			CSKeyPacket Move;
			recv(sock, (char*)&Move, sizeof(CSKeyPacket), 0);
			printf("keypacket\n");
			moveCharacter(Move.keytype, player);
			break;
		case CSRESUMEPACKET:
			CSResumePacket Resume;
			recv(sock, (char*)&Resume, sizeof(CSResumePacket), MSG_WAITALL);
			reStart();
			break;
		default:
			std::cout << "invalid Packet" << std::endl;
			break;
		}
	}

}

void writeRankInfoFile(const char* filename, const RankedInfo* rankInfo, int meter)
{
	//meter write �߰�
	std::ofstream file(filename, std::ios::binary | std::ios::app);
	if (!file.is_open()) {
		std::cerr << "file write open error" << std::endl;
		return;
	}
	file.write(reinterpret_cast<const char*>(&rankInfo), sizeof(RankedInfo));
	file.close();
}

void readRankInfoFile(const char* filename, RankedInfo*& rankInfo, int*& meter)
{
	//meter read ó�� �߰�
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

	int numCopyElements = std::min(RANKERS, static_cast<int>(allRankings.size()));

	for (int i = 0; i < numCopyElements; ++i) {
		rankInfo[i] = allRankings[i];
	}

	file.close();
}

void initPlayer()
{
	// �÷��̾� ��ġ ����� �ʱ�ȭ (���� ���߾� ��ġ)
	Character.characterXYZ.posX = 0.0;
	Character.characterXYZ.posY = 0.4;
	Character.characterXYZ.posZ = 3.0;
	Character.isCollide = false;
	Character.type = SCCHARACTERPACKET;
}

void initObstacle()
{
	// ��ֹ� ��ġ ������������ �ʱ�ȭ
	Obstacles.reserve(OBSTACLES);
	int i;

	// cube
	for (i = 0; i < 5; ++i) {
		Obstacles.emplace_back(0.5, 1.0, 2, 29);
	}
	// normalCube
	for (i; i < 10; ++i) {
		Obstacles.emplace_back(0.5, 1.0, 2);
	}
	// hardCube
	for (i; i < 15; ++i) {
		Obstacles.emplace_back(1.0, 2.0, 2);
	}
	// hard2Cube
	for (i; i < 18; ++i) {
		Obstacles.emplace_back(4.0, 1.0, 4);
	}
}

void initGamePlayer()
{
	Obstacles.clear();
	srand(time(NULL));
	//���� �ʱ�ȭ�� �ʿ��� �Լ� �߰�
	initPlayer();
	initObstacle();
}

bool isDead() {
	return Character.isCollide > 10;
	//return player_hp > 2.0;
}

void sphere_jump_timer(int value)
{
	if (jumped == true && Character.characterXYZ.posY < 2.6) {
		Character.characterXYZ.posY += 0.2;
		if (Character.characterXYZ.posY >= 2.6) {
			jumped = false;
			falling = true;
		}
	}
	else if (falling == true) {
		Character.characterXYZ.posY -= 0.2;
		if (Character.characterXYZ.posY <= 0.4) {
			falling = false;
		}
	}
	else if (falling == false) {
		Character.characterXYZ.posY = 0.4;
		jumpRunning = false;
		//mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL);    //���� ��� ��ġ�� ó������ �ʱ�ȭ
	}
}

void sphere_hide_timer(int value)
{
	if (hide == true && Character.characterXYZ.posY >= -2.0) {
		Character.characterXYZ.posY -= 0.35;
		if (Character.characterXYZ.posY <= -2.0) {
			hide = false;
			unhide = true;
		}
	}
	else if (unhide == true) {
		Character.characterXYZ.posY += 0.35;
		if (Character.characterXYZ.posY >= 0.4) {
			unhide = false;
		}
	}
	else if (unhide == false) {
		Character.characterXYZ.posY = 0.4;
		hideRunning = false;
		//mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL);
	}
}

void cube_move_timer(int value)
{
	for(int i = 0 ; i < Obstacles.size(); ++ i) {
		Obstacles[i].move(i, meter);
		Obstacles[i].reSetObstacle(i);

		Obs.obstacleXYZ[i] = Obstacles[i].mPos;
		if (Obstacles[i].collide()) {
			Character.isCollide = true;
			
		}
	}
	meter++;
}

void OverGame(SOCKET sock)
{
	SCPausePacket pause;
	pause.seconds = 5;
	pause.type = SCPAUSEPACKET;
	send(sock, (char*)&pause, sizeof(SCPausePacket), MSG_WAITALL);

	SCWinnerPacket Winner;
	Winner.type = SCWINNERPACKET;
	Winner.winner = false;
	send(sock, (char*)&Winner, sizeof(SCWinnerPacket), MSG_WAITALL);
}

void sendObstaclePacket(SOCKET sock)
{
	Obs.type = SCOBSTACLEPACKET;

	int retval = send(sock, (char*)&Obs, sizeof(SCObstaclePacket), 0); // obstacle��Ŷ send
	if (retval == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
			printf("\n%d\n", error);
		}
		//exit(-1);
	}

}

void sendCharacterPacket(SOCKET sock)
{
	SCCharacterPacket Character;
	Character.type = SCCHARACTERPACKET;

	int retval = send(sock, (char*)&Character, sizeof(SCCharacterPacket), 0); // character��Ŷ send
	if (retval == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
			printf("\n%d\n", error);
		}
		//exit(-1);
	}
}

void sendEnemyPacket(SOCKET sock)
{
	//int retval = send(sock, (char*)&Enemy, sizeof(SCEnemyPacket), 0); // enemy��Ŷ send
	//if (retval == SOCKET_ERROR) {
	//	int error = WSAGetLastError();
	//	if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
	//		printf("\n%d\n", error);
	//	}
	//	//exit(-1);
	//}
}


void reStart()
{
	//game restart
}