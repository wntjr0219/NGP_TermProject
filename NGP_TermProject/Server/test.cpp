//#include <iostream>
//#include <thread>
//#include "protocol.h"
//#include "Common.h" // 추가
//#include "OpenGL.h" // 추가
//#include "GameUtilities.h" // 추가
//
////------------------------Common.h에 있는것들
////void err_quit(const char* s) { }
//#define SERVERPORT 4500
////-------------------------함수선언
//bool isDead();
//void OverGame();
//void RecvProcess(SOCKET& sock);
//void setRankedInfo(int meter, char* Inintial);
//void moveCharacter(int keytype);
//void reStart();
////------------------------전역변수(공유자원포함)
//int numOfClient = 0;
//
//// 11.22 - 이벤트 추가
//HANDLE moveEvent;
//HANDLE playerEvent;
//
//
//// 숫자로 적혀있는것 변수로 변경 - 11.21
//#define rx_limit 3.0  // 오른쪽 경계
//#define lx_limit -3.0 // 왼쪽 경계
//#define y_limit 2.6
//
////------------------------Thread 정의
//
//// 11.22 플레이어 스레드로 이름 변경
//// 캐릭터, 장애물 좌표값, 체력, 승리여부 보내기
//DWORD WINAPI PlayerThread(LPVOID arg) {
//
//	SOCKET sock = (SOCKET)arg;
//
//	while (1) {
//
//		RecvProcess(sock);
//
//		bool deadflag = isDead();
//		if (deadflag == true) { OverGame(); }
//
//		// 스레드 완료
//		SetEvent(playerEvent);
//	}
//
//	return 0;
//}
//
//// 11.21 무브 스레드
//// 장애물 위치 변경, 충돌검사
//DWORD WINAPI MoveThread(LPVOID arg) {
//
//	SOCKET obsSock = (SOCKET)arg;
//	// 장애물 위치 변경
//	cube_move_timer(1);
//	// 충돌검사
//	
//	// 장애물 위치든 뭐든 일단 변경
//	while (1) {
//		//send();
//
//	}
//	// 스레드 완료
//	SetEvent(moveEvent);
//	return 0;
//}
//
////------------------------ Main 함수
//
//int main(void) {
//
//
//	WSADATA wsa;
//	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
//
//	SOCKET sersock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (sersock == INVALID_SOCKET) err_quit("socket()");
//
//	SOCKADDR_IN seraddr;
//	memset(&seraddr, 0, sizeof(seraddr));
//	seraddr.sin_family = AF_INET;
//	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
//	seraddr.sin_port = htons(SERVERPORT);
//
//	if (SOCKET_ERROR == bind(sersock, (SOCKADDR*)&seraddr, sizeof(seraddr))) err_quit("bind()");
//
//	if (listen(sersock, SOMAXCONN) == SOCKET_ERROR) err_quit("listen()");
//
//	SOCKET clisock;
//	SOCKADDR_IN cliaddr;
//	int addrlen = sizeof(cliaddr);
//
//	// 이벤트 생성
//	moveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	playerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	
//
//	while (1) {
//
//		clisock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);
//	
//		HANDLE MovThread = CreateThread(NULL, 0, MoveThread, 0, 0, NULL);
//
//
//
//
//	}
//
//	closesocket(clisock);
//	closesocket(sersock);
//
//	WSACleanup();
//
//}
//
//
//// 11.21 함수 구현
//void moveCharacter(int keytype) {
//	switch (keytype)
//	{
//	case KEYUP:
//		sphere_jump_timer(1);
//		break;
//	case KEYDOWN:
//		sphere_hide_timer(1);
//		break;
//	case KEYLEFT:		
//		if (!(sphere_pos_x <= lx_limit)) {
//			sphere_pos_x -= 1.0;
//		}
//		break;
//	case KEYRIGHT:
//		if (!(sphere_pos_x >= rx_limit)) {
//			sphere_pos_x += 1.0;
//		}
//		break;
//	default: // 값이 없을때 안누른거냐 오류인거냐 모름
//		break;
//
//	}
//	//Collsion Detection
//}
//
//
//
//void setRankedInfo(int meter, char* Initial) {
//	//place aggregation
//}
//void reStart() {
//	//game restart
//}
//
//
//void RecvProcess(SOCKET& sock) {
//	BYTE type;
//	int ret = recv(sock, (char*)type, sizeof(BYTE), MSG_PEEK);
//	if (ret == SOCKET_ERROR) { exit(-1); }
//	switch (type)
//	{
//	case CSINITIALPACKET:
//		CSInitialPacket Initial;
//		recv(sock, (char*)&Initial, sizeof(CSInitialPacket), MSG_WAITALL);
//		setRankedInfo(Initial.meter, Initial.nameInitial);
//		break;
//	case CSKEYPACKET:
//		CSKeyPacket Move;
//		recv(sock, (char*)&Move, sizeof(CSKeyPacket), MSG_WAITALL);
//		moveCharacter(Move.keytype);
//		break;
//	case CSRESTARTPACKET:
//		CSReStartPacket Restart;
//		recv(sock, (char*)&Restart, sizeof(CSReStartPacket), MSG_WAITALL);
//		reStart();
//		break;
//	default:
//		std::cout << "invalid Packet" << std::endl;
//		exit(-1);
//		break;
//	}
//
//}
//
//bool isDead() {
//
//}
//
//void OverGame() {
//
//}
//
//
//// timer
//void sphere_jump_timer(int value)
//{
//	if (jumped == true && sphere_pos_y < 2.6) {
//		sphere_pos_y += 0.2;
//		if (sphere_pos_y >= 2.6) {
//			jumped = false;
//			falling = true;
//		}
//		glutTimerFunc(30, sphere_jump_timer, 1);
//	}
//	else if (falling == true) {
//		sphere_pos_y -= 0.2;
//		if (sphere_pos_y <= 0.4) {
//			falling = false;
//		}
//		glutTimerFunc(30, sphere_jump_timer, 1);
//	}
//	else if (falling == false) {
//		sphere_pos_y = 0.4;
//		running = false;
//		mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL);    //음원 재생 위치를 처음으로 초기화
//	}
//}
//
//void sphere_hide_timer(int value)
//{
//	if (hide == true && sphere_pos_y >= -2.0) {
//		sphere_pos_y -= 0.35;
//		if (sphere_pos_y <= -2.0) {
//			hide = false;
//			unhide = true;
//		}
//		glutTimerFunc(40, sphere_hide_timer, 1);
//	}
//	else if (unhide == true) {
//		sphere_pos_y += 0.35;
//		if (sphere_pos_y >= 0.4) {
//			unhide = false;
//		}
//		glutTimerFunc(40, sphere_hide_timer, 1);
//	}
//	else if (unhide == false) {
//		sphere_pos_y = 0.4;
//		running = false;
//		mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL);
//	}
//}
//
//void cube_move_timer(int value)
//{
//	// 랜덤 생성
//	for (int i = 0; i < 5; i++) {
//		cube_pos_z[i] += 0.5;
//		if (meter > 200) { normal_cube_pos_z[i] += 1.0; }
//		if (meter > 600) { hard_cube_pos_z[i] += 1.0; }
//		if (meter > 1000) { hard_cube2_pos_z[i] += 1.15; }
//		if (cube_pos_z[i] >= 6.0) {
//			cube_pos_z[i] = -50.0;
//			cube_pos_x[i] = (float)(rand() % 7);
//			cube_pos_y[i] = (float)(rand() % 2);
//		}
//		if (meter > 200 && normal_cube_pos_z[i] >= 6.0) {
//			normal_cube_pos_z[i] = -50.0;
//			normal_cube_pos_x[i] = (float)(rand() % 7);
//			normal_cube_pos_y[i] = (float)(rand() % 2);
//		}
//		if (meter > 600 && hard_cube_pos_z[i] >= 6.0) {
//			hard_cube_pos_z[i] = -50.0;
//			hard_cube_pos_x[i] = (float)(rand() % 7);
//			hard_cube_pos_y[i] = (float)(rand() % 2);
//		}
//		if (meter > 100 && hard_cube2_pos_z[i] >= 6.0) {
//			hard_cube2_pos_z[i] = -(float)(rand() % 200 + 40);
//			hard_cube2_pos_y[i] = (float)(rand() % 3);
//		}
//		if (sphere_hp_color > 2.0) {
//			printf("\nGame Over\n");
//			printf("최종 기록 : %d 미터\n", meter);
//			death = true;
//			glutTimerFunc(100, game_over_timer, 1);
//			drawScene();
//			return;
//		}
//	}
//	meter++;
//	glutTimerFunc(50, cube_move_timer, 1);
//	
//}
//
//void InitPlayer() {
//
//	// 플레이어 초기 위치값 보내기(위치는 GameUtilities.cpp에)
//	POSXYZ player;
//	player.posX = sphere_pos_x;
//	player.posY = sphere_pos_y;
//	player.posZ = sphere_pos_z;
//	SCCharacterPacket playerStart;
//	playerStart.type = SCCHARACTERPACKET;
//	playerStart.isCollide = false;
//	playerStart.characterXYZ = player;
//
//	//retval = send();
//}	
//
//
//void InitObstacle() {
//
//
//} //장애물 위치 생성
//
//// 장애물 초기 위치값 설정

