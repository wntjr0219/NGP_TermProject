#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cmath>
#include "openGL.h"

GLubyte* LoadDIBitmap(const char* filename, BITMAPINFO** info);
bool InitProgram(unsigned int& ShaderProgram);
bool Check(unsigned int ShaderProgram);
void Create_xyz_axis(GLuint& VBO);
void CreateCone(GLuint& EBO, GLuint& VBO);
void CreateBoard(GLuint& EBO, GLuint& VBO);
void CreatePolygon(GLuint& VBO, GLuint& EBO);
void CreateSphere(GLuint& EBO, GLuint& VBO);
void CreateCube(GLuint& EBO, GLuint& VBO);