#pragma once
#include "OpenGL.h"
#include "protocol.h"
MCI_OPEN_PARMS openBgm;
MCI_OPEN_PARMS jump_n_hideSound;
MCI_OPEN_PARMS collideSound;

void Jump_n_Hide_Sound();
void playingBgm();


POSXYZ cubePos[5];
POSXYZ normalCubePos[5];
POSXYZ hardCubePos[5];
POSXYZ hardCube2Pos[3];

GLuint texture;
BITMAPINFO* bmp;
unsigned char* data;

int dwID0;
int dwID;


float sphere_hp_color = 0.0;
float x_scale = 4.0;
float y_scale = 4.0;
float z_scale = 4.0;
float R = 1.0;
float G = 1.0;
float B = 1.0;
float cameraPos_x = 2.5;
float cameraPos_y = 2.0;
float cameraPos_z = 4.75;
float x_m_camera = 0.0;
float z_m_camera = -10.0;

// 캐릭터(공) 위치 초기값
float sphere_pos_x = 0.0;
float sphere_pos_y = 0.4;
float sphere_pos_z = 3.0;
float enemy_sphere_pos_x = 0.0;
float enemy_sphere_pos_y = 0.4;
float enemy_sphere_pos_z = 3.0;

float z_rotate = 0.0;
int i = 0;
int timer = 0;
int meter = 0; // 점수 역할 , 난이도 조정장치(일정 meter 이상 가면 난이도 오르고 상자 추가되는형식)

// POSXYZ 구조체로 변경할 예정
POSXYZ EnemysPos, PlayerPos;
POSXYZ Obstacles;

void game_over_timer(int value);
bool recvRankings = false;
bool isMulti = false;
bool isWin = false;
bool collide = false;
bool jumped = false;
bool falling = false;
bool running = false;
bool hide = false;
bool unhide = false;
bool view_third_person = true;
bool view_first_person = false;
bool death = false;
void render(int value);
void Game_Over();
void draw_land();
void draw_universe();
void draw_board();
void draw_sphere();
void draw_enemy_sphere();
void draw_cube(POSXYZ cubePos);
void draw_normal_cube(POSXYZ cubePos);
void draw_hard_cube(POSXYZ cubePos);
void draw_hard_cube2(POSXYZ cubePos);
GLuint eVBO, eEBO;
GLuint enemy_eVBO, enemy_eEBO;
GLuint xzboardVBO, xzboardEBO, landVBO, landEBO;
GLuint cubeVBO, cubeEBO, nCubeVBO, nCubeEBO, hCubeVBO, hCubeEBO, h2CubeVBO, h2CubeEBO;
GLuint s_program;
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Special(int key, int x, int y);

void cleanUp();