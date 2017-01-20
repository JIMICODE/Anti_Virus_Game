#pragma once
//header file
#define WIN32_EXTRA_LEAN
#define DIRECTINPUT_VERSION 0x0800

#include<Windows.h>
#include<d3d9.h>
#include<d3dx9.h>
#include<dinput.h>
#include<Xinput.h>
#include<ctime>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<io.h>
#include<algorithm>
#include "DirectSound.h"
//Namespcae
using namespace std;
//Libraries
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxerr.lib")
//Program values
extern const string APPTITLE;
extern const int SCREENW;
extern const int SCREENH;
extern bool gameover;
//Transformation math pram
extern const double PI;
extern const double PI_under_180;
extern const double PI_over_180;
//Sprite structure
struct SPRITE
{
	float x, y;
	int frame, columns;
	int width, height;
	float scaling, rotation;
	int startframe, endframe;
	int starttime, delay;
	int direction;
	float velx, vely;
	bool alive;
	D3DCOLOR color;
	//construct
	SPRITE()
	{
		frame = 0;
		columns = 1;
		width = height = 0;
		scaling = 1.0f;
		rotation = 0.0f;
		startframe = endframe = 0;
		direction = 1;
		starttime = delay = 0;
		velx = vely = 0.0f;
		alive = true;
		color = D3DCOLOR_XRGB(25, 255, 255);
	}
};
//Define the MODEL struct
struct MODEL
{
	LPD3DXMESH mesh;
	D3DMATERIAL9* materials;
	LPDIRECT3DTEXTURE9* textures;
	DWORD material_count;
	D3DXVECTOR3 translate;
	D3DXVECTOR3 rotate;
	D3DXVECTOR3 scale;

	MODEL()
	{
		material_count = 0;
		mesh = NULL;
		materials = NULL;
		textures = NULL;
		translate = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		rotate = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	}
};
//Direct3D objects
extern LPDIRECT3D9 d3d;
extern LPDIRECT3DDEVICE9 d3ddev;
extern LPDIRECT3DSURFACE9 backbuffer;
extern LPD3DXSPRITE spriteobj;
//Primary DirectSound object
extern CSoundManager *dsound;
//DIrect3D functions
bool Direct3D_Init(HWND hwnd, int width, int height, bool fullscreen);
void Direct3D_ShutDown();
LPDIRECT3DSURFACE9 LoadSurface(string filename);
void DrawSurface(LPDIRECT3DSURFACE9 dest, float x, float y, LPDIRECT3DSURFACE9 source);
//DirectInput object, devices, and states
extern LPDIRECTINPUT8 input;
extern LPDIRECTINPUTDEVICE8 dimouse;
extern LPDIRECTINPUTDEVICE8 dikeyboard;
extern DIMOUSESTATE mouse_state;
extern XINPUT_GAMEPAD controllers[4]; 
//DirectInput functions
bool DirectInput_Init(HWND);
void DirectInput_Update();
void DirectInput_Shutdown();
int Key_Down(int);
int Mouse_Button(int);
int Mouse_X();
int Mouse_Y();
void XInput_Vibrate(int contNum = 0, int amount = 65535);
bool XInput_Controller_Found();
//Direct3D texture and sprite
D3DXVECTOR2 GetBitmapSize(string filename);
LPDIRECT3DTEXTURE9 LoadTexture(string filename, D3DCOLOR transcolor = D3DCOLOR_XRGB(0,0,0));
//Game function
bool Game_Init(HWND window);
void Game_Run(HWND window);
void Game_End();
//Sprite ainimation function
void Sprite_Draw_Frame(LPDIRECT3DTEXTURE9 texture = NULL, int destx = 0, int desty = 0,
	 int framew = 64, int frameh = 64, int framenum = 0, int colums = 1);
void Sprite_Animate(int &frame, int startframe, int endframe,
	int direction, int &starttime, int delay);
//Sprite transform
void Sprite_Transfrom_Draw(LPDIRECT3DTEXTURE9 image, int x, int y, int width, int height, int frame = 0,
	int columns = 1, float rotation = 0.0f, float scaling = 1.0f, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255));
//To radians
double toRadians(double degrees);
//To degrees
double toDegrees(double radians);
//Bounding box collision detection
int Collision(SPRITE sprite1, SPRITE sprite2);
//Test collision by distance
bool CollisionD(SPRITE sprite1, SPRITE sprite2);
//Font fuctions
LPD3DXFONT MakeFont(string name, int size);
void FontPrint(LPD3DXFONT font, string text, int x = 0, int y = 0, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255));
//functions prototypes
bool DirectSound_Init(HWND hwnd);
void DirectSound_Shutdown();
CSound* LoadSound(string filename);
void MPlaySound(CSound *sound);
void MLoopSound(CSound *sound);
void MStopSound(CSound *sound);
//3D mesh functions prototypes
void DrawModel(MODEL *model);
void DeleteModel(MODEL* model);
MODEL *LoadModel(string filename);
bool FindFile(string* filename);
bool DoesFileExist(const string& filename);
void SplitPath(const string& inputPath, string* pathOnly, string* filenameOnly);
void SetCamera(float posx, float posy, float posz,
	float lookx = 0.0f, float looky = 0.0f, float lookz = 0.0f);