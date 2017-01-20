//Begining Game Programing
//Anti_Virus Game
#include "MyDirectX.h"

const string APPTITLE = "Anti_Virus Game";
const string ERRSTR = "Fuck!!!";
const int SCREENW = 1024;
const int SCREENH = 768;
const bool FULLSCREEN = true;
#define STR_ERR ERRSTR.c_str()

//Game state variables
enum GAME_STATES
{
	BRIEFING = 0,
	PLAYING = 1
};

GAME_STATES game_state = BRIEFING;
//font variables
LPD3DXFONT font;
LPD3DXFONT hugefont;
LPD3DXFONT debugfont;
//timing variables
DWORD refresh = 0;
DWORD screentime = 0;
double screencount = 0.0;
double screenfps = 0.0;
DWORD coretime = 0;
double corefps = 0.0;
double corecount = 0.0;
DWORD currenttime;
//background scrolling variables
const int BUFFERW = SCREENW * 2;
const int BUFFERH = SCREENH;
LPDIRECT3DSURFACE9 background = NULL;
double scrollx = 0;
double scrolly = 0;
const double virtual_level_size = BUFFERW * 5;
double virtual_scrollx = 0;
//player variables
LPDIRECT3DTEXTURE9 player_ship;
SPRITE player;
enum PLAYER_STATES
{
	NORMAL = 0,
	PHASING = 1,
	OVERLOADING = 2
};
PLAYER_STATES player_state = NORMAL;
PLAYER_STATES player_state_previous = NORMAL;
D3DXVECTOR2 position_hisrtory[8];
int position_history_index = 0;
DWORD position_history_timer = 0;
double charge_angle = 0.0;
double charge_tweak = 0.0;
double charge_tweak_dir = 1.0;
int energy = 100;
int health = 100;
int lives = 3;
int score = 0;
//enemy virus objects
const int VIRUSES = 200;
LPDIRECT3DTEXTURE9 virus_image;
SPRITE viruses[VIRUSES];
const int FRAGMENTS = 300;
LPDIRECT3DTEXTURE9 fragment_image;
SPRITE fragments[FRAGMENTS];
//bullet variables
LPDIRECT3DTEXTURE9 purple_fire;
const int BULLETS = 300;
SPRITE bullets[BULLETS];
int player_shoot_timer = 0;
int bulletcount = 0;
//sound effects
CSound* snd_tisk = NULL;
CSound* snd_foom = NULL;
CSound* snd_charging = NULL;
CSound* snd_killed = NULL;
CSound* snd_hit = NULL;
//GUI elements
LPDIRECT3DTEXTURE9 energy_slice;
LPDIRECT3DTEXTURE9 health_slice;
//controller vibration
int vibrating = 0;
int vibration = 100;

//allow quick string conversion anywhere in the program
template <class T>
std::string static ToString(const T & t, int places = 2)
{
	ostringstream oss;
	oss.precision(places);
	oss.setf(ios_base::fixed);
	oss << t;
	return oss.str();
}

bool Create_Viruses()
{
	virus_image = LoadTexture("virus.tga");
	if (!virus_image)	return false;

	for (int n = 0; n < VIRUSES; ++n)
	{
		D3DCOLOR color = D3DCOLOR_ARGB(
			170 + rand() % 80,
			150 + rand() % 100,
			25 + rand() % 50,
			25 + rand() % 50
		);
		viruses[n].color = color;
		viruses[n].scaling = (float)((rand() % 25 + 50) / 100.0f);
		viruses[n].alive = true;
		viruses[n].width = 96;
		viruses[n].height = 96;
		viruses[n].x = (float)(1000 + rand() % BUFFERW);
		viruses[n].y = (float)(rand() % SCREENH);
		viruses[n].velx = (float)((rand() % 8) * -1);
		viruses[n].vely = (float)(rand() % 2 - 1);
	}
	return true;
}

