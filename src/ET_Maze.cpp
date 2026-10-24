/**************************************************************************************************
* SuperSDG v2.2 release																              *
* Copyright (c) 2002-2008 Ravent 																  *
* All rights reserved																              *
**************************************************************************************************/
#pragma comment (lib,"glut32.lib")
#pragma comment (lib,"glaux.lib")
#pragma comment (lib, "legacy_stdio_definitions.lib")


#define _USE_MATH_DEFINES

#include "glaux.h"
#include "glut.h"
#include "playerObj.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <iostream>


////		variables		////

AUX_RGBImageRec *imageWall = NULL, *imageFloor = NULL, *imageitems = NULL, *imageBatterys = NULL, *localWayOut = NULL;

GLuint playerUnit;
GLuint miniUnit;

const int mapSize = 15, mm = mapSize + 1;
const int speedDelay = 5;
const int turnSpeed = 4;
const int turnDelay = 9;
const int direction_parts = 36;
const int move[4][2] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };
const int move_key[4] = { GLUT_KEY_UP, GLUT_KEY_LEFT, GLUT_KEY_DOWN, GLUT_KEY_RIGHT };
const int batteryMax = 50;

int viewKind = 1;
int width, height;
int playerDir=0;
int mapDir = 0;
int cur_direction = 0;
int playerObjNum = 0;
int glWin, pathLen;
int startTime, currentTime, tempTime = 0;


unsigned textureWall = -1, textureFloor = -1, textureItems = -1, textureBatterys = -1, textureWayOut = -1;

double cameraDistance = 4.;

bool viewChanged = false;

time_t start_t = 0;
time_t current_t = 0;


char zone[mapSize + 2][mapSize + 2], cp[mapSize + 2][mapSize + 2];
float lightAmb[] = { 0.03, 0.03, 0.03 };
float lightDif[] = { 0.95, 0.95, 0.95 };


/////		structs		////

struct Tpos{
	int x, y;
};

struct Player{
	float x, y, z;
	float dx, dz;
	float px,py,pz;
	bool isGo;
	bool isTurn = false;
	int turnDir;
	int getItems = 0;
}player;

struct GameInfo {
	int time;
	int score;
}gameInfo;

struct Objects {
	float x=0, y=0, z=0;
	float gapX, gapY, gapZ;
}camera, target, wayOut;

struct Items {
	float x = 0, y = 0, z = 0;
	int cur;
	int dir;
} item1, item2, batterys[batteryMax];



struct coordinate{
	float x, y, z;
	coordinate(float a, float b, float c) : x(a), y(b), z(c) {};
};

//for faces, it can contain triangles and quads as well, the four variable contain which is that
struct face{
	int facenum;
	bool four;
	int faces[4];
	face(int facen, int f1, int f2, int f3) : facenum(facen){	//constructor for triangle
		faces[0] = f1;
		faces[1] = f2;
		faces[2] = f3;
		four = false;
	}
	face(int facen, int f1, int f2, int f3, int f4) : facenum(facen){ //overloaded constructor for quad
		faces[0] = f1;
		faces[1] = f2;
		faces[2] = f3;
		faces[3] = f4;
		four = true;
	}
};

struct MazeMap{
	int x, z;
} map;



////		functions		////

void start();
void finish();
void display(void);
void halt(bool f = false);
void Draw_Floor(GLfloat x1, GLfloat x2, GLfloat z1, GLfloat z2, unsigned texture);
void Draw_Box(GLint j, GLint i, unsigned texture);
void Draw_Player();
void Draw_Items(float x, float y, float z, unsigned texture);
void Draw_Batterys(int i,unsigned texture);
void Draw_WayOut(unsigned texture);
void Move_Camera();
int loadObj(char *fname);
void animate();
void init();
void reshape(int w, int h);
void key(unsigned char key, int x, int y);
bool good_move(int z, int x);
void keys(int key, int x, int y);
void display();
void genMap(void);
bool isGood(Tpos a);
int step(int y, int x);
void zoneToCp(void);
void fill(int y, int x, Tpos *v, int *l);
void displayUpdate();
void timeCheck();

void logCat_int(std::string str, int i);
void logCat_float(std::string str, float f);
void logCat_bool(std::string str, bool b);


////		main		////

