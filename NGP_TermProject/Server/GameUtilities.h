#pragma once
#include "OpenGL.h"
MCI_OPEN_PARMS openBgm;
MCI_OPEN_PARMS jump_n_hideSound;
MCI_OPEN_PARMS collideSound;

void Jump_n_Hide_Sound();
void playingBgm();

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
float sphere_pos_x = 0.0;
float sphere_pos_y = 0.4;
float sphere_pos_z = 3.0;
float z_rotate = 0.0;
int i = 0;
int timer = 0;
int meter = 0; // 점수 역할 , 난이도 조정장치(일정 meter 이상 가면 난이도 오르고 상자 추가되는형식)
float cube_pos_x[100]; float cube_pos_y[100]; float cube_pos_z[100];
float normal_cube_pos_x[100]; float normal_cube_pos_y[100]; float normal_cube_pos_z[100];
float hard_cube_pos_x[100]; float hard_cube_pos_y[100]; float hard_cube_pos_z[100];
float hard_cube2_pos_x[100]; float hard_cube2_pos_y[100]; float hard_cube2_pos_z[100];

void game_over_timer(int value);
void cube_move_timer(int value);
void sphere_jump_timer(int value);
void sphere_hide_timer(int value);
bool collide = false;
bool jumped = false;
bool falling = false;
bool running = false;
bool hide = false;
bool unhide = false;
bool view_third_person = true;
bool view_first_person = false;
bool death = false;
void Game_Over();
void draw_land();
void draw_universe();
void draw_board();
void draw_sphere();
void draw_cube();
void draw_normal_cube();
void draw_hard_cube();
void draw_hard_cube2();
bool handle_collide(int i);
bool normal_handle_collide(int i);
bool hard_handle_collide(int i);
bool hard2_handle_collide(int i);
GLuint eVBO, eEBO;
GLuint xzboardVBO, xzboardEBO;
GLuint cubeVBO, cubeEBO;
GLuint s_program;
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Special(int key, int x, int y);