#include "OpenGL.h"
#include "Init.h"
#include "GameUtilities.h"
#include <math.h>

#include <WS2tcpip.h>
//#pragma comment(lib, "WS32.lib");
#pragma comment(lib, "ws2_32.lib")

#define window_w 500
#define window_h 400

const char* SERVERIP = "127.0.0.1";
int SERVERPORT = 4500;

SOCKET wSock;

void ConnectServer() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) exit(1);

	wSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (wSock == INVALID_SOCKET) {
		printf("socket()"); exit(1);
	}

	SOCKADDR_IN seraddr;

	memset(&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &seraddr.sin_addr);

	seraddr.sin_port = htons(SERVERPORT);

	if (connect(wSock, (sockaddr*)&seraddr, sizeof(seraddr)) == SOCKET_ERROR) {
		printf("connect()"); exit(1);
	}

}

void DisConnectServer() {

	closesocket(wSock);

	WSACleanup();
}

void moveCharacter(char* packet) {
	if (packet[0] == SCCHARACTERPACKET) {
		SCCharacterPacket player;
		memcpy(&player, packet, sizeof(SCCharacterPacket));
		PlayerPos = player.characterXYZ;
		sphere_pos_x = PlayerPos.posX;
		sphere_pos_y = PlayerPos.posY;
		sphere_pos_z = PlayerPos.posZ;
		if (player.isCollide) {
			sphere_hp_color += 0.2;
			printf("collide!!\n");
		}
	}
	else if (packet[0] == SCENEMYPACKET) {
		SCEnemyPacket enemy;
		memcpy(&enemy, packet, sizeof(SCEnemyPacket));
		EnemysPos = enemy.EnemyXYZ;
		enemy_sphere_pos_x = EnemysPos.posX;
		enemy_sphere_pos_y = EnemysPos.posY;
		enemy_sphere_pos_z = EnemysPos.posZ;
	}
}

void moveObstacles(char* packet) {
	SCObstaclePacket Obs;
	memcpy(&Obs, packet, sizeof(SCObstaclePacket));

	for (int i = 0; i < OBSTACLES; ++i) {
		if (i < 5) {
			cubePos[i] = Obs.obstacleXYZ[i];
		}
		else if (i < 10 and i >= 5) {
			normalCubePos[i - 5] = Obs.obstacleXYZ[i];
		}
		else if (i < 15 and i >= 10) {
			hardCubePos[i - 10] = Obs.obstacleXYZ[i];
		}
		else {
			hardCube2Pos[i - 15] = Obs.obstacleXYZ[i];
		}
	}


	//Obstacles.posXYZ = Obs.obstacleXYZ;
}