bool Create_Fragments()
{
	fragment_image = LoadTexture("fragment.tga");
	if (!fragment_image)	return false;

	for (int n = 0; n < FRAGMENTS; ++n)
	{
		fragments[n].alive = true;
		D3DCOLOR fragmentcolor = D3DCOLOR_ARGB(
			125 + rand() % 50,
			150 + rand() % 100,
			150 + rand() % 100,
			150 + rand() % 100
		);
		fragments[n].color = fragmentcolor;
		fragments[n].width = 128;
		fragments[n].height = 128;
		fragments[n].scaling = (float)(rand() % 4 - 1)* -1.0f;
		fragments[n].rotation = (float)(rand() % 360);
		fragments[n].velx = (float)(rand() % 4 + 1) * -1.0f;
		fragments[n].vely = (float)(rand() % 10 - 5) / 10.0f;
		fragments[n].x = (float)(rand() % BUFFERW);
		fragments[n].y = (float)(rand() % SCREENH);
	}
	return true;
}

bool Create_Background()
{
	//load background
	LPDIRECT3DSURFACE9 image = NULL;
	image = LoadSurface("binary.png");
	if (!image)	return false;
	HRESULT result = d3ddev->CreateOffscreenPlainSurface(
		BUFFERW,
		BUFFERH,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&background,
		NULL
	);
	if (result != D3D_OK)	return false;
	//copy image to upper left corner of background
	RECT source_rect = { 0, 0, SCREENW, SCREENH };
	RECT dest_ul = { 0, 0, SCREENW, SCREENH };
	d3ddev->StretchRect(
		image,
		&source_rect,
		background,
		&dest_ul,
		D3DTEXF_NONE
	);
	//copy image to upper right of background
	RECT dest_ur = { SCREENW, 0, SCREENW * 2, SCREENH };
	d3ddev->StretchRect(
		image,
		&source_rect,
		background,
		&dest_ur,
		D3DTEXF_NONE);
	//get pointer to the back buffer
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	//remove image
	image->Release();
	return true;
}

bool Game_Init(HWND window)
{
	srand(time(NULL));
	//initialize Direct3D
	if (!Direct3D_Init(window, SCREENW, SCREENH, false))
	{
		MessageBox(0, "Error initializing Direct3D", "Error", NULL);
		return false;
	}
	//initialize DirectInput
	if (!DirectInput_Init(window))
	{
		MessageBox(0, "Error initializing DirectInput", "Error", NULL);
		return false;
	}
	//initialize DirectSound
	if (!DirectSound_Init(window))
	{
		MessageBox(window, "Error initializing DirectSound", ERRSTR.c_str(), NULL);
		return false;
	}
	
	//craete a font
	font = MakeFont("Arial Bold", 24);
	debugfont = MakeFont("Arial", 14);
	hugefont = MakeFont("Arial Bold", 80);

	//load player sprite
	player_ship = LoadTexture("ship.png");
	player.x = 100;
	player.y = 350;

	player.width = player.height = 64;
	for (int n = 0; n < 4; ++n)
		position_hisrtory[n] = D3DXVECTOR2(-100, 0);
	//load bullets
	purple_fire = LoadTexture("purplefire.tga");
	for (int n = 0; n < BULLETS; ++n)
	{
		bullets[n].alive = false;
		bullets[n].x = 0;
		bullets[n].y = 0;
		bullets[n].width = 55;
		bullets[n].height = 16;
	}
	//create enemy viruses
	if (!Create_Viruses())	return false;
	//load gui elements
	energy_slice = LoadTexture("energyslice.tga");
	health_slice = LoadTexture("healthslice.tga");
	//laod audio files
	snd_tisk = LoadSound("clip.wav");
	snd_charging = LoadSound("longfoom.wav");
	snd_killed = LoadSound("killed.wav");
	snd_hit = LoadSound("hit.wav");

	//create memory fragments (energy)
	if (!Create_Fragments())	return false;
	//create background
	if (!Create_Background())	return false;


	return true;
}

