#pragma once
#include "OpenGL.h"
#include "protocol.h"
#include "Obstacle.h"
MCI_OPEN_PARMS openBgm;
MCI_OPEN_PARMS jump_n_hideSound;
MCI_OPEN_PARMS collideSound;

void Jump_n_Hide_Sound();
void playingBgm();


POSXYZ cubePos[5];
POSXYZ normalCubePos[5];
POSXYZ hardCubePos[5];
POSXYZ hardCube2Pos[3];
std::vector<Obstacle> Obstacles;

GLuint texture;
BITMAPINFO* bmp;
unsigned char* data;

int dwID0;
int dwID;
float sphere_hp_color = 0.0;

int meter = 0; // 점수 역할 , 난이도 조정장치(일정 meter 이상 가면 난이도 오르고 상자 추가되는형식)


void game_over_timer(int value);
void cube_move_timer(int value);
void sphere_jump_timer(int value);
void sphere_hide_timer(int value);
bool jumped = false;
bool falling = false;
bool jumpRunning = false;
bool hideRunning = false;
bool hide = false;
bool unhide = false;