#include <iostream>
#include "protocol.h"

void main() {
	POSXYZ player;
		player.posX = 1;
		player.posY = 2;
		player.posZ = 3;
		SCCharacterPacket playerStart;
		playerStart.type = SCCHARACTERPACKET;
		playerStart.isCollide = false;
		playerStart.characterXYZ = player;

<<<<<<< Updated upstream
		std::cout << playerStart.characterXYZ.posX <<  playerStart.characterXYZ.posY << playerStart.characterXYZ.posZ << std::endl;
=======
//------------------------Thread 정의

// 여기서는 뭘 처리할 것인가
// Obstacle Thread를 싱글 멀티로 어떻게 나눌 것인가? -> 도성 제시 : 스레드 Supend후 자식 스레드 생성

DWORD WINAPI MoveThread(LPVOID arg) {
	
	//SOCKET obsSock = (SOCKET)arg;

	// 11.30 수정
	while (1) {
		printf("\nmovethread");
		cube_move_timer(1);  // 직접 호출하거나, 적절한 시간 간격을 계산하여 호출
		SetEvent(moveEvent);
		Sleep(10);  // 적절한 시간 동안 스레드를 대기시킴
	}
	return 0;
}

//여기서 뭘 처리할 것인가
DWORD WINAPI PlayerThread(LPVOID arg) {

	DWORD retval;
	SOCKET sock = (SOCKET)arg;
	
	while (1) {
		retval = WaitForSingleObject(moveEvent, INFINITE);
		printf("\nmeter : %d\n", meter);
		printf("playerthread");
		Character.type = SCCHARACTERPACKET;
		Character.isCollide = false;

		SCObstaclePacket Obs;
		Obs.type = SCOBSTACLEPACKET;
		
		for (int i = 0; i < 18; ++i) {
			Obs.obstacleXYZ[i] = Obstacles[i].mPos;
			if (Obstacles[i].collide()) {
				Character.isCollide = true;
			}
		}
		int retval = send(sock, (char*)&Obs, sizeof(SCObstaclePacket), 0); // obstacle패킷 send
		if (retval == SOCKET_ERROR) {
			printf("\n%d\n", WSAGetLastError());
		}

		//RecvProcess(sock);


		send(sock, (char*)&Character, sizeof(SCCharacterPacket), 0); // character패킷 send
		//send(sock, (char*)&Enemy, sizeof(SCEnemyPacket), MSG_WAITALL); // enemy패킷 send

		
		if (isDead()) { OverGame(sock); }

		SetEvent(playerEvent);
	}

	return 0;
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
	initGamePlayer();


	while (1) {
		clisock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);

		HANDLE hThread[2];
		hThread[0] = CreateThread(NULL, 0, MoveThread, (LPVOID)clisock, 0, NULL);
		hThread[1] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);

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
		if (!(Character.characterXYZ.posX <= -3.0)) {
			Character.characterXYZ.posX -= 1.0;
		}
		break;
	case KEYRIGHT:
		if (!(Character.characterXYZ.posX >= 3.0)) {
			Character.characterXYZ.posX += 1.0;
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
	int* meters = new int[RANKERS];
	readRankInfoFile(filename, rankInfo, meters);

	rankingPacket.type = SCRANKINGPACKET;
	for (int i = 0; i < RANKERS; ++i) {
		rankingPacket.rankings[i] = rankInfo[i];
	}
	
	send(sock, (char*)&rankingPacket, sizeof(rankingPacket), MSG_WAITALL);
	delete[] rankInfo;
}


void RecvProcess(SOCKET& sock) {
	BYTE type = 0;
	int ret = recv(sock, (char*)&type, sizeof(BYTE), MSG_PEEK);
	if (ret == SOCKET_ERROR) { exit(-1); }
	type = (packet_type)type;
	printf("\ntypenum: %d", type);
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
		recv(sock, (char*)&Move, sizeof(CSKeyPacket), MSG_WAITALL);
		moveCharacter(Move.keytype);
		break;
	case CSRESUMEPACKET:
		CSResumePacket Resume;
		recv(sock, (char*)&Resume, sizeof(CSResumePacket), MSG_WAITALL);
		reStart();
		break;
	default:
		//std::cout << "invalid Packet" << std::endl;
		//exit(-1);
		break;
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
	Character.characterXYZ.posX = 0.0;
	Character.characterXYZ.posY = 0.4;
	Character.characterXYZ.posZ = 3.0;
	Character.isCollide = false;
	Character.type = SCCHARACTERPACKET;
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
		glutTimerFunc(30, sphere_jump_timer, 1);
	}
	else if (falling == true) {
		Character.characterXYZ.posY -= 0.2;
		if (Character.characterXYZ.posY <= 0.4) {
			falling = false;
		}
		glutTimerFunc(30, sphere_jump_timer, 1);
	}
	else if (falling == false) {
		Character.characterXYZ.posY = 0.4;
		running = false;
		mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL);    //음원 재생 위치를 처음으로 초기화
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
		glutTimerFunc(40, sphere_hide_timer, 1);
	}
	else if (unhide == true) {
		Character.characterXYZ.posY += 0.35;
		if (Character.characterXYZ.posY >= 0.4) {
			unhide = false;
		}
		glutTimerFunc(40, sphere_hide_timer, 1);
	}
	else if (unhide == false) {
		Character.characterXYZ.posY = 0.4;
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
	printf("%d\n", meter);
	glutTimerFunc(50, cube_move_timer, 1);
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



void reStart()
{
	//game restart
>>>>>>> Stashed changes
}