void showRankings(char* packet) {
	SCRankingPacket rankings;
	memcpy(&rankings, packet, sizeof(SCRankingPacket));

	for (int i = 0; i < RANKERS; ++i) {
		printf("%d  - %s - %dm\n", i, rankings.rankings[i].name, rankings.rankings[i].meter);
	}
}
void ReceiveProcess() {
	BYTE type = 0;
	int ret = recv(wSock, (char*)&type, sizeof(BYTE), MSG_PEEK);
	if (ret == SOCKET_ERROR) { exit(-1); }
	if (ret > 0) {
		switch (type)
		{
		case SCCHARACTERPACKET:
			SCCharacterPacket charactermove;
			recv(wSock, (char*)&charactermove, sizeof(SCCharacterPacket), MSG_WAITALL);
			moveCharacter((char*)&charactermove);
			break;
		case SCENEMYPACKET:
			SCEnemyPacket enemymove;
			recv(wSock, (char*)&enemymove, sizeof(SCEnemyPacket), MSG_WAITALL);
			moveCharacter((char*)&enemymove);
			break;
		case SCWINNERPACKET:
			SCWinnerPacket winner;
			recv(wSock, (char*)&winner, sizeof(SCWinnerPacket), MSG_WAITALL);
			printf("Winner\n");
			if (winner.winner) isWin = true;
			else death = true;
			break;
		case SCOBSTACLEPACKET:
			SCObstaclePacket obastaclesmove;
			recv(wSock, (char*)&obastaclesmove, sizeof(SCObstaclePacket), MSG_WAITALL);
			isMulti = obastaclesmove.isMulti;
			moveObstacles((char*)&obastaclesmove);
			break;
		case SCRANKINGPACKET:
			SCRankingPacket rankings;
			recv(wSock, (char*)&rankings, sizeof(SCRankingPacket), MSG_WAITALL);
			printf("recv rankings\n");
			recvRankings = true;
			showRankings((char*)&rankings);
			break;
		default:
			std::cout << "invalid Packet" << std::endl;
			break;
		}
	}
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	ConnectServer();

	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(window_w, window_h); // 윈도우의 크기 지정
	glutCreateWindow("CG_project"); // 윈도우 생성 (윈도우 이름 )

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	CreateSphere(eEBO, eVBO);
	CreateSphere(enemy_eEBO, enemy_eVBO);

	srand(time(NULL));

	// init 부분 함수화 
	InitProgram(s_program);

	playingBgm();

	glutDisplayFunc(drawScene); // 출력 콜백함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력 콜백함수 지정
	glutSpecialFunc(Special);
	glutTimerFunc(65, render, 1);
	glutMainLoop(); // 이벤트 처리 시작

	DisConnectServer();

}
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	//그리기 전에 먼저 값을 recv하여 렌더링에 반영해주어야함.
	//ReceiveProcess();

	//--- 변경된 배경색 설정
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(s_program);
	glEnable(GL_DEPTH_TEST);

	//뷰포트 사용x
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 pt = glm::mat4(1.0f);
	glm::mat4 pR = glm::mat4(1.0f);

	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
	pt = glm::translate(pt, glm::vec3(x_m_camera, 0.0, z_m_camera));
	pR = glm::rotate(pR, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));

	if (view_third_person == true) {
		projection = projection * pt * pR;
	}
	else if (view_first_person == true) {
		pt = glm::mat4(1.0f);
		pR = glm::mat4(1.0f);
		pR = glm::rotate(pR, glm::radians(10.0f), glm::vec3(1.0, 0.0, 0.0));
		pt = glm::translate(pt, glm::vec3(-sphere_pos_x, -sphere_pos_y, -sphere_pos_z + 2));
		projection = projection * pt * pR;
	}

	unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

	int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	glUniform3f(lightColorLocation, R, G, B);
	int lightPosColorLocation = glGetUniformLocation(s_program, "lightPos");
	glUniform3f(lightPosColorLocation, 0.0, 0.0, 5.0);
	int viewPosColorLocation = glGetUniformLocation(s_program, "viewPos");
	glUniform3f(lightPosColorLocation, cameraPos_x, cameraPos_y, cameraPos_z);



	if (death == true) {
		Game_Over();
	}
	draw_universe();
	draw_board();
	draw_sphere();
	if (isMulti) {
		draw_enemy_sphere();
	}
	for (i = 0; i < 5; i++) {
		draw_cube(cubePos[i]);
		draw_normal_cube(normalCubePos[i]);
		draw_hard_cube(hardCubePos[i]);
	}
	for (i = 0; i < 3; i++) {
		draw_hard_cube2(hardCube2Pos[i]);
	}

	glutSwapBuffers(); //--- 화면에 출력하기

	cleanUp(); // 렌더링 후 사용자원 정리
}
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	if (key == '3') {
		view_third_person = true;
		view_first_person = false;
	}
	else if (key == '1') {
		view_third_person = false;
		view_first_person = true;
	}
	else if (key == 'i' || key == 'I') {
		cameraPos_z += 0.5;
		drawScene();
	}
	else if (key == 'o' || key == 'O') {
		cameraPos_z -= 0.5;
		drawScene();
	}
	else if (key == '+') {
		R += 0.1;
		G += 0.1;
		B += 0.1;
		drawScene();
	}
	else if (key == '-') {
		R -= 0.1;
		G -= 0.1;
		B -= 0.1;
		drawScene();
	}
	else if (key == 'q' || key == 'Q') {
		exit(0);
	}
}
GLvoid Special(int key, int x, int y)
{
	CSKeyPacket pmove;
	pmove.type = CSKEYPACKET;
	switch (key)
	{
	case GLUT_KEY_UP:
		pmove.keytype = KEYUP;
		break;
	case GLUT_KEY_DOWN:
		pmove.keytype = KEYDOWN;
		break;
	case GLUT_KEY_RIGHT:
		pmove.keytype = KEYRIGHT;
		break;
	case GLUT_KEY_LEFT:
		pmove.keytype = KEYLEFT;
		break;
	default:
		break;
	}
	send(wSock, (char*)&pmove, sizeof(CSKeyPacket), 0);

}
void game_over_timer(int value)
{
	z_rotate += 5.0;
	timer += 1;
	if (timer > 100) {
		
		CSExitPacket Exit;
		Exit.type = CSEXITPACKET;
		Exit.exit = true;
		send(wSock, (char*)&Exit, sizeof(Exit), 0);

		if (!isMulti) {
			while (1) {
				ReceiveProcess(); // showRankings를 위한 랭킹 리시브 패킷
				if (recvRankings) break;
			}
		}
		printf("EXit\n");

		exit(0);
	}
	drawScene();
	glutTimerFunc(40, game_over_timer, 1);
}
void playingBgm()
{
	openBgm.lpstrElementName = L"universe2.wav";
	openBgm.lpstrDeviceType = L"WaveAudio";
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openBgm);
	dwID0 = openBgm.wDeviceID;
	mciSendCommand(dwID0, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&openBgm);    //음악 반복 재생
}
void Jump_n_Hide_Sound()
{
	jump_n_hideSound.lpstrElementName = L"Jump2.mp3";
	jump_n_hideSound.lpstrDeviceType = L"mpegvideo";
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&jump_n_hideSound);
	dwID = jump_n_hideSound.wDeviceID;
	mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&jump_n_hideSound);    //음악을 한 번 재생
}
void Game_Over()
{
	GLuint VBO, VAO;
	GLfloat vertex[] = {
		-1.5f, 0.0f, 1.5f,   0.0, 1.0, 0.0,	 0.0, 0.0,
		1.5f, 0.0f,  1.5f,	 0.0, 1.0, 0.0,	 1.0, 0.0,
		1.5f, 0.0f, -1.5f,	 0.0, 1.0, 0.0,	 1.0, 1.0,
		1.5f, 0.0f, -1.5f,	 0.0, 1.0, 0.0,	 1.0, 1.0,
		-1.5f, 0.0f, -1.5f,	 0.0, 1.0, 0.0,	 0.0, 1.0,
		-1.5f, 0.0f, 1.5f,   0.0, 1.0, 0.0,	 0.0, 0.0,
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //--- 위치 속성 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); //--- 노말값 속성
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); //--- 텍스처 좌표 속성
	glEnableVertexAttribArray(2);

	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Rx = glm::mat4(1.0);
	glm::mat4 Ry = glm::mat4(1.0);
	glm::mat4 Rz = glm::mat4(1.0);
	glm::mat4 Tm = glm::mat4(1.0);
	glm::mat4 Ts = glm::mat4(1.0);
	Rx = rotate(Rx, glm::radians(80.0f), glm::vec3(1.0, 0.0, 0.0));
	Ry = rotate(Ry, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
	Rz = rotate(Rz, glm::radians(z_rotate), glm::vec3(0.0, 0.0, 1.0));
	Tm = glm::translate(Tm, glm::vec3(0.0, 3.0, 3.0));

	model = Tm * Rx * Ry * Rz;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));


	//--- 텍스처 그릴 부분
	GLuint texture;
	BITMAPINFO* bmp;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); //--- 텍스처 바인딩
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //--- 현재 바인딩된 텍스처의 파라미터 설정하기
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data = LoadDIBitmap("game_over.bmp", &bmp); //--- 텍스처로 사용할 비트맵 이미지 로드하기
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 594, 347, 0, GL_RGB, GL_UNSIGNED_BYTE, data); //---텍스처 이미지 정의

	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);


}
void draw_universe()
{
	GLuint VBO, VAO;
	GLfloat vertex[] = {
		-100.0f, 0.0f, 100.0f, 0.0, 1.0, 0.0,	 0.0, 0.0,
		100.0f, 0.0f,  100.0f,	 0.0, 1.0, 0.0,	 1.0, 0.0,
		100.0f, 0.0f, -100.0f,	 0.0, 1.0, 0.0,	 1.0, 1.0,
		100.0f, 0.0f, -100.0f,	 0.0, 1.0, 0.0,	 1.0, 1.0,
		-100.0f, 0.0f, -100.0f,	 0.0, 1.0, 0.0,	 0.0, 1.0,
		-100.0f, 0.0f, 100.0f, 0.0, 1.0, 0.0,	 0.0, 0.0,
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //--- 위치 속성 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); //--- 노말값 속성
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); //--- 텍스처 좌표 속성
	glEnableVertexAttribArray(2);

	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Rx = glm::mat4(1.0);
	glm::mat4 Ry = glm::mat4(1.0);
	glm::mat4 Rz = glm::mat4(1.0);
	glm::mat4 Tm = glm::mat4(1.0);
	glm::mat4 Ts = glm::mat4(1.0);
	Rx = rotate(Rx, glm::radians(60.0f), glm::vec3(1.0, 0.0, 0.0));
	Ry = rotate(Ry, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
	Rz = rotate(Rz, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	Tm = glm::translate(Tm, glm::vec3(0.0, 20.0, -45.0));

	model = Tm * Rx * Ry * Rz;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));


	//--- 텍스처 그릴 부분

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); //--- 텍스처 바인딩
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //--- 현재 바인딩된 텍스처의 파라미터 설정하기
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = LoadDIBitmap("sky.bmp", &bmp); //--- 텍스처로 사용할 비트맵 이미지 로드하기
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 654, 386, 0, GL_RGB, GL_UNSIGNED_BYTE, data); //---텍스처 이미지 정의

	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 0.2f, 0.2f, 0.2f);
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

}
GLvoid draw_board() {
	GLfloat vertex[] = {
		-4.0f,  0.0f,  6.0f,		1.0f,0.0f,0.0f,  //0번점
		-4.0f,  0.0f, -30.0f,		1.0f,1.0f,0.0f,  //1번점
		 4.0f,  0.0f, -30.0f,		0.0f,1.0f,1.0f,  //2번점
		 4.0f,  0.0f,  6.0f,		0.0f,0.0f,1.0f   //3번점
	};
	glGenBuffers(1, &xzboardVBO);
	glBindBuffer(GL_ARRAY_BUFFER, xzboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	GLint gIndices[]
	{
		0,2,1,
		0,3,2
	};
	glGenBuffers(1, &xzboardEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, xzboardEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Rx = glm::mat4(1.0);
	glm::mat4 Ry = glm::mat4(1.0);
	glm::mat4 Rz = glm::mat4(1.0);
	Rx = rotate(Rx, glm::radians(0.0f), glm::vec3(1.0, 0.0, 0.0));
	Ry = rotate(Ry, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
	Rz = rotate(Rz, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));

	model = Rx * Ry * Rz;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 0.2f, 0.2f, 0.2f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}
GLvoid draw_sphere()
{
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Tt = glm::mat4(1.0f);
	glm::mat4 Ttt = glm::mat4(1.0f);
	Tt = glm::translate(Tt, glm::vec3(sphere_pos_x, 0.0, sphere_pos_z));
	Ttt = glm::translate(Ttt, glm::vec3(0.0, sphere_pos_y, 0.0));
	model = Tt * Ttt;

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glBindBuffer(GL_ARRAY_BUFFER, eVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eEBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, sphere_hp_color, 1.0f, sphere_hp_color);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 2880, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
GLvoid draw_enemy_sphere()
{
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Tt = glm::mat4(1.0f);
	glm::mat4 Ttt = glm::mat4(1.0f);
	Tt = glm::translate(Tt, glm::vec3(enemy_sphere_pos_x, 0.0, enemy_sphere_pos_z));
	Ttt = glm::translate(Ttt, glm::vec3(0.0, enemy_sphere_pos_y, 0.0));
	model = Tt * Ttt;

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glBindBuffer(GL_ARRAY_BUFFER, enemy_eVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, enemy_eEBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 0.9f, 0.9f, 0.9f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 2880, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void draw_cube(POSXYZ cubePos)
{
	GLfloat vertex[] = {
		-0.5f, 1.0f, -0.5f,		-1.0f, 1.0f, -1.0f,  //0번점
		-0.5f, 1.0f, 0.5f,		-1.0f ,1.0f, 1.0f,  //1번점
		0.5f, 1.0f, 0.5f,		1.0f, 1.0f,  1.0f,  //2번점
		0.5f, 1.0f, -0.5f,		1.0f, 1.0f, -1.0f,  //3번점

		-0.5f, 0.0f, -0.5f,		-1.0f, -1.0f, -1.0f,  //4번점
		-0.5f, 0.0f, 0.5f,		-1.0f, -1.0f, 1.0f,  //5번점
		0.5f, 0.0f, 0.5f,		1.0f, -1.0f,  1.0f,  //6번점
		0.5f, 0.0f, -0.5f,		1.0f, -1.0f, -1.0f,  //7번점
	};
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);


	GLint gIndices[]
	{
		0,1,2,
		0,2,3,  //윗면

		1,5,6,
		1,6,2, //앞면

		2,6,7,
		2,7,3, //우측면

		0,4,5,
		0,5,1, //좌측면

		5,4,6,
		4,7,6,// 밑면

		0,7,4, //뒷면
		0,3,7
	};
	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);



	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Rx = glm::mat4(1.0);
	glm::mat4 Ry = glm::mat4(1.0);
	glm::mat4 Rz = glm::mat4(1.0);
	glm::mat4 Tx = glm::mat4(1.0);
	glm::mat4 Txx = glm::mat4(1.0);
	glm::mat4 Ty = glm::mat4(1.0);
	glm::mat4 Sm = glm::mat4(1.0);


	Rx = rotate(Rx, glm::radians(00.0f), glm::vec3(1.0, 0.0, 0.0));
	Ry = rotate(Ry, glm::radians(00.0f), glm::vec3(0.0, 1.0, 0.0));
	Rz = rotate(Rz, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	Sm = glm::scale(Sm, glm::vec3(x_scale, y_scale, z_scale));

	Txx = glm::translate(Txx, glm::vec3(-3.0, 0.0, 0.0));
	Tx = glm::translate(Tx, glm::vec3(cubePos.posX, 0.0, cubePos.posZ));
	Ty = glm::translate(Ty, glm::vec3(0.0, cubePos.posY, 0.0));

	model = Rx * Ry * Rz * Txx * Tx * Ty;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 0.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void draw_normal_cube(POSXYZ cubePos)
{
	GLfloat vertex[] = {
		-0.5f, 1.0f, -0.5f,		-1.0f, 1.0f, -1.0f,  //0번점
		-0.5f, 1.0f, 0.5f,		-1.0f ,1.0f, 1.0f,  //1번점
		0.5f, 1.0f, 0.5f,		1.0f, 1.0f,  1.0f,  //2번점
		0.5f, 1.0f, -0.5f,		1.0f, 1.0f, -1.0f,  //3번점

		-0.5f, 0.0f, -0.5f,		-1.0f, -1.0f, -1.0f,  //4번점
		-0.5f, 0.0f, 0.5f,		-1.0f, -1.0f, 1.0f,  //5번점
		0.5f, 0.0f, 0.5f,		1.0f, -1.0f,  1.0f,  //6번점
		0.5f, 0.0f, -0.5f,		1.0f, -1.0f, -1.0f,  //7번점
	};
	glGenBuffers(1, &nCubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, nCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	GLint gIndices[]
	{
		0,1,2,
		0,2,3,  //윗면

		1,5,6,
		1,6,2, //앞면

		2,6,7,
		2,7,3, //우측면

		0,4,5,
		0,5,1, //좌측면

		5,4,6,
		4,7,6,// 밑면

		0,7,4, //뒷면
		0,3,7
	};
	glGenBuffers(1, &nCubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nCubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Rx = glm::mat4(1.0);
	glm::mat4 Ry = glm::mat4(1.0);
	glm::mat4 Rz = glm::mat4(1.0);
	glm::mat4 Tx = glm::mat4(1.0);
	glm::mat4 Txx = glm::mat4(1.0);
	glm::mat4 Ty = glm::mat4(1.0);
	glm::mat4 Sm = glm::mat4(1.0);


	Rx = rotate(Rx, glm::radians(00.0f), glm::vec3(1.0, 0.0, 0.0));
	Ry = rotate(Ry, glm::radians(00.0f), glm::vec3(0.0, 1.0, 0.0));
	Rz = rotate(Rz, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	Sm = glm::scale(Sm, glm::vec3(x_scale, y_scale, z_scale));
	Txx = glm::translate(Txx, glm::vec3(-3.0, 0.0, 0.0));
	Tx = glm::translate(Tx, glm::vec3(cubePos.posX, 0.0, cubePos.posZ));
	Ty = glm::translate(Ty, glm::vec3(0.0, cubePos.posY, 0.0));

	model = Rx * Ry * Rz * Txx * Tx * Ty;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 0.83f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void draw_hard_cube(POSXYZ cubePos)
{
	GLfloat vertex[] = {
		-1.0f, 2.0f, -0.5f,		-1.0f, 1.0f, -1.0f,  //0번점
		-1.0f, 2.0f, 0.5f,		-1.0f ,1.0f, 1.0f,  //1번점
		1.0f, 2.0f, 0.5f,		1.0f, 1.0f,  1.0f,  //2번점
		1.0f, 2.0f, -0.5f,		1.0f, 1.0f, -1.0f,  //3번점

		-1.0f, 0.0f, -0.5f,		-1.0f, -1.0f, -1.0f,  //4번점
		-1.0f, 0.0f, 0.5f,		-1.0f, -1.0f, 1.0f,  //5번점
		1.0f, 0.0f, 0.5f,		1.0f, -1.0f,  1.0f,  //6번점
		1.0f, 0.0f, -0.5f,		1.0f, -1.0f, -1.0f,  //7번점
	};
	glGenBuffers(1, &hCubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, hCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	GLint gIndices[]
	{
		0,1,2,
		0,2,3,  //윗면

		1,5,6,
		1,6,2, //앞면

		2,6,7,
		2,7,3, //우측면

		0,4,5,
		0,5,1, //좌측면

		5,4,6,
		4,7,6,// 밑면

		0,7,4, //뒷면
		0,3,7
	};
	glGenBuffers(1, &hCubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hCubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Rx = glm::mat4(1.0);
	glm::mat4 Ry = glm::mat4(1.0);
	glm::mat4 Rz = glm::mat4(1.0);
	glm::mat4 Tx = glm::mat4(1.0);
	glm::mat4 Txx = glm::mat4(1.0);
	glm::mat4 Ty = glm::mat4(1.0);
	glm::mat4 Sm = glm::mat4(1.0);


	Rx = rotate(Rx, glm::radians(00.0f), glm::vec3(1.0, 0.0, 0.0));
	Ry = rotate(Ry, glm::radians(00.0f), glm::vec3(0.0, 1.0, 0.0));
	Rz = rotate(Rz, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	Sm = glm::scale(Sm, glm::vec3(x_scale, y_scale, z_scale));
	Txx = glm::translate(Txx, glm::vec3(-3.0, 0.0, 0.0));
	Tx = glm::translate(Tx, glm::vec3(cubePos.posX, 0.0, cubePos.posZ));
	Ty = glm::translate(Ty, glm::vec3(0.0, cubePos.posY, 0.0));

	model = Rx * Ry * Rz * Txx * Tx * Ty;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 0.5f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void draw_hard_cube2(POSXYZ cubePos)
{
	GLfloat vertex[] = {
		-4.0f, 1.0f, -0.5f,		-1.0f, 1.0f, -1.0f,  //0번점
		-4.0f, 1.0f, 0.5f,		-1.0f ,1.0f, 1.0f,  //1번점
		4.0f, 1.0f, 0.5f,		1.0f, 1.0f,  1.0f,  //2번점
		4.0f, 1.0f, -0.5f,		1.0f, 1.0f, -1.0f,  //3번점

		-4.0f, 0.0f, -0.5f,		-1.0f, -1.0f, -1.0f,  //4번점
		-4.0f, 0.0f, 0.5f,		-1.0f, -1.0f, 1.0f,  //5번점
		4.0f, 0.0f, 0.5f,		1.0f, -1.0f,  1.0f,  //6번점
		4.0f, 0.0f, -0.5f,		1.0f, -1.0f, -1.0f,  //7번점
	};
	glGenBuffers(1, &h2CubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, h2CubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	GLint gIndices[]
	{
		0,1,2,
		0,2,3,  //윗면

		1,5,6,
		1,6,2, //앞면

		2,6,7,
		2,7,3, //우측면

		0,4,5,
		0,5,1, //좌측면

		5,4,6,
		4,7,6,// 밑면

		0,7,4, //뒷면
		0,3,7
	};
	glGenBuffers(1, &h2CubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, h2CubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환
	glm::mat4 Rx = glm::mat4(1.0);
	glm::mat4 Ry = glm::mat4(1.0);
	glm::mat4 Rz = glm::mat4(1.0);
	glm::mat4 Tx = glm::mat4(1.0);
	glm::mat4 Txx = glm::mat4(1.0);
	glm::mat4 Ty = glm::mat4(1.0);
	glm::mat4 Sm = glm::mat4(1.0);


	Rx = rotate(Rx, glm::radians(00.0f), glm::vec3(1.0, 0.0, 0.0));
	Ry = rotate(Ry, glm::radians(00.0f), glm::vec3(0.0, 1.0, 0.0));
	Rz = rotate(Rz, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	Sm = glm::scale(Sm, glm::vec3(x_scale, y_scale, z_scale));
	Txx = glm::translate(Txx, glm::vec3(-3.0, 0.0, 0.0));
	Tx = glm::translate(Tx, glm::vec3(cubePos.posX, 0.0, cubePos.posZ));
	Ty = glm::translate(Ty, glm::vec3(0.0, cubePos.posY, 0.0));

	model = Rx * Ry * Rz * Txx * Tx * Ty;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 0.0f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void draw_land()
{
	GLfloat vertex[] = {
		-30.0f,  -0.05f,  30.0f,		1.0f,0.0f,0.0f,  //0번점
		-30.0f,  -0.05f, -100.0f,		1.0f,1.0f,0.0f,  //1번점
		 30.0f,  -0.05f, -100.0f,		0.0f,1.0f,1.0f,  //2번점
		 30.0f,  -0.05f,  30.0f,		0.0f,0.0f,1.0f   //3번점
	};
	glGenBuffers(1, &landVBO);
	glBindBuffer(GL_ARRAY_BUFFER, landVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	GLint gIndices[]
	{
		0,2,1,
		0,3,2
	};
	glGenBuffers(1, &landEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, landEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");
	glm::mat4 model = glm::mat4(1.0f);		// 모델변환

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));		// 모델변환
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	glUniform3f(objColorLocation, 0.9f, 0.9f, 0.9f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void cleanUp()
{
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteBuffers(1, &nCubeVBO);
	glDeleteBuffers(1, &nCubeEBO);
	glDeleteBuffers(1, &hCubeVBO);
	glDeleteBuffers(1, &hCubeEBO);
	glDeleteBuffers(1, &h2CubeVBO);
	glDeleteBuffers(1, &h2CubeEBO);
	glDeleteBuffers(1, &xzboardEBO);
	glDeleteBuffers(1, &xzboardVBO);
	glDeleteTextures(1, &texture);
	free(data);

}

void render(int value)
{
	if (isWin) {
		// 클라에서 자체적으로 몇 초 pause 후, pause가 끝났다고 서버에 send
		// 서버가 send를 받으면, 저장했던 정보를 다시 보내줄 것임.
		printf("당신이 이겼습니다!\n");
		for (int i = 5; i >= 1; --i) {
			printf("%d초 후 시작\n", i);
		}
		CSReStartPacket reStart;
		reStart.type = CSRESTARTPACKET;
		reStart.start = true;
		send(wSock, (char*)&reStart, sizeof(CSReStartPacket), 0);
		isWin = false;
	}

	ReceiveProcess();	// obstacle 움직임 수신을 위한 recv
	ReceiveProcess();	// 캐릭터 패킷 움직임 수신을 위한 recv
	ReceiveProcess();	// 적 패킷 움직임 수신을 위한 recv

	drawScene();
	if (!death) {
		glutTimerFunc(30, render, 1);
	}
	else {
		if (!isMulti) {
			CSInitialPacket rank;
			rank.type = CSINITIALPACKET;
			printf("랭킹 등록 이름을 작성해주세요\n");
			std::cin >> rank.rank.name;
			send(wSock, (char*)&rank, sizeof(CSInitialPacket), 0);
		}
		glutTimerFunc(40, game_over_timer, 1);
	}
}