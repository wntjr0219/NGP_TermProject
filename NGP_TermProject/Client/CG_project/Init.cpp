#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include "Init.h"
GLubyte* LoadDIBitmap(const char* filename, BITMAPINFO** info)
{
	FILE* fp;
	GLubyte* bits;
	int bitsize, infosize;
	BITMAPFILEHEADER header;
	//--- ���̳ʸ� �б� ���� ������ ����
	if ((fp = fopen(filename, "rb")) == NULL)
		return NULL;
	//--- ��Ʈ�� ���� ����� �д´�.
	if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1) {
		fclose(fp); return NULL;
	}
	//--- ������ BMP �������� Ȯ���Ѵ�.
	if (header.bfType != 'MB') {
		fclose(fp); return NULL;
	}
	//--- BITMAPINFOHEADER ��ġ�� ����.
	infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
	//--- ��Ʈ�� �̹��� �����͸� ���� �޸� �Ҵ��� �Ѵ�.
	if ((*info = (BITMAPINFO*)malloc(infosize)) == NULL) {
		fclose(fp); return NULL;
	}
	//--- ��Ʈ�� ���� ����� �д´�.
	if (fread(*info, 1, infosize, fp) < (unsigned int)infosize) {
		free(*info);
		fclose(fp); return NULL;
	}
	//--- ��Ʈ���� ũ�� ����
	if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0) {
		bitsize = ((*info)->bmiHeader.biWidth * (*info)->bmiHeader.biBitCount + 7 / 8.0 * abs((*info)->bmiHeader.biHeight));
	}
	//--- ��Ʈ���� ũ�⸸ŭ �޸𸮸� �Ҵ��Ѵ�.
	if ((bits = (unsigned char*)malloc(bitsize)) == NULL) {
		free(*info);
		fclose(fp); return NULL;
	}
	//--- ��Ʈ�� �����͸� bit(GLubyte Ÿ��)�� �����Ѵ�.
	if (fread(bits, 1, bitsize, fp) < (unsigned int)bitsize) {
		free(*info); free(bits);
		fclose(fp); return NULL;
	}
	fclose(fp);
	return bits;

}
const char* filetobuf(const char* file) {
	FILE* fptr;
	long length;
	char* buf;
	fopen_s(&fptr, file, "rb"); // Open file for reading 
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
	fclose(fptr); // Close the file 
	buf[length] = 0; // Null terminator 
	return buf; // Return the buffer 
}
bool InitProgram(unsigned int& ShaderProgram)
{
	const char* vertexSource = filetobuf("vertex.glsl");
	const char* fragmentSource = filetobuf("fragment.glsl");
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	if (!Check(vertexShader)) return false;

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	if (!Check(fragmentShader)) return false;

	ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, vertexShader);
	glAttachShader(ShaderProgram, fragmentShader);
	glLinkProgram(ShaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if (!Check(ShaderProgram)) {
		glDeleteProgram(ShaderProgram);
		return false;

	}

}
bool Check(unsigned int ShaderProgram)
{
	GLint state;
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &state);
	if (state == GL_FALSE) {
		int infologLength = 0;
		glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 0) {
			char* infolog = new char[infologLength];
			glGetProgramInfoLog(ShaderProgram, infologLength, NULL, infolog);
			std::cout << infolog << std::endl;
			delete[] infolog;
		}
		return false;
	}
	return true;
}
void Create_xyz_axis(GLuint& VBO)
{
	GLfloat vertex[] = {
		-1.0f, 0.0f, 0.0f,		1.0f,   1.0f,  1.0f,
		1.0f,  0.0f, 0.0f,		1.0f,   1.0f,  1.0f,
		0.0f, -1.0f, 0.0f,		1.0f,   1.0f,  1.0f,
		0.0f,  1.0f, 0.0f,		1.0f,   1.0f,  1.0f,
		0.0f,  0.0f, -1.0f,		1.0f,   1.0f,  1.0f,
		0.0f,  0.0f, 1.0f,		1.0f,   1.0f,  1.0f
	};
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
}
void CreateCone(GLuint& ConEBO, GLuint& ConVBO)
{
	GLfloat vertexData[] = {
		0.0f,  0.5f, 0.0f,		0.0f,    1.0f,  0.0f,	//0����
		-0.3f, 0.0f, -0.3f,		-0.5f, -0.25f, -0.5f,   //1����
		-0.3f, 0.0f, 0.3f,		-0.5f, -0.25f,  0.5f,   //2����
		0.3f,  0.0f, 0.3f,		0.5f,  -0.25f,  0.5f,   //3����
		0.3f,  0.0f, -0.3f,		0.5f,  -0.25f, -0.5f    //4����
	};
	glGenBuffers(1, &ConVBO);

	glBindBuffer(GL_ARRAY_BUFFER, ConVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	unsigned int indexDeta[] = {
		2,1,3,
		3,1,4, //�ظ�
		0,2,3, //�ո�
		0,1,2,//������
		0,4,1,//�޸�
		0,3,4,//������	
	};
	glGenBuffers(1, &ConEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ConEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 18, &indexDeta, GL_STATIC_DRAW);
}
void CreateBoard(GLuint& EBO, GLuint& VBO)
{
	GLfloat vertex[] = {
	-2.0f,  0.0f,  2.0f,		0.0f,0.0f,1.0f,  //0����
	-2.0f,  0.0f, -2.0f,		0.0f,0.0f,1.0f,  //1����
	 2.0f,  0.0f, -2.0f,		0.0f,0.0f,1.0f,  //2����
	 2.0f,  0.0f,  2.0f,		0.0f,0.0f,1.0f   //3����
	};

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	GLint gIndices[]
	{
		0,2,1,
		0,3,2
	};

	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);

}
void CreateCube(GLuint& EBO, GLuint& VBO)
{
	GLfloat vertex[] = {
		-0.5f, 1.0f, -0.5f,		-1.0f, 1.0f, -1.0f,  //0����
		-0.5f, 1.0f, 0.5f,		-1.0f ,1.0f, 1.0f,  //1����
		0.5f, 1.0f, 0.5f,		1.0f, 1.0f,  1.0f,  //2����
		0.5f, 1.0f, -0.5f,		1.0f, 1.0f, -1.0f,  //3����

		-0.5f, 0.0f, -0.5f,		-1.0f, -1.0f, -1.0f,  //4����
		-0.5f, 0.0f, 0.5f,		-1.0f, -1.0f, 1.0f,  //5����
		0.5f, 0.0f, 0.5f,		1.0f, -1.0f,  1.0f,  //6����
		0.5f, 0.0f, -0.5f,		1.0f, -1.0f, -1.0f,  //7����
	};
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	//front
	GLint gIndices[]
	{
		0,1,2,
		0,2,3,  //����

		1,5,6,
		1,6,2, //�ո�

		2,6,7,
		2,7,3, //������

		0,4,5,
		0,5,1, //������

		5,4,6,
		4,7,6,// �ظ�

		0,7,4, //�޸�
		0,3,7
	};

	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);
}
void CreatePolygon(GLuint& VBO, GLuint& EBO)
{
	GLfloat vertex[] = {
		-0.1f,  -0.1f,  0.0f,		0.0f,0.0f,1.0f,  //0����
		-0.1f,  0.1f,  0.0f,		0.0f,0.0f,1.0f,  //1����
		 0.1f,  0.1f,  0.0f,		0.0f,0.0f,1.0f,  //2����
		 0.1f,  -0.1f,  0.0f,		0.0f,0.0f,1.0f   //3����
	};

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	GLint gIndices[]
	{
		0,2,1,
		0,3,2
	};

	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);
}

void CreateSphere(GLuint& EBO, GLuint& VBO)
{
	GLfloat vertices[2892] = { 0, };

	GLuint gIndices[2880] = { 0, };

	int vcount = 0;
	int ncount = 0;
	int icount = 0;


	FILE* objFile;

	fopen_s(&objFile, "sphere.obj", "rb");

	if (objFile == NULL) {
		printf("Impossible to open the file !\n");
	}
	while (1) {

		char lineHeader[256];
		// read the first word of the line
		int res = fscanf(objFile, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(objFile, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);

			vertex.x = vertex.x / 50;
			vertex.y = vertex.y / 50;
			vertex.z = vertex.z / 50;

			vertices[vcount] = vertex.x;
			vertices[vcount + 1] = vertex.y;
			vertices[vcount + 2] = vertex.z;

			vcount = vcount + 6;


		}
		else if (strcmp(lineHeader, "vt") == 0) {
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(objFile, "%f %f %f\n", &normal.x, &normal.y, &normal.z);

			vertices[ncount + 3] = normal.x;
			vertices[ncount + 4] = normal.y;
			vertices[ncount + 5] = normal.z;

			ncount = ncount + 6;

		}
		else if (strcmp(lineHeader, "f") == 0) {
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(objFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
			}

			gIndices[icount] = vertexIndex[0];
			gIndices[icount + 1] = vertexIndex[1];
			gIndices[icount + 2] = vertexIndex[2];

			icount = icount + 3;
		}
	}

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), &gIndices, GL_STATIC_DRAW);

}