void Game_Run(HWND window)
{
	//make sure the Direct3D device is vaild
	if (!d3ddev)	return;
	//update input devices
	DirectInput_Update();
	//clear the backbuffer
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);

	//start rendering
	if (d3ddev->BeginScene())
	{
		d3ddev->EndScene();
		d3ddev->Present(NULL, NULL, NULL, NULL);
	}

	//escape key exits
	if (Key_Down(DIK_SPACE) || Key_Down(DIK_ESCAPE))
		gameover = true;
	//controller back button also exits
	if (controllers[0].wButtons & XINPUT_GAMEPAD_BACK)
		gameover = true;
}

void Game_End()
{
	if (!background)
	{
		background->Release();
		background = NULL;
	}
	if (font)
	{
		font->Release();
		font = NULL;
	}
	if (debugfont)
	{
		debugfont->Release();
		debugfont = NULL;
	}
	if (hugefont)
	{
		hugefont->Release();
		hugefont = NULL;
	}
	if (fragment_image)
	{
		fragment_image->Release();
		fragment_image = NULL;
	}
	if (player_ship)
	{
		player_ship->Release();
		player_ship = NULL;
	}
	if (virus_image)
	{
		virus_image->Release();
		virus_image = NULL;
	}
	if (purple_fire)
	{
		purple_fire->Release();
		purple_fire = NULL;
	}
	if (health_slice)
	{
		health_slice->Release();
		health_slice = NULL;
	}
	if (energy_slice)
	{
		energy_slice->Release();
		energy_slice = NULL;
	}
	if (snd_charging)	delete snd_charging;
	if (snd_foom)	delete snd_foom;
	if (snd_tisk)	delete snd_tisk;
	if (snd_killed)	delete snd_killed;
	if (snd_hit)		delete snd_hit;
	//Direct component
	DirectSound_Shutdown();
	DirectInput_Shutdown();
	Direct3D_ShutDown();
}

void move_player(float movex, float movey)
{
	//cannot move while overloading
	if (player_state == OVERLOADING
		|| player_state_previous == OVERLOADING
		|| player_state == PHASING
		|| player_state_previous == PHASING)
		return;

	float multi = 4.0f;
	player.x += movex * multi;
	player.y += movey * multi;

	if (player.x < 0)
		player.x = 0;
	else if (player.x > 300.0f)
		player.x = 300.0;

	if (player.y < 0)
		player.y = 0;
	else if (player.y > SCREENH - (player.height * player.scaling))
		player.y = SCREENH - (player.height * player.scaling);
}

double warp(double value, double bounds)
{
	double result = fmod(value, bounds);
	if (result < 0)	result += bounds;
	return result;
}
double warpAngleDegs(double degs)
{
	return warp(degs, 360.0);
}
double LinearVelocityX(double angle)
{
	if (angle < 0)	angle = 360 + angle;
	return cos(angle*PI_over_180);
}
double LinearVelocityY(double angle)
{
	if (angle < 0)	angle = 360 + angle;
	return sin(angle*PI_over_180);
}
void add_energy(double value)
{
	energy += value;
	if (energy < 0.0)		energy = 0.0;
	if (energy > 100.0)	energy = 100.0;
}
void Vibrate(int contnum, int amount, int lenght)
{
	vibrating = 1;
	vibration = lenght;
	XInput_Vibrate(contnum, amount);
}
int find_bullet()
{
	int bullet = -1;
	for (int n = 0; n < BULLETS; ++n)
	{
		if (!bullets[n].alive)
		{
			bullet = n;
			break;
		}
	}
	return bullet;
}

bool player_overload()
{
	//disallow overload unless energy is at 100%
	if (energy < 50.0)		return false;
	//reduce energy for this shot
	add_energy(-0.5);
	//play charging sound
	MPlaySound(snd_charging);
	//vibrate controller
	Vibrate(0, 20000, 20);
	int b1 = find_bullet();
	if (b1 == -1)	return true;
	bullets[b1].alive = true;
	bullets[b1].velx = 0.0f;
	bullets[b1].vely = 0.0f;
	bullets[b1].rotation = (float)(rand() % 360);
	bullets[b1].x = player.x + player.width;
	bullets[b1].y = player.y + player.height / 2 - bullets[b1].height / 2;
	bullets[b1].y += (float)(rand() % 20 - 10);

	return true;
}

void player_shoot()
{

}