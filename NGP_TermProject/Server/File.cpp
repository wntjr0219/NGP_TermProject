//int main(int argc, char** argv) {
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
//	// 플레이어 위치, 장애물 위치 초기화
//	glutInit(&argc, argv); // glut 초기화
//	initGamePlayer();
//
//
//	while (1) {
//		clisock = accept(sersock, (SOCKADDR*)&cliaddr, &addrlen);
//
//		HANDLE hThread[2];
//		hThread[0] = CreateThread(NULL, 0, MoveThread, (LPVOID)clisock, 0, NULL);
//		hThread[1] = CreateThread(NULL, 0, PlayerThread, (LPVOID)clisock, 0, NULL);
//
//		CloseHandle(hThread[0]);
//		CloseHandle(hThread[1]);
//		printf("main");
//	}
//
//	CloseHandle(moveEvent);
//	CloseHandle(playerEvent);
//
//	closesocket(clisock);
//	closesocket(sersock);
//
//	WSACleanup();
//
//	glutMainLoop();
//}
//
//DWORD WINAPI MoveThread(LPVOID arg) {
//	printf("\nmovethread");
//	//SOCKET obsSock = (SOCKET)arg;
//
//	//gluttimerfunc 작동 확인
//	while (1) {
//		glutTimerFunc(50, cube_move_timer, 1);
//
//		SetEvent(moveEvent);
//	}
//
//	return 0;
//}
//DWORD WINAPI PlayerThread(LPVOID arg) {
//
//	DWORD retval;
//	SOCKET sock = (SOCKET)arg;
//
//	while (1) {
//		retval = WaitForSingleObject(moveEvent, INFINITE);
//		printf("\nmeter : %d\n", meter);
//		printf("playerthread");
//		Character.type = SCCHARACTERPACKET;
//		Character.isCollide = false;
//
//		SCObstaclePacket Obs;
//		Obs.type = SCOBSTACLEPACKET;
//
//		for (int i = 0; i < 18; ++i) {
//			//------------임시 테스트용 추가----------------
//			Obstacles[i].mPos.posZ += +1.0;
//			Obstacles[i].reSetObstacle(i);
//			//----------------------------------------------
//			Obs.obstacleXYZ[i] = Obstacles[i].mPos;
//			if (Obstacles[i].collide()) {
//				Character.isCollide = true;
//			}
//		}
//		int retval = send(sock, (char*)&Obs, sizeof(SCObstaclePacket), 0); // obstacle패킷 send
//		if (retval == SOCKET_ERROR) {
//			printf("\n%d\n", WSAGetLastError());
//		}
//
//		RecvProcess(sock);
//
//		//send(sock, (char*)&Character, sizeof(SCCharacterPacket), 0); // character패킷 send
//		//send(sock, (char*)&Enemy, sizeof(SCEnemyPacket), MSG_WAITALL); // enemy패킷 send
//
//
//		if (isDead()) { OverGame(sock); }
//
//		SetEvent(playerEvent);
//	}
//
//	return 0;
//}
//
//void RecvProcess(SOCKET& sock) {
//	BYTE type = 0;
//	int ret = recv(sock, (char*)&type, sizeof(BYTE), MSG_PEEK);
//	if (ret == SOCKET_ERROR) { exit(-1); }
//	type = (packet_type)type;
//	printf("\ntypenum: %d", type);
//	switch (type)
//	{
//	case CSINITIALPACKET:
//		CSInitialPacket Initial;
//		recv(sock, (char*)&Initial, sizeof(CSInitialPacket), MSG_WAITALL);
//		writeRankInfoFile("rankingFile.bin", (RankedInfo*)Initial.nameInitial, Initial.meter);
//		setRankedInfo(sock);
//		break;
//	case CSKEYPACKET:
//		CSKeyPacket Move;
//		recv(sock, (char*)&Move, sizeof(CSKeyPacket), MSG_WAITALL);
//		moveCharacter(Move.keytype);
//		break;
//	case CSRESUMEPACKET:
//		CSResumePacket Resume;
//		recv(sock, (char*)&Resume, sizeof(CSResumePacket), MSG_WAITALL);
//		reStart();
//		break;
//	default:
//		std::cout << "invalid Packet" << std::endl;
//		exit(-1);
//		break;
//	}
//
//}