#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include "protocol.h"
#include "Common.h"
#include "OpenGL.h"
#include "GameUtilities.h"
//------------------------Common.h에 있는것들
//void err_quit(const char* s) { }
#define SERVERPORT 4500
//-------------------------함수선언
void writeRankInfoFile(const char* filename, const RankedInfo* rankInfo);
void readRankInfoFile(const char* filename, RankedInfo*& rankInfo);
void initPlayer();
void initObstacle();
void initGamePlayer();
bool isDead();
void OverGame(SOCKET sock);
void RecvProcess(SOCKET& sock);
void setRankedInfo(SOCKET sock);
void moveCharacter(int keytype);
void reStart();
//------------------------전역변수(공유자원포함)
int numOfClient = 0;
HANDLE moveEvent;
HANDLE playerEvent;
const char* filename;
SCRankingPacket rankingPacket;
POSXYZ playerPos;
RankedInfo rankInfo;
float player_hp;


//------------------------Thread 정의

// 여기서는 뭘 처리할 것인가
// Obstacle Thread를 싱글 멀티로 어떻게 나눌 것인가? -> 도성 제시 : 스레드 Supend후 자식 스레드 생성
DWORD WINAPI MoveThread(LPVOID arg) {

	SOCKET obsSock = (SOCKET)arg;

	while (1) {
		// 장애물 위치 변경, 충돌검사
		glutTimerFunc(50, cube_move_timer, 1);
	}

	SetEvent(moveEvent);
	return 0;
}

//여기서 뭘 처리할 것인가
DWORD WINAPI PlayerThread(LPVOID arg) {

	DWORD retval;
	SOCKET sock = (SOCKET)arg;

	while (1) {
		retval = WaitForSingleObject(moveEvent, INFINITE);
		RecvProcess(sock);

		//send() 장애물에 대한 변경 : 한꺼번에 위치변경값, 충돌검사값;
		//send하는 부분 함수화로 간략하게 수정할 예정
		for (int i = 0; i < 18; ++i) {
			if (!Obstacles[i].collide()) {
				SCCollidePacket Collide;
				sphere_hp_color += 0.2;
				Collide.type = SCCOLLIDE;
				Collide.playerHpColor = sphere_hp_color;
				send(sock, (char*)&Collide, sizeof(SCCollidePacket), MSG_WAITALL);
			}
			SCCharacterPacket Character;
			//charcter 구조체 값 초기화해야함
			//Character.characterXYZ ....
			send(sock, (char*)&Character, sizeof(SCCharacterPacket), MSG_WAITALL);
			// 구조체 값 초기화해야함
			SCWinnerPacket Winner;
			send(sock, (char*)&Winner, sizeof(SCWinnerPacket), MSG_WAITALL);
			//
		}
		

		if (isDead()) { OverGame(sock); }

		SetEvent(playerEvent);
	}

	return 0;
}



//------------------------ Main 함수

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

	// 이벤트 생성
	moveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	playerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 플레이어 위치, 장애물 위치 초기화
	initGamePlayer();


	while (1) {

		clisock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);

		HANDLE hThread[2];
		hThread[0] = CreateThread(NULL, 0, MoveThread, 0, 0, NULL);
		hThread[1] = CreateThread(NULL, 0, PlayerThread, 0, 0, NULL);

	}

	CloseHandle(moveEvent);
	CloseHandle(playerEvent);

	closesocket(clisock);
	closesocket(sersock);

	WSACleanup();

}


void moveCharacter(int keytype) 
{
	switch (keytype)
	{
	case KEYUP:
		glutTimerFunc(30, sphere_jump_timer, 1);
		break;
	case KEYDOWN:
		glutTimerFunc(30, sphere_hide_timer, 1);
		break;
	case KEYLEFT:
		if (!(sphere_pos_x <= -3.0)) {
			sphere_pos_x -= 1.0;
		}
		break;
	case KEYRIGHT:
		if (!(sphere_pos_x >= 3.0)) {
			sphere_pos_x += 1.0;
		}
		break;
	default: // 값이 없을때 안누른거냐 오류인거냐 모름
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
	}
	
	send(sock, (char*)&rankingPacket, sizeof(rankingPacket), MSG_WAITALL);
	delete[] rankInfo;
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
		//meter값도 받아야함.(수정필요)
		writeRankInfoFile("rankingFile.bin", (RankedInfo*)Initial.nameInitial);
		setRankedInfo(sock);
		break;
	case CSKEYPACKET:
		CSKeyPacket Move;
		recv(sock, (char*)&Move, sizeof(CSKeyPacket), MSG_WAITALL);
		moveCharacter(Move.keytype);
		break;
	case CSRESUMEPACKET:
		CSResumePacket Resume;
		recv(sock, (char*)&Resume, sizeof(CSResumePacket), MSG_WAITALL);
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

	int numCopyElements = std::min(RANKERS, static_cast<int>(allRankings.size()));

	for (int i = 0; i < numCopyElements; ++i) {
		rankInfo[i] = allRankings[i];
	}

	file.close();
}

void initPlayer()
{
	// 플레이어 위치 상수값 초기화 (맵의 정중앙 위치)
	playerPos.posX = 0.0;
	playerPos.posY = 0.4;
	playerPos.posZ = 3.0;
}

void initObstacle()
{
	// 장애물 위치 랜덤값범위로 초기화
	Obstacles.reserve(18);
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
		Obstacles.emplace_back(4.0, 1.0, 2);
	}
}

void initGamePlayer()
{
	//게임 초기화에 필요한 함수 추가
	initPlayer();
	initObstacle();
}

bool isDead() {
	return player_hp > 2.0;
}

void sphere_jump_timer(int value)
{
	if (jumped == true && sphere_pos_y < 2.6) {
		sphere_pos_y += 0.2;
		if (sphere_pos_y >= 2.6) {
			jumped = false;
			falling = true;
		}
		glutTimerFunc(30, sphere_jump_timer, 1);
	}
	else if (falling == true) {
		sphere_pos_y -= 0.2;
		if (sphere_pos_y <= 0.4) {
			falling = false;
		}
		glutTimerFunc(30, sphere_jump_timer, 1);
	}
	else if (falling == false) {
		sphere_pos_y = 0.4;
		running = false;
		mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL);    //음원 재생 위치를 처음으로 초기화
	}
}

void sphere_hide_timer(int value)
{
	if (hide == true && sphere_pos_y >= -2.0) {
		sphere_pos_y -= 0.35;
		if (sphere_pos_y <= -2.0) {
			hide = false;
			unhide = true;
		}
		glutTimerFunc(40, sphere_hide_timer, 1);
	}
	else if (unhide == true) {
		sphere_pos_y += 0.35;
		if (sphere_pos_y >= 0.4) {
			unhide = false;
		}
		glutTimerFunc(40, sphere_hide_timer, 1);
	}
	else if (unhide == false) {
		sphere_pos_y = 0.4;
		running = false;
		mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL);
	}
}

void cube_move_timer(int value)
{
	for(int i = 0 ; i < Obstacles.size(); ++ i) {
		Obstacles[i].move(i, meter);
		Obstacles[i].reSetObstacle(i);
	}
 
	meter++;
	glutTimerFunc(50, cube_move_timer, 1);

}

void OverGame(SOCKET sock)
{
	SCPausePacket pause;
	pause.seconds = 5;
	pause.type = SCPAUSEPACKET;
	send(sock, (char*)&pause, sizeof(SCPausePacket), MSG_WAITALL);
}



void reStart()
{
	//game restart
}