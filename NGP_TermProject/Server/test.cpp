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
// 12월 6일 조기검수
// 12월 12,13일 최종검수
//-----------------------해야할 것들
// 클라이언트에 enemydraw함수 추가, 서버에 enemy인 경우의 처리 추가
// reStart, Pause 함수 처리
// 몇명인지 파악하는 거 / 어떤 곳에 보내줘야할지(ip이용)
// 패킷받고 랭킹 파일쓰기/읽기/출력 테스트
// 최종 테스트
//------------------------Common.h에 있는것들
//void err_quit(const char* s) { }
#define SERVERPORT 4500
//--------------------------------------------------
struct PLAYER {
	POSXYZ Pos;
	bool isCollide;
	int id;
};
//-------------------------함수선언
void writeRankInfoFile(const char* filename, const RankedInfo* rankInfo, int meter);
void readRankInfoFile(const char* filename, RankedInfo*& rankInfo, int*& meter);
void initPlayer();
void initObstacle();
void initGamePlayer();
bool isDead(PLAYER player);
void OverGame(SOCKET sock);
void RecvProcess(SOCKET sock, PLAYER player);
void setRankedInfo(SOCKET sock);
void moveCharacter(int keytype, PLAYER player);
void sendObstaclePacket(SOCKET sock);
void sendCharacterPacket(SOCKET sock, PLAYER player);
void sendEnemyPacket(SOCKET sock, PLAYER player);
void reStart();
//------------------------전역변수(공유자원포함)
int numOfClient = 0;
HANDLE moveEvent;
HANDLE playerEvent;
const char* filename;
SCRankingPacket rankingPacket;
//SCCharacterPacket Character;
RankedInfo rankInfo;
SCObstaclePacket Obs;
PLAYER player0, player1;

//POSXYZ playerPos;

struct savingIP {
	SOCKET sock;
	char ip[INET_ADDRSTRLEN];
	bool winner = false;
};

savingIP playersINFO[2];

//------------------------Thread 정의
// 여기서는 뭘 처리할 것인가
// Obstacle Thread를 싱글 멀티로 어떻게 나눌 것인가? -> 도성 제시 : 스레드 Supend후 자식 스레드 생성
DWORD WINAPI MoveThread(LPVOID arg) {

	while (1) {
		player0.isCollide = false;
		player1.isCollide = false;
		//Character.isCollide = false;
		if (jumpRunning0) {
			std::thread([] {
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
				sphere_jump_timer0(1);
				}).detach();
		}
		if (hideRunning0) {
			std::thread([] {
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
				sphere_hide_timer0(1);
				}).detach();
		}
		if (jumpRunning1) {
			std::thread([] {
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
				sphere_jump_timer1(1);
				}).detach();
		}
		if (hideRunning1) {
			std::thread([] {
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
				sphere_hide_timer1(1);
				}).detach();
		}
		
			
		std::thread([] {
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			cube_move_timer(1, player0.Pos, player1.Pos);
			}).detach();


		Sleep(30);
		SetEvent(moveEvent);
	}
	
	return 0;
}

//여기서 뭘 처리할 것인가
DWORD WINAPI PlayerThread(LPVOID arg) {
	DWORD retval;
	SOCKET sock = (SOCKET)arg;

	PLAYER player;
	PLAYER Enemy;

	u_long mode = 1;
	if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("error code : %d\n", WSAGetLastError());
	}
	while (1) {
		if (playersINFO[0].sock == sock) {	// 첫 번째로 들어온 유저인 경우의 PlayerThread
			player = player0;
			Enemy = player1;
		}
		else {	// 두 번째 들어온 유저인 경우의 PlayerThread
			player = player1;
			Enemy = player0;
		}


		retval = WaitForSingleObject(moveEvent, INFINITE);
		
		sendObstaclePacket(sock);

		RecvProcess(sock, player);

		sendCharacterPacket(sock, player);
		sendEnemyPacket(sock, Enemy);
		
		if (isDead(player)) { OverGame(sock); }

		Sleep(65);
		SetEvent(playerEvent);
	}

	return 0;
}

void saveIP(SOCKET& clisock) {

	sockaddr_in addr;
	int addrLen = sizeof(addr);
	if (getpeername(clisock, (sockaddr*)&addr, &addrLen) == 0) {
		if (playersINFO[0].ip[0] == '\0') {
			inet_ntop(AF_INET, &addr.sin_addr, playersINFO[0].ip, sizeof(addr));
			printf("ip : %s\n",playersINFO[0].ip);
		}
		else {
			inet_ntop(AF_INET, &addr.sin_addr, playersINFO[1].ip, sizeof(addr));
			printf("ip : %s\n", playersINFO[1].ip);
		}
	}
}

//------------------------ Main 함수

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

	// 이벤트 생성
	moveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	playerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 플레이어 위치, 장애물 위치 초기화
	glutInit(&argc, argv); // glut 초기화

	HANDLE hThread[3];
	hThread[0] = CreateThread(NULL, 0, MoveThread, 0, 0, NULL);
	CloseHandle(hThread[0]);

	while (1) {
		if (playersINFO[0].winner == false and playersINFO[1].winner == false) {
			if (playersINFO[0].sock == NULL) {
				playersINFO[0].sock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);
				playersINFO[0].ip[0] = '\0';
				printf("accept1\n");
				clisock = playersINFO[0].sock;
			}
			else if (playersINFO[1].sock == NULL) {
				playersINFO[1].sock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);
				playersINFO[1].ip[0] = '\0';
				clisock = playersINFO[1].sock;
			}
		}
		else { // 데이터 옮기고 ip위치 0으로 재조정 : 대전게임이 끝났을때의 경우
			if(playersINFO[0].winner == true) {}
			else {}
		}
		
		initGamePlayer();

		if (playersINFO[0].ip[0] == '\0') {
			printf("create 1\n");
			saveIP(clisock);
			hThread[1] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);
			CloseHandle(hThread[1]);
		}
		else if (playersINFO[1].ip[0] == '\0') {
			printf("create 2\n");
			saveIP(clisock);
			hThread[2] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);
			CloseHandle(hThread[2]);
		}
	}

	CloseHandle(moveEvent);
	CloseHandle(playerEvent);

	closesocket(clisock);
	closesocket(sersock);

	WSACleanup();

	glutMainLoop();
}


void moveCharacter(int keytype, PLAYER player) 
{
	switch (keytype)
	{
	case KEYUP:
		if (player.id == 0) {
			if (jumpRunning0 == false) {
				jumped0 = true;
				jumpRunning0 = true;
				printf("player0Keyup\n");
			}
		}
		else {
			if (jumpRunning1 == false) {
				jumped1 = true;
				jumpRunning1 = true;
				printf("Keyup\n");
			}
		}
		break;
	case KEYDOWN:
		if (player.id == 0) {
			if (hideRunning0 == false) {
				hide0 = true;
				hideRunning0 = true;
				printf("Keydown\n");
			}
		}
		else {
			if (hideRunning1 == false) {
				hide1 = true;
				hideRunning1 = true;
				printf("Keydown\n");
			}
		}
		break;
	case KEYLEFT:
		if (player.id == 0) {
			if (!(player0.Pos.posX <= -3.0)) {
				player0.Pos.posX -= 1.0;
				printf("recv LEFT\n");
			}
			break;
		}
		else {
			if (!(player1.Pos.posX <= -3.0)) {
				player1.Pos.posX -= 1.0;
				printf("recv LEFT\n");
			}
		}
	case KEYRIGHT:
		if (player.id == 0) {
			if (!(player0.Pos.posX >= 3.0)) {
				player0.Pos.posX += 1.0;
				printf("recv RIGHT\n");
			}
			break;
		}
		else {
			if (!(player1.Pos.posX >= 3.0)) {
				player1.Pos.posX += 1.0;
				printf("recv RIGHT\n");
			}
			break;
		}
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


void RecvProcess(SOCKET sock, PLAYER player) {

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
	//meter write 추가
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
	//meter read 처리 추가
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
	// 플레이어 위치 상수값 초기화 (맵의 정중앙 위치)
	/*Character.characterXYZ.posX = 0.0;
	Character.characterXYZ.posY = 0.4;
	Character.characterXYZ.posZ = 3.0;
	Character.isCollide = false;
	Character.type = SCCHARACTERPACKET;*/

	player0.Pos.posX = 0.0;
	player0.Pos.posY = 0.4;
	player0.Pos.posZ = 3.0;
	player0.isCollide = false;
	player0.id = 0;

	player1.Pos.posX = 0.0;
	player1.Pos.posY = 0.4;
	player1.Pos.posZ = 3.0;
	player1.isCollide = false;
	player1.id = 1;
}

void initObstacle()
{
	// 장애물 위치 랜덤값범위로 초기화
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
	//게임 초기화에 필요한 함수 추가
	initPlayer();
	initObstacle();
}

bool isDead(PLAYER player) {
	return player.isCollide > 10;
}

void sphere_jump_timer0(int value)
{
	// if player0 jump
	if (jumped0 == true && player0.Pos.posY < 2.6) {
		player0.Pos.posY += 0.2;
		if (player0.Pos.posY >= 2.6) {
			jumped0 = false;
			falling0 = true;
		}
	}
	else if (falling0 == true) {
		player0.Pos.posY -= 0.2;
		if (player0.Pos.posY <= 0.4) {
			falling0 = false;
		}
	}
	else if (falling0 == false) {
		player0.Pos.posY = 0.4;
		jumpRunning0 = false;
	}
}
void sphere_hide_timer0(int value)
{
	// if player0 hide
	if (hide0 == true && player0.Pos.posY >= -2.0) {
		player0.Pos.posY -= 0.35;
		if (player0.Pos.posY <= -2.0) {
			hide0 = false;
			unhide0 = true;
		}
	}
	else if (unhide0 == true) {
		player0.Pos.posY += 0.35;
		if (player0.Pos.posY >= 0.4) {
			unhide0 = false;
		}
	}
	else if (unhide0 == false) {
		player0.Pos.posY = 0.4;
		hideRunning0 = false;
	}
}
void sphere_jump_timer1(int value)
{
	// if player1 jump
	if (jumped1 == true && player1.Pos.posY < 2.6) {
		player1.Pos.posY += 0.2;
		if (player1.Pos.posY >= 2.6) {
			jumped1 = false;
			falling1 = true;
		}
	}
	else if (falling1 == true) {
		player1.Pos.posY -= 0.2;
		if (player1.Pos.posY <= 0.4) {
			falling1 = false;
		}
	}
	else if (falling1 == false) {
		player1.Pos.posY = 0.4;
		jumpRunning1 = false;
	}
}
void sphere_hide_timer1(int value)
{
	// if player1 hide
	if (hide1 == true && player1.Pos.posY >= -2.0) {
		player1.Pos.posY -= 0.35;
		if (player1.Pos.posY <= -2.0) {
			hide1 = false;
			unhide1 = true;
		}
	}
	else if (unhide1 == true) {
		player1.Pos.posY += 0.35;
		if (player1.Pos.posY >= 0.4) {
			unhide1 = false;
		}
	}
	else if (unhide1 == false) {
		player1.Pos.posY = 0.4;
		hideRunning1 = false;
	}
}

void cube_move_timer(int value, POSXYZ playerPos0, POSXYZ playerPos1)
{
	for(int i = 0 ; i < Obstacles.size(); ++ i) {
		Obstacles[i].move(i, meter);
		Obstacles[i].reSetObstacle(i);

		Obs.obstacleXYZ[i] = Obstacles[i].mPos;

		if (Obstacles[i].collide(playerPos0)) {
			player0.isCollide = true;
		}
		else if (Obstacles[i].collide(playerPos1)) {
			player1.isCollide = true;
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

	int retval = send(sock, (char*)&Obs, sizeof(SCObstaclePacket), 0); // obstacle패킷 send
	if (retval == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
			printf("\n%d\n", error);
		}
		//exit(-1);
	}

}

void sendCharacterPacket(SOCKET sock, PLAYER player)
{
	SCCharacterPacket Character;
	Character.type = SCCHARACTERPACKET;
	Character.isCollide = player.isCollide;
	Character.characterXYZ = player.Pos;

	int retval = send(sock, (char*)&Character, sizeof(SCCharacterPacket), 0); // character패킷 send
	if (retval == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
			printf("\n%d\n", error);
		}
		//exit(-1);
	}
}

void sendEnemyPacket(SOCKET sock, PLAYER player)
{
	SCEnemyPacket Enemy;
	Enemy.type = SCENEMYPACKET;
	Enemy.EnemyXYZ = player.Pos;

	int retval = send(sock, (char*)&Enemy, sizeof(SCEnemyPacket), 0); // enemy패킷 send
	if (retval == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
			printf("\n%d\n", error);
		}
		//exit(-1);
	}
}


void reStart()
{
	//game restart
}