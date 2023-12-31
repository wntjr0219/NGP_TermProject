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
	int collideCnt = 0;
	bool exist;
};

struct saveData {
	std::vector<Obstacle> ObstaclesSave;
	int saveMeter;
	PLAYER playerSave;
};
saveData save;

struct savingIP {
	SOCKET sock;
	char ip[INET_ADDRSTRLEN];
	bool winner = false;

	void init() {
		sock = NULL;
		ip[0] = '\0';
		winner = false;
	}
};
savingIP playersINFO[2];
//-------------------------함수선언
void writeRankInfoFile(const char* filename, RankedInfo* rankInfo);
void readRankInfoFile(const char* filename, RankedInfo*& rankInfo);
void initPlayer();
void initObstacle();
void initGamePlayer();
void saveGameSnapshot(SOCKET sock);
bool isDead(PLAYER player);
void OverGame(SOCKET sock, PLAYER& player, PLAYER& enemy);
void RecvProcess(SOCKET sock, PLAYER player);
void setRankedInfo(SOCKET sock);
void moveCharacter(int keytype, PLAYER player);
void sendObstaclePacket(SOCKET sock);
void sendCharacterPacket(SOCKET sock, PLAYER player);
void sendEnemyPacket(SOCKET sock, PLAYER player);
void reStart(SOCKET sock);
//------------------------전역변수(공유자원포함)
int numOfClient = 0;
bool restart = false;
bool threadExit = false;
bool clearThread[2] = { false, false };
HANDLE moveEvent;
HANDLE playerEvent;
const char* filename;
SCRankingPacket rankingPacket;
RankedInfo rankInfo;
SCObstaclePacket Obs;
PLAYER player0, player1;



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
	PLAYER enemy;

	u_long mode = 1;
	if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("error code : %d\n", WSAGetLastError());
	}
	while (1) {
		if (playersINFO[0].sock == sock) {	// 첫 번째로 들어온 유저인 경우의 PlayerThread
			player = player0;
			enemy = player1;
			player0.exist = true;
		}
		else {	// 두 번째 들어온 유저인 경우의 PlayerThread
			player = player1;
			enemy = player0;
			player1.exist = true;
		}

		retval = WaitForSingleObject(moveEvent, INFINITE);

		sendObstaclePacket(sock);

		RecvProcess(sock, player);

		sendCharacterPacket(sock, player);
		sendEnemyPacket(sock, enemy);

		
		if (isDead(player) || isDead(enemy)) {
			OverGame(sock, player, enemy); 
		}

		if (threadExit) {
			threadExit = false;
			numOfClient--;
			if (player.id == 0) {
				if (save.playerSave.id == player.id) {
					reStart(sock);
				}
				player0.exist = false;
				clearThread[0] = true;
				printf("%d\n", clearThread[0]);
			}
			else if (player.id == 1) {
				if (save.playerSave.id == player.id) {
					reStart(sock);
				}
				player1.exist = false;
				clearThread[1] = true;
				printf("%d\n", clearThread[1]);
			}
			break;
		}

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
				printf("create 1\n");
				saveIP(clisock);
				hThread[1] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);
				CloseHandle(hThread[1]);
				numOfClient++;
				if (playersINFO[1].sock != NULL) {
					saveGameSnapshot(clisock);	
				}
				initGamePlayer();
			}
			else if (playersINFO[1].sock == NULL) {
				playersINFO[1].sock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);
				playersINFO[1].ip[0] = '\0';
				clisock = playersINFO[1].sock;
				printf("create 2\n");
				saveIP(clisock);
				hThread[2] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);
				CloseHandle(hThread[2]);
				numOfClient++;
				//두번째 플레이어가 들어왔을때, 모든 게임 정보를 저장해야함.
				if (playersINFO[0].sock != NULL) {
					saveGameSnapshot(clisock);
				}
				initGamePlayer();
			}
		}

		if (clearThread[0]) {
			closesocket(playersINFO[0].sock);
			playersINFO[0].init();
			printf("playerThread1 clear\n");
			clearThread[0] = false;
		}
		if (clearThread[1]) {
			closesocket(playersINFO[1].sock);
			playersINFO[1].init();
			printf("playerThread2 clear\n");
			clearThread[1] = false;
		}	
	}

	CloseHandle(moveEvent);
	CloseHandle(playerEvent);

	//closesocket(clisock);
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
			}
		}
		else {
			if (jumpRunning1 == false) {
				jumped1 = true;
				jumpRunning1 = true;
			}
		}
		break;
	case KEYDOWN:
		if (player.id == 0) {
			if (hideRunning0 == false) {
				hide0 = true;
				hideRunning0 = true;
			}
		}
		else {
			if (hideRunning1 == false) {
				hide1 = true;
				hideRunning1 = true;
			}
		}
		break;
	case KEYLEFT:
		if (player.id == 0) {
			if (!(player0.Pos.posX <= -3.0)) {
				player0.Pos.posX -= 1.0;
			}
			break;
		}
		else {
			if (!(player1.Pos.posX <= -3.0)) {
				player1.Pos.posX -= 1.0;
			}
		}
		break;
	case KEYRIGHT:
		if (player.id == 0) {
			if (!(player0.Pos.posX >= 3.0)) {
				player0.Pos.posX += 1.0;
			}
		}
		else {
			if (!(player1.Pos.posX >= 3.0)) {
				player1.Pos.posX += 1.0;
			}
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
	readRankInfoFile(filename, rankInfo);

	rankingPacket.type = SCRANKINGPACKET;
	for (int i = 0; i < RANKERS; ++i) {
		rankingPacket.rankings[i] = rankInfo[i];
		printf("rankinfo : %s meter : %d\n", rankingPacket.rankings[i].name, rankingPacket.rankings[i].meter);
	}
	
	send(sock, (char*)&rankingPacket, sizeof(rankingPacket), 0);
	printf("send rank\n");
	delete[] rankInfo;
}


void RecvProcess(SOCKET sock, PLAYER player) {

	BYTE type = 0;
	int ret = recv(sock, (char*)&type, sizeof(BYTE), MSG_PEEK);
	if (WSAGetLastError() == WSAECONNRESET) {
		threadExit = true;
	}

	if (ret > 0) {
		switch (type)
		{
		case CSINITIALPACKET:
			CSInitialPacket Initial;
			recv(sock, (char*)&Initial, sizeof(CSInitialPacket), 0);
			writeRankInfoFile("rankingFile.bin", &Initial.rank);
			setRankedInfo(sock);
			break;
		case CSKEYPACKET:
			CSKeyPacket Move;
			recv(sock, (char*)&Move, sizeof(CSKeyPacket), 0);
			moveCharacter(Move.keytype, player);
			break;
		case CSRESUMEPACKET:
			CSResumePacket Resume;
			recv(sock, (char*)&Resume, sizeof(CSResumePacket), 0);
			printf("restart\n");
			if(Resume.start) reStart(sock);
			break;
		case CSEXITPACKET:
			CSExitPacket Exit;
			recv(sock, (char*)&Exit, sizeof(CSExitPacket), 0);
			threadExit = Exit.exit;
		default:
			std::cout << "invalid Packet"<< std::endl;
			break;
		}
	}
}

void writeRankInfoFile(const char* filename, RankedInfo* rankInfo)
{
	rankInfo->meter = meter;
	//printf("이니셜 : %s , meter: %d\n", rankInfo->name, rankInfo->meter);
	std::ofstream file(filename, std::ios::binary | std::ios::app);
	if (!file.is_open()) {
		std::cerr << "file write open error" << std::endl;
		return;
	}
	file.write(reinterpret_cast<const char*>(rankInfo), sizeof(RankedInfo));
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

	int numCopyElements = std::min(RANKERS, static_cast<int>(allRankings.size()));
	for (int i = 0; i < numCopyElements; ++i) {
		rankInfo[i] = allRankings[i];
		//printf("이니셜 : %s , meter: %d\n", rankInfo[i].name, rankInfo[i].meter);
	}
	
	
	file.close();
}

void initPlayer()
{
	player0.Pos.posX = 0.0;
	player0.Pos.posY = 0.4;
	player0.Pos.posZ = 3.0;
	player0.isCollide = false;
	player0.id = 0;
	player0.collideCnt = 0;

	player1.Pos.posX = 0.0;
	player1.Pos.posY = 0.4;
	player1.Pos.posZ = 3.0;
	player1.isCollide = false;
	player1.id = 1;
	player1.collideCnt = 0;
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
	meter = 0;
	Obstacles.clear();
	srand(time(NULL));
	//게임 초기화에 필요한 함수 추가
	initPlayer();
	initObstacle();
	printf("initGamePlayer\n");
}

void saveGameSnapshot(SOCKET sock)
{
	save.ObstaclesSave.reserve(OBSTACLES);
	save.ObstaclesSave = Obstacles;
	save.saveMeter = meter;
	if (playersINFO[0].sock == sock) {
		save.playerSave = player0;
	}
	else if (playersINFO[1].sock == sock) {
		save.playerSave = player1;
	}
	
}

bool isDead(PLAYER player) {
	//printf("collide count : %d\n", player.collideCnt);
	return player.collideCnt >= 10; 
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
			if (player0.exist) {
				player0.isCollide = true;
				++player0.collideCnt;
				printf("player0 coll\n");
				printf("%f\n", playerPos0.posX);
			}
		}
		else if (Obstacles[i].collide(playerPos1)) {
			if (player1.exist) {
				player1.isCollide = true;
				++player1.collideCnt;
				printf("player1 coll\n");
				printf("%f\n", playerPos1.posX);
			}
		}
	}
	meter++;
}

void OverGame(SOCKET sock, PLAYER& player, PLAYER& enemy)
{
	SCWinnerPacket Winner;
	Winner.type = SCWINNERPACKET;
	Winner.winner = !isDead(player);
	std::cout << "winner" << std::endl;
	send(sock, (char*)&Winner, sizeof(SCWinnerPacket), 0);

	if (sock == playersINFO[0].sock) {
		playersINFO[0].winner = true;
		Winner.winner = !isDead(enemy);
		send(playersINFO[1].sock, (char*)&Winner, sizeof(SCWinnerPacket), 0);
		player0.exist = false;
		player1.collideCnt = 0;
		player0.collideCnt = 0;
	}
	else {
		playersINFO[1].winner = true;
		Winner.winner = !isDead(enemy);
		send(playersINFO[0].sock, (char*)&Winner, sizeof(SCWinnerPacket), 0);
		player1.exist = false;
		player1.collideCnt = 0;
		player0.collideCnt = 0;
	}
}

void sendObstaclePacket(SOCKET sock)
{
	Obs.type = SCOBSTACLEPACKET;
	if (numOfClient == 2) {
		Obs.isMulti = true;
	}
	else Obs.isMulti = false;

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

void reStart(SOCKET sock)
{
	//initGamePlayer();
	Obstacles = save.ObstaclesSave;
	meter = save.saveMeter;
	if (playersINFO[0].sock == sock) {
		player0 = save.playerSave;
	}
	else if (playersINFO[1].sock == sock) {
		player1 = save.playerSave;
	}
	
}