int main(int argc, char** argv)
{

	start();

	genMap();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);

	glWin = glutCreateWindow("ObjLoader");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutSpecialFunc(keys);
	glutIdleFunc(animate);

	imageWall = auxDIBImageLoad(L"wall.bmp");
	glGenTextures(1, &textureWall);
	glBindTexture(GL_TEXTURE_2D, textureWall);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageWall->sizeX, imageWall->sizeY, GL_RGB, GL_UNSIGNED_BYTE, imageWall->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	imageFloor = auxDIBImageLoad(L"floor.bmp");
	glGenTextures(1, &textureFloor);
	glBindTexture(GL_TEXTURE_2D, textureFloor);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageFloor->sizeX, imageFloor->sizeY, GL_RGB, GL_UNSIGNED_BYTE, imageFloor->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	imageitems = auxDIBImageLoad(L"item.bmp");
	glGenTextures(1, &textureItems);
	glBindTexture(GL_TEXTURE_2D, textureItems);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageitems->sizeX, imageitems->sizeY, GL_RGB, GL_UNSIGNED_BYTE, imageitems->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	

	imageBatterys = auxDIBImageLoad(L"batterys.bmp");
	glGenTextures(1, &textureBatterys);
	glBindTexture(GL_TEXTURE_2D, textureBatterys);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageBatterys->sizeX, imageBatterys->sizeY, GL_RGB, GL_UNSIGNED_BYTE, imageBatterys->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	localWayOut = auxDIBImageLoad(L"wayout.bmp");
	glGenTextures(1, &textureWayOut);
	glBindTexture(GL_TEXTURE_2D, textureWayOut);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, localWayOut->sizeX, localWayOut->sizeY, GL_RGB, GL_UNSIGNED_BYTE, localWayOut->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glutFullScreen();
	glutMainLoop();

	

	return 0;
}



////		detail		////

int loadObj(char *fname)
{
	std::vector<std::string*> coord;	//read every single line of the obj file as a string
	std::vector<coordinate*> vertex;
	std::vector<face*> faces;
	std::vector<coordinate*> normals;	//normal vectors for every face
	std::ifstream in(fname);	//open the .obj file
	if (!in.is_open())	//if not opened, exit with -1
	{
		std::cout << "Nor oepened" << std::endl;
		return -1;
	}
	char buf[256];
	//read in every line to coord
	while (!in.eof())
	{
		in.getline(buf, 256);
		coord.push_back(new std::string(buf));
	}
	//go through all of the elements of coord, and decide what kind of element is that
	for (int i = 0; i < coord.size(); i++)
	{
		if (coord[i]->c_str()[0] == '#')	//if it is a comment (the first character is #)
			continue;	//we don't care about that
		else if (coord[i]->c_str()[0] == 'v' && coord[i]->c_str()[1] == ' ')	//if vector
		{
			float tmpx, tmpy, tmpz;
			sscanf_s(coord[i]->c_str(), "v %f %f %f", &tmpx, &tmpy, &tmpz);	//read in the 3 float coordinate to tmpx,tmpy,tmpz
			vertex.push_back(new coordinate(tmpx, tmpy, tmpz));	//and then add it to the end of our vertex list
		}
		else if (coord[i]->c_str()[0] == 'v' && coord[i]->c_str()[1] == 'n')	//if normal vector
		{
			float tmpx, tmpy, tmpz;	//do the same thing
			sscanf_s(coord[i]->c_str(), "vn %f %f %f", &tmpx, &tmpy, &tmpz);
			normals.push_back(new coordinate(tmpx, tmpy, tmpz));
		}
		else if (coord[i]->c_str()[0] == 'f')	//if face
		{
			int a, b, c, d, e;
			if (count(coord[i]->begin(), coord[i]->end(), ' ') == 3)	//if it is a triangle (it has 3 space in it)
			{
				sscanf_s(coord[i]->c_str(), "f %d//%d %d//%d %d//%d", &a, &b, &c, &b, &d, &b);
				faces.push_back(new face(b, a, c, d));	//read in, and add to the end of the face list
			}
			else {
				sscanf_s(coord[i]->c_str(), "f %d//%d %d//%d %d//%d %d//%d", &a, &b, &c, &b, &d, &b, &e, &b);
				faces.push_back(new face(b, a, c, d, e));	//do the same, except we call another constructor, and we use different pattern
			}
		}
	}

	//raw
	int num;	//the id for the list
	num = glGenLists(1);	//generate a uniqe
	glNewList(num, GL_COMPILE);	//and create it
	for (int i = 0; i < faces.size(); i++)
	{
		if (faces[i]->four)	//if it's a quad draw a quad
		{
			glBindTexture(GL_TEXTURE, textureWall);
			glBegin(GL_QUADS);
			
			//basically all I do here, is use the facenum (so the number of the face) as an index for the normal, so the 1st normal owe to the first face
			//I subtract 1 because the index start from 0 in C++
			glNormal3f(normals[faces[i]->facenum - 1]->x, normals[faces[i]->facenum - 1]->y, normals[faces[i]->facenum - 1]->z);
			//draw the faces
			glVertex3f(vertex[faces[i]->faces[0] - 1]->x, vertex[faces[i]->faces[0] - 1]->y, vertex[faces[i]->faces[0] - 1]->z);
			glVertex3f(vertex[faces[i]->faces[1] - 1]->x, vertex[faces[i]->faces[1] - 1]->y, vertex[faces[i]->faces[1] - 1]->z);
			glVertex3f(vertex[faces[i]->faces[2] - 1]->x, vertex[faces[i]->faces[2] - 1]->y, vertex[faces[i]->faces[2] - 1]->z);
			glVertex3f(vertex[faces[i]->faces[3] - 1]->x, vertex[faces[i]->faces[3] - 1]->y, vertex[faces[i]->faces[3] - 1]->z);
			glEnd();
		}
		else {
			glBegin(GL_TRIANGLES);
			glNormal3f(normals[faces[i]->facenum - 1]->x, normals[faces[i]->facenum - 1]->y, normals[faces[i]->facenum - 1]->z);
			glVertex3f(vertex[faces[i]->faces[0] - 1]->x, vertex[faces[i]->faces[0] - 1]->y, vertex[faces[i]->faces[0] - 1]->z);
			glVertex3f(vertex[faces[i]->faces[1] - 1]->x, vertex[faces[i]->faces[1] - 1]->y, vertex[faces[i]->faces[1] - 1]->z);
			glVertex3f(vertex[faces[i]->faces[2] - 1]->x, vertex[faces[i]->faces[2] - 1]->y, vertex[faces[i]->faces[2] - 1]->z);
			glEnd();
		}
	}
	glEndList();

	//delete everything to avoid memory leaks
	for (int i = 0; i < coord.size(); i++)
		delete coord[i];
	for (int i = 0; i < faces.size(); i++)
		delete faces[i];
	for (int i = 0; i < normals.size(); i++)
		delete normals[i];
	for (int i = 0; i < vertex.size(); i++)
		delete vertex[i];

	return num;	//return with the id
}


void genMap(){
	int i, j;
	map.x = mapSize + 2;
	map.z = mapSize + 2;
	printf("\n\n===============================================================\n\n");

	srand((unsigned)time(NULL));

	for (j = 0; j < map.z; j++)
		for (i = 0; i < map.x; i++)
		{
			if ((i == 0) || (j == 0) || (i == map.x - 1) || (j == map.z - 1))
				zone[j][i] = 'x';
			else
				zone[j][i] = '.';
		}

	int k = 0;
	Tpos t;
	while (k < (int)mapSize*mapSize / 2.5)
	{
		t.x = rand() % mm + 1;
		t.y = rand() % mm + 1;
		if ((zone[t.y][t.x] == '.') && (isGood(t)))
		{
			zone[t.y][t.x] = 'x';
			k++;
		}
	}

	for (j = 1; j <= mapSize; j++)
		for (i = 1; i <= mapSize; i++)
			if ((zone[j][i] == 'x') && (step(j, i) > 3))
			{
				zone[j][i] = '.';
			}
	for (j = 1; j <= mapSize; j++)
		for (i = 1; i <= mapSize; i++)
			if ((zone[j][i] == '.') && (step(j, i) < 2))
			{
				t.x = i; t.y = j;
				if (isGood(t))
					zone[j][i] = 'x';
			}
	

	// - player 배치 - //
	srand((unsigned)time(NULL));

	Tpos ps[11];
	k = 0;
	while (k <= 10)
	{
		t.x = rand() % mm + 1;
		t.y = rand() % mm + 1;
		if (zone[t.y][t.x] == '.')
		{
			ps[k] = t;
			k++;
		}
	}
	k = rand() % 11;

	player.x = ps[k].x;
	player.z = ps[k].y;


	// - item1 배치 - //
	//srand((unsigned)time(NULL));
	Tpos pss[11];
	k = 0;
	while (k <= 10)
	{
		t.x = rand() % mm + 1;
		t.y = rand() % mm + 1;
		if (zone[t.y][t.x] == '.')
		{
			pss[k] = t;
			k++;
		}
	}
	k = rand() % 11;

	item1.x = pss[k].x;
	item1.z = pss[k].y;

	// - item2 배치 - //

	Tpos psss[11];
	k = 0;
	while (k <= 10)
	{
		t.x = rand() % mm + 1;
		t.y = rand() % mm + 1;
		if (zone[t.y][t.x] == '.')
		{
			psss[k] = t;
			k++;
		}
	}
	k = rand() % 11;

	item2.x = psss[k].x;
	item2.z = psss[k].y;


	


	// - wayOut 배치 - //

	zoneToCp();
	fill(pss[k].y, pss[k].x, &t, &pathLen);
	wayOut.x = t.x;
	wayOut.z = t.y;


	// - batterys 배치 - //

	k = 0;
	bool can = false;
	t.x = rand() % mm + 1;
	t.y = rand() % mm + 1;

	while (k <= batteryMax) {
		
		if (zone[t.y][t.x] == '.') {
			for (int i = 0; i < batteryMax; i++) {
				if (batterys[i].x == t.x && batterys[i].z == t.y) {
					can = false;
					break;
				}
			}
		}
		else {
			can = false;
		}

		if (t.x == wayOut.x && t.y == wayOut.z) {
			can = false;
		}

		if (can) {
			batterys[k].x = t.x;
			batterys[k].z = t.y;
			k++;
			can = false;
		}
		else {
			t.x = rand() % mm + 1;
			t.y = rand() % mm + 1;
			can = true;
		}
	}

	printf("   game starting..........  \n");
}



void init()
{
	startTime = time(&start_t);

	gameInfo.time = 0;
	gameInfo.score = 0;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	player.dx = 0;
	player.dz = 0;
	player.isGo = false;

	target.x = camera.x;
	target.y = camera.y;
	target.z = camera.z + 0.1;

	camera.gapX = 0;
	camera.gapY = 0;
	camera.gapZ = 0.01f;


	srand((unsigned)time(NULL));

	for (int i = 0; i < batteryMax;i++) {
		batterys[i].cur = rand() % 360;
		if (int k = rand() % 20 > 10)
			batterys[i].dir = 10;
		else
			batterys[i].dir = -10;
	}

	glEnable(GL_COLOR_MATERIAL);

	playerUnit = loadObj("main1.obj");
	miniUnit = loadObj("main1.obj");
}

void displayUpdate() {

	player.px = player.x + 0.5f;
	player.py = (double)player.y;
	player.pz = player.z + 0.5;
	
	if (viewChanged) {
		cur_direction = 0;
		viewChanged = false;
	}

	timeCheck();
	
	switch (viewKind) {
	case 1:
		// 1인칭

		target.x = player.px + (1.0*player.dx / speedDelay);
		target.y = player.py + 1.25;
		target.z = player.pz + (1.0*player.dz / speedDelay);;

		camera.x = player.px + camera.gapX + cos(M_PI_2 + cur_direction / double(direction_parts)*2.*M_PI);
		camera.y = player.py + 1.3 + camera.gapY+0.2f;
		camera.z = player.pz + camera.gapZ + sin(M_PI_2 + cur_direction / double(direction_parts)*2.*M_PI);

		break;
	case 2:
		// 머리위
		camera.x = player.px + (1.0*player.dx / speedDelay);
		camera.y = player.py + 10;
		camera.z = player.pz + (1.0*player.dz / speedDelay) + 0.1f;

		target.x = player.px + (1.0*player.dx / speedDelay);
		target.y = player.py;
		target.z = player.pz + (1.0*player.dz / speedDelay);

		break;
	case 3:
		// 맵 전체
		camera.x = (mapSize + 2) / 2 + 0.5;
		camera.y = 27;
		camera.z = (mapSize + 2) / 2 + 0.5 + 0.01;

		target.x = (mapSize + 2) / 2 + 0.5;
		target.y = 0;
		target.z = (mapSize + 2) / 2 + 0.5;

		break;
	case 4:
		// 테스트용
;

		break;
	default:
		// 원본 카메라
		camera.x = player.px + (1.0*player.dx / speedDelay) + 3 * cos(M_PI_2 + cur_direction / double(direction_parts)*2.*M_PI);
		camera.y = player.py + cameraDistance;
		camera.z = player.pz + (1.0*player.dz / speedDelay) + 3 * sin(M_PI_2 + cur_direction / double(direction_parts)*2.*M_PI);

		target.x = player.px + (1.0*player.dx / speedDelay);
		target.y = player.py + 0.5f;
		target.z = player.pz + (1.0*player.dz / speedDelay);
		break;
	}
}


void Move_Camera() {
	glTranslatef(player.x + 0.5f, player.y + 0.5f, player.z + 0.5f);

	if ((mapDir == 4 && playerDir == 0) || (mapDir == 1 && playerDir == 3 || (mapDir == 2 && playerDir == 2)) || (mapDir == 3 && playerDir == 1)) {
		//glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		
		cur_direction = 0;
		camera.gapX = 0;
		camera.gapZ = 0.01f;


	}
	else if ((mapDir == 4 && playerDir == 1) || (mapDir == 1 && playerDir == 0) || (mapDir == 2 && playerDir == 3) || (mapDir == 3 && playerDir == 2)) {
		//glRotatef(270.0f, 0.0f, 1.0f, 0.0f);

		cur_direction = 27;
		camera.gapX = 0.01f;
		camera.gapZ = 0;
	}
	else if ((mapDir == 4 && playerDir == 2) || (mapDir == 1 && playerDir == 1) || (mapDir == 2 && playerDir == 0) || (mapDir == 3 && playerDir == 3)) {
		//glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
		cur_direction = 18;
		camera.gapX = 0;
		camera.gapZ = -0.01f;
		
	}
	else if ((mapDir == 4 && playerDir == 3) || (mapDir == 1 && playerDir == 2) || (mapDir == 2 && playerDir == 1) || (mapDir == 3 && playerDir == 0)) {
		//glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		cur_direction = 9;
		camera.gapX = -0.01f;
		camera.gapZ = 0;
	}
}
void Draw_Player()
{
	
	glTranslatef(player.x + (1.0*player.dx / speedDelay) + 0.5f, player.y + 0.5f, player.z + (1.0*player.dz / speedDelay) + 0.5f);
	glScalef(0.15, 0.4, 0.15);
	glRotatef(-35.0f, 1.0f, 0.0f, 0.0f);

		switch (mapDir) {
		case 4: // 뒤 -> 앞
			if (playerDir == 0) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 1) glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 2) glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 3) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
			break;
		case 1: //우 -> 좌
			if (playerDir == 3) 	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 0) glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 1) glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 2) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
			break;
		case 2: //앞 -> 뒤
			if (playerDir == 2) 	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 3) glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 0) glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 1) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
			break;
		case 3: //좌 -> 우
			if (playerDir == 1) 	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 2) glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 3) glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
			else if (playerDir == 0) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
			break;
		default:
			break;
		}

}

void Draw_Items(float px, float py, float pz,unsigned texture = textureItems)
{

	glTranslatef(px,py,pz);
	glScalef(0.4, 0.75, 0.4);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glVertex3f(-1.0f, 1.0f, 1.0f); // { Front }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Front }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, -1.0f, 1.0f); // { Front }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, 1.0f, 1.0f); // { Front }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, 1.0f, 1.0f); // { Right }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, -1.0f, 1.0f); // { Right }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Right }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, 1.0f, -1.0f); // { Right }
	glTexCoord2f(0, 1);

    glVertex3f(1.0f, 1.0f, -1.0f); // { Back }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Back }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Back }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, -1.0f); // { Back }
	glTexCoord2f(0, 1);

	glVertex3f(-1.0f, 1.0f, -1.0f); // { Left }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Left }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Left }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, 1.0f); // { Left }
	glTexCoord2f(0, 1);

	glVertex3f(-1.0f, 1.0f, 1.0f); // { Top }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, 1.0f, 1.0f); // { Top }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, 1.0f, -1.0f); // { Top }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, -1.0f); // { Top }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, -1.0f, 1.0f); // { Bottom }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Bottom }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Bottom }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Bottom }
	glTexCoord2f(0, 1);
	glEnd();

}

void Draw_Batterys(int i , unsigned texture = textureBatterys)
{
	glTranslatef(batterys[i].x + 0.5f, 0.5f, batterys[i].z + 0.5f);
	glScalef(0.1, 0.2, 0.1);

	batterys[i].cur++;

	if (batterys[i].cur > 360) {
		batterys[i].cur = 0;
	}

	glRotatef(batterys[i].dir, 1.0f, 0.0f, 0.0f);
	glRotatef(batterys[i].cur, 0.0f, 1.0f, 0.0f);


	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glVertex3f(-1.0f, 1.0f, 1.0f); // { Front }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Front }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, -1.0f, 1.0f); // { Front }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, 1.0f, 1.0f); // { Front }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, 1.0f, 1.0f); // { Right }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, -1.0f, 1.0f); // { Right }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Right }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, 1.0f, -1.0f); // { Right }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, 1.0f, -1.0f); // { Back }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Back }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Back }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, -1.0f); // { Back }
	glTexCoord2f(0, 1);

	glVertex3f(-1.0f, 1.0f, -1.0f); // { Left }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Left }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Left }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, 1.0f); // { Left }
	glTexCoord2f(0, 1);

	glVertex3f(-1.0f, 1.0f, 1.0f); // { Top }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, 1.0f, 1.0f); // { Top }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, 1.0f, -1.0f); // { Top }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, -1.0f); // { Top }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, -1.0f, 1.0f); // { Bottom }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Bottom }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Bottom }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Bottom }
	glTexCoord2f(0, 1);
	glEnd();



}

void Draw_Floor(GLfloat x1, GLfloat x2, GLfloat z1, GLfloat z2, unsigned texture = textureFloor)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_POLYGON);
	glNormal3f(0.0, 1.0, 0.0);
	glTexCoord2f(0, 0);
	glVertex3f(x1, 0, z2);
	glTexCoord2f(1, 0);
	glVertex3f(x2, 0, z2);
	glTexCoord2f(1, 1);
	glVertex3f(x2, 0, z1);
	glTexCoord2f(0, 1);
	glVertex3f(x1, 0, z1);
	glEnd();
}


void Draw_Box(GLint j, GLint i, unsigned texture = textureWall)
{
	GLfloat x1 = i, x2 = i + 1, y1 = 0, y2 = 1, z1 = j, z2 = j + 1;
	glBindTexture(GL_TEXTURE_2D, texture);

	if ((j == map.z + 1) || (zone[j - 1][i] != 'x'))
	{
		glBegin(GL_POLYGON); // Back
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(0, 0);
		glVertex3f(x2, y1, z1);
		glTexCoord2f(1, 0);
		glVertex3f(x1, y1, z1);
		glTexCoord2f(1, 1);
		glVertex3f(x1, y2, z1);
		glTexCoord2f(0, 1);
		glVertex3f(x2, y2, z1);
		glEnd();
	}
	if ((j == map.z - 1) || (zone[j + 1][i] != 'x'))
	{
		glBegin(GL_POLYGON); // 
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0, 0);
		glVertex3f(x1, y1, z2);
		glTexCoord2f(1, 0);
		glVertex3f(x2, y1, z2);
		glTexCoord2f(1, 1);
		glVertex3f(x2, y2, z2);
		glTexCoord2f(0, 1);
		glVertex3f(x1, y2, z2);
		glEnd();
	}
	if ((i > 0) && (zone[j][i - 1] != 'x'))
	{
		glBegin(GL_POLYGON); // Left
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(0, 0);
		glVertex3f(x1, y1, z1);
		glTexCoord2f(1, 0);
		glVertex3f(x1, y1, z2);
		glTexCoord2f(1, 1);
		glVertex3f(x1, y2, z2);
		glTexCoord2f(0, 1);
		glVertex3f(x1, y2, z1);
		glEnd();
	}
	if ((i < map.x) && (zone[j][i + 1] != 'x'))
	{
		glBegin(GL_POLYGON); // Right
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(0, 0);
		glVertex3f(x2, y1, z2);
		glTexCoord2f(1, 0);
		glVertex3f(x2, y1, z1);
		glTexCoord2f(1, 1);
		glVertex3f(x2, y2, z1);
		glTexCoord2f(0, 1);
		glVertex3f(x2, y2, z2);
		glEnd();
	}
	glBegin(GL_POLYGON); // Top
	glNormal3f(0.0, 1.0, 0.0);
	glTexCoord2f(0, 0);
	glVertex3f(x1, y2, z2);
	glTexCoord2f(1, 0);
	glVertex3f(x2, y2, z2);
	glTexCoord2f(1, 1);
	glVertex3f(x2, y2, z1);
	glTexCoord2f(0, 1);
	glVertex3f(x1, y2, z1);
	glEnd();
}


void Draw_WayOut(unsigned texture = textureWayOut)
{

	glTranslatef(wayOut.x + 0.5f, wayOut.y + 0.5f, wayOut.z + 0.5f);

	glScalef(0.3, 0.3, 0.3);
	glBindTexture(GL_TEXTURE_2D, textureWayOut);
	glBegin(GL_QUADS);
	glVertex3f(-1.0f, 1.0f, 1.0f); // { Front }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Front }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, -1.0f, 1.0f); // { Front }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, 1.0f, 1.0f); // { Front }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, 1.0f, 1.0f); // { Right }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, -1.0f, 1.0f); // { Right }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Right }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, 1.0f, -1.0f); // { Right }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, 1.0f, -1.0f); // { Back }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Back }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Back }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, -1.0f); // { Back }
	glTexCoord2f(0, 1);

	glVertex3f(-1.0f, 1.0f, -1.0f); // { Left }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Left }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Left }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, 1.0f); // { Left }
	glTexCoord2f(0, 1);

	glVertex3f(-1.0f, 1.0f, 1.0f); // { Top }
	glTexCoord2f(0, 0);
	glVertex3f(1.0f, 1.0f, 1.0f); // { Top }
	glTexCoord2f(1, 0);
	glVertex3f(1.0f, 1.0f, -1.0f); // { Top }
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f, 1.0f, -1.0f); // { Top }
	glTexCoord2f(0, 1);

	glVertex3f(1.0f, -1.0f, 1.0f); // { Bottom }
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f); // { Bottom }
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f); // { Bottom }
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, -1.0f, -1.0f); // { Bottom }
	glTexCoord2f(0, 1);
	glEnd();

}

void reshape(int w, int h)
{
	width = w;
	height = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(35.0, (GLfloat)w / (GLfloat)h, 1.0, 60.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 25, 0, 0, 0, 0, 1, 0);
}


void animate()
{
	if (playerObjNum == 50) {
		playerObjNum = 0;
	}

	if ((player.x == item1.x) && (player.z == item1.z)){
		if (player.getItems == 0) {
			viewKind = 2;
			viewChanged = true;
		}
		if (player.getItems == 1) {
			viewKind = 3;
			viewChanged = true;
		}
		player.getItems++;
		item1.x = 0;
		item1.y = -2;
		item1.z = 0;
	}

	if ((player.x == item2.x) && (player.z == item2.z)) {
		if (player.getItems == 0) {
			viewKind = 2;
			viewChanged = true;
		}
		if (player.getItems == 1) {
			viewKind = 3;
			viewChanged = true;
		}
		player.getItems++;
		item2.x = 0;
		item2.y = -2;
		item2.z = 0;
	}

	for (int i=0; i < batteryMax; i++) {
		if ((player.x == batterys[i].x) && (player.z == batterys[i].z)) {
			
			gameInfo.score += 100;
			batterys[i].x = 0;
			batterys[i].y = -2;
			batterys[i].z = 0;
		}
	}

	if ((player.x == wayOut.x) && (player.z == wayOut.z))
	{
		halt(true);
	}

	if (player.isGo == true)
	{
		if (player.dx > 0) {
			player.dx += 0.2;
			playerUnit = loadObj(playerObj[playerObjNum]);
			playerObjNum++;
		}
		else {
			if (player.dz > 0) {
				player.dz += 0.2;
				playerUnit = loadObj(playerObj[playerObjNum]);
				playerObjNum++;

			}

			else {
				if (player.dx < 0) {
					player.dx -= 0.2;
					playerUnit = loadObj(playerObj[playerObjNum]);

					playerObjNum++;

				}
				else {
					if (player.dz < 0) {
						player.dz -= 0.2;
						playerUnit = loadObj(playerObj[playerObjNum]);

						playerObjNum++;
					}
				}
			}
		}
		if ((player.dx >= speedDelay) || (player.dz >= speedDelay))
		{
			player.isGo = false;
			if (player.dx>0)	player.x += 1;
			if (player.dz > 0)	player.z += 1;
			player.dx = 0; player.dz = 0;
		}
		else {
			if ((player.dx <= -speedDelay) || (player.dz <= -speedDelay))
			{
				player.isGo = false;
				if (player.dx < 0)	player.x -= 1;
				if (player.dz < 0)	player.z -= 1;
				player.dx = 0; player.dz = 0;
			}
		}
	}

	glutPostRedisplay();
}


void key(unsigned char key, int x, int y)
{

	if (key == 'q' || key == 'Q' || key == 27)
		halt(false);

	if (key == 0x31) {
		viewChanged = true;
		viewKind =1;
	}

	if (player.getItems >= 1) {
		if (key == 0x32) {
			viewChanged = true;
			viewKind = 2;
		}
		if (player.getItems >= 2) {
			if (key == 0x33) {
				viewChanged = true;
				viewKind = 3;
			}
		}
	}
	
	if (key == 0x30) {
		viewChanged = true;
		viewKind = 3;
	}
}


bool good_move(int z, int x) {
	return (0 <= z && 0 <= x && z < map.z && x < map.x && zone[z][x] != 'x');
}


void keys(int key, int x, int y)
{
	if (player.isGo) return;
	mapDir = int(((direction_parts - cur_direction - 1) / double(direction_parts))*4. + 0.5);
	
	if (viewKind == 1) {
		
		for (int i = 0; i < 4; i++) {
			if (key == move_key[i]) {

				playerDir = i;
				if (key == GLUT_KEY_UP) {
					int newz = player.z + move[(mapDir + i) % 4][0];
					int newx = player.x + move[(mapDir + i) % 4][1];

					if (good_move(newz, newx)) {
						player.isGo = true;
						player.dz += move[(mapDir + i) % 4][0];
						player.dx += move[(mapDir + i) % 4][1];
					}
				}
				
				if (key == GLUT_KEY_LEFT) {
					player.turnDir = -1;
						
				}
				else if (key == GLUT_KEY_RIGHT) {
					player.turnDir = 1;

				}
				
			}

		}
	}
	else {
		for (int i = 0; i < 4; i++) {
			if (key == move_key[i]) {
				int newz = player.z + move[(mapDir + i) % 4][0];
				int newx = player.x + move[(mapDir + i) % 4][1];
				playerDir = i;
				if (good_move(newz, newx)) {
					playerDir = i;
					player.isGo = true;
					player.dz += move[(mapDir + i) % 4][0];
					player.dx += move[(mapDir + i) % 4][1];


				}

			}
		}
	}
	
}



void display()
{

	displayUpdate();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(camera.x,camera.y,camera.z,
			target.x,target.y,target.z,
			0,1,0);

	for (int i = 0; i < map.x; i++) {
		for (int j = 0; j < map.z; j++) {
			if (zone[j][i] == 'x'){
				Draw_Box(j, i);
			}else {
				Draw_Floor(i, i + 1, j, j + 1);
			}
		}
	}
	// - player Matrix - //

	glPushMatrix();
	
	if (viewKind == 1) {
		Move_Camera();
	}
	else {
		Draw_Player();
		
		glCallList(playerUnit);

	}
		
	glPopMatrix();

	// - Item Matrixs - //


	glPushMatrix();
	Draw_Items(item1.x + 0.5f, item1.y + 0.7, item1.z + 0.5f);
	glPopMatrix();

	glPushMatrix();
	Draw_Items(item2.x + 0.5f, item2.y + 0.7, item2.z + 0.5f);
	glPopMatrix();

	for(int i=0;i<batteryMax;i++){
		glPushMatrix();
		Draw_Batterys(i);
		glPopMatrix();
	}


	// - wayOut Matrix - //

	glPushMatrix();

	Draw_WayOut();

	glPopMatrix();

	glutSwapBuffers();
}


int step(int y, int x)
{
	int res = 0;
	if ((zone[y][x] == 'x')) res++;
	if ((y<mm) && (zone[y + 1][x] == 'x')) res++;
	if ((y>0) && (zone[y - 1][x] == 'x')) res++;
	if ((x<mm) && (zone[y][x + 1] == 'x')) res++;
	if ((x>0) && (zone[y][x - 1] == 'x')) res++;
	return res;
}


void zoneToCp(void)
{
	for (int j = 1; j <= mm; j++)
	for (int i = 1; i <= mm; i++)
		cp[j][i] = zone[j][i];
}


void fill(int y, int x, Tpos *v, int *l)
{
	int st = 1, en = 0;
	int G[mm*mm], Ll[mm*mm];
	G[0] = y*mm + x;
	Ll[0] = 0;
	cp[y][x] = 'x';
	while (st != en)
	{
		if ((G[en] % mm + 1<mm) && (cp[(int)G[en] / mm][G[en] % mm + 1] == '.'))
		{
			G[st] = G[en] + 1;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		if ((G[en] % mm - 1 > 0) && (cp[(int)G[en] / mm][G[en] % mm - 1] == '.'))
		{
			G[st] = G[en] - 1;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		if ((((int)G[en] / mm) + 1<mm) && (cp[((int)G[en] / mm) + 1][G[en] % mm] == '.'))
		{
			G[st] = G[en] + mm;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		if ((((int)G[en] / mm) - 1 > 0) && (cp[((int)G[en] / mm) - 1][G[en] % mm] == '.'))
		{
			G[st] = G[en] - mm;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		en++;
	}
	int rnd = rand() % 5 + 1;
	(*v).x = (int)G[en - rnd] % mm;
	(*v).y = (int)G[en - rnd] / mm;
	*l = Ll[en - rnd];
}


bool isGood(Tpos a)
{
	zoneToCp();
	int k = 0, i, j, lt;
	Tpos temp;
	cp[a.y][a.x] = 'x';
	if ((step(a.y, a.x) > 3) || (step(a.y + 1, a.x) > 3) || (step(a.y - 1, a.x) > 3) || (step(a.y, a.x + 1) > 3) || (step(a.y, a.x - 1) > 3))
		return false;
	for (j = 1; j <= mapSize; j++)
	for (i = 1; i <= mapSize; i++)
	if (cp[j][i] == '.')
	{
		if (k > 0)
			return false;
		fill(j, i, &temp, &lt);
		k++;
	}
	return true;
}


void start()
{
	int su;
	//┏┓━┃┗┛
	printf("                                                                               \n");
	printf("                                                       제 작 자 : 임동빈       \n");
	printf("                                                                  (팀원)       \n");
	printf("                                                                  (팀원)       \n");
	printf("                                                                               \n");
	printf("    ┏━━━━━━━━┓       ┏━┓    ┏━━━┓ ┏━━━━━┓             \n");
	printf("    ┃  ┏━┓┏━┓  ┃      ┃    ┃   ┗━━┓┃ ┃  ┏━━━┛             \n");
	printf("    ┃  ┃  ┃┃  ┃  ┃     ┃ ┏┓ ┃       ┃ ┃ ┃  ┗━━━┓             \n");
	printf("    ┃  ┃  ┃┃  ┃  ┃   ┃  ┃  ┃  ┃    ┃ ┃  ┃  ┏━━━┛             \n");
	printf("    ┃  ┃  ┃┃  ┃  ┃  ┃  ┃    ┃  ┃  ┃ ┃   ┃  ┗━━━┓             \n");
	printf("    ┃  ┃  ┃┃  ┃  ┃  ┗━┛    ┗━┛┃ ┃     ┗━━━━━┛             \n");
	printf("    ┗━┛  ┗┛  ┗━┛                 ┃  ┗━━━━━━━━━━┓          \n");
	printf("                                         ┗━━━━━━━━━━━━┛          \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("        시작 하려면 아무키나누르세요....");
	su = getch();
}


void finish()
{
	int su;
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("                                                                               \n");
	printf("         ┏━━━━┓          ┏━┓    ┏┓        ┏━━━┓                \n");
	printf("         ┃┏━━━┛         ┃┏┓┃   ┃┃        ┃┏━┓ ┃               \n");
	printf("         ┃┗━━━┓         ┃┃ ┃┃  ┃┃        ┃┃   ┃┃               \n");
	printf("         ┃┏━━━┛         ┃┃  ┃┃ ┃┃        ┃┃   ┃┃               \n");
	printf("         ┃┗━━━┓         ┃┃   ┃┗┛┃        ┃┗━┛ ┃               \n");
	printf("         ┗━━━━┛         ┗┛    ┗━┛         ┗━━━┛                \n");
	printf("                                                                               \n");
	printf("===================================================================\n\n\n");

	printf("                         [   %d   :    sec ]  \n\n", gameInfo.time);

	printf("                         [   %d   :    score ]  \n\n", gameInfo.score);

	printf("===================================================================\n");
	printf("                                                                               \n");
}


void halt(bool gameOver)
{
	glutDestroyWindow(glWin);
	

	if (gameOver == true)
	{
		finish();
	}
	for(int i=0; i < 10; i++) _getch();
		
	exit(0);
} 

void logCat_int(std::string str,int i) {
	std::cout << str << std::to_string(i) << std::endl;
		
}
void logCat_float(std::string str,float f) {
	std::cout << str << std::to_string(f) << std::endl;
}
void logCat_bool(std::string str, bool b) {
	std::cout << str << std::to_string(b) << std::endl;
}
void timeCheck() {
	currentTime = time(&current_t) - startTime;
	gameInfo.time = currentTime;
	//printf("[ %d  sec ]", currentTime);
}


