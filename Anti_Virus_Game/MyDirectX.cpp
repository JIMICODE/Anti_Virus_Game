#include "MyDirectX.h"
#include "DirectSound.h"
#include<iostream>

using namespace std;
//Direct3D variables
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
LPDIRECT3DSURFACE9 backbuffer = NULL;
LPD3DXSPRITE spriteobj = NULL;

//DirectInput variables
LPDIRECTINPUT8 dinput = NULL;
LPDIRECTINPUTDEVICE8 dimouse = NULL;
LPDIRECTINPUTDEVICE8 dikeyboard = NULL;
DIMOUSESTATE mouse_state;
char keys[256];
XINPUT_GAMEPAD controllers[4];

//Transformation math pram
const double PI = 3.1415926535;
const double PI_under_180 = 180.0f / PI;
const double PI_over_180 = PI / 180.0f;

//Primary DirectSound object
CSoundManager *dsound = NULL;

//Direct3D initialization
bool Direct3D_Init(HWND window, int width, int height, bool fullscreen)
{
	//initialize Direct3D
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d) { MessageBox(window, "d3d create faill", "fuck!!!", NULL); return false; }

	//set Direct3D presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	//create displayparam
	D3DDISPLAYMODE dm;
	d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);
	//init d3dpp
	d3dpp.Windowed = FALSE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = dm.Format;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = dm.Width;
	d3dpp.BackBufferHeight = dm.Height;
	d3dpp.hDeviceWindow = window;
	d3dpp.EnableAutoDepthStencil = 1;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	//create Direct3D device
	d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);
	if (!d3ddev) { MessageBox(window, "create d3d device fail", "fuck!!!", NULL); return false; }

	//get a point to the buffer surface
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

	//create sprite object
	D3DXCreateSprite(d3ddev, &spriteobj);

	return true;
}

//Direct3D shutdown
void Direct3D_ShutDown()
{
	if (d3ddev) d3ddev->Release();
	if (d3d) d3d->Release();
	if (spriteobj) spriteobj->Release();
}

//Draw a surface to the screen using StretchRect
void DrawSurface(LPDIRECT3DSURFACE9 dest, float x, float y, LPDIRECT3DSURFACE9 source)
{
	//get width/height from souce surface
	D3DSURFACE_DESC desc;
	source->GetDesc(&desc);
	//create rects for drawing
	RECT source_rect = { 0, 0,
		(long)desc.Width, (long)desc.Height };
	RECT dest_rect = { (long)x, (long)y,
		(long)x + desc.Width, (long)y + desc.Height };
	//draw the source surface onto the dest
	d3ddev->StretchRect(source, &source_rect, dest, &dest_rect, D3DTEXF_NONE);
}
//Loads a bitmap file into a surface
LPDIRECT3DSURFACE9 LoadSurface(string filename)
{
	LPDIRECT3DSURFACE9 image = NULL;

	//get width and height from bitmap file
	D3DXIMAGE_INFO info;
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result != D3D_OK) return NULL;
	//create surface
	result = d3ddev->CreateOffscreenPlainSurface(
		info.Width,
		info.Height,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&image,
		NULL);
	if (result != D3D_OK) return NULL;
	//load surface from file into newly created surface
	result = D3DXLoadSurfaceFromFile(
		image,
		NULL,
		NULL,
		filename.c_str(),
		NULL,
		D3DX_DEFAULT,
		D3DCOLOR_XRGB(0, 0, 0),
		NULL);
	//mack sure file was loaded okay
	if (result != D3D_OK) return NULL;
	return image;
}

//DirectInput initialization
bool DirectInput_Init(HWND hwnd)
{
	//initialize DirectInput object
	HRESULT result = DirectInput8Create(
		GetModuleHandle(NULL),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&dinput,
		NULL);

	//initialize the keyboard
	dinput->CreateDevice(GUID_SysKeyboard, &dikeyboard, NULL);
	dikeyboard->SetDataFormat(&c_dfDIKeyboard);
	dikeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	dikeyboard->Acquire();
	//initialize the mouse
	dinput->CreateDevice(GUID_SysMouse, &dimouse, NULL);
	dimouse->SetDataFormat(&c_dfDIMouse);
	dimouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	dimouse->Acquire();
	d3ddev->ShowCursor(false);
	return true;
}

//DirectInput update
void DirectInput_Update()
{
	//update mouse
	dimouse->GetDeviceState(sizeof(mouse_state), (LPVOID)&mouse_state);
	//update keyboard
	dikeyboard->GetDeviceState(sizeof(keys), (LPVOID)&keys);
	//update controllers
	for (int i = 0; i < 4; ++i)
	{
		ZeroMemory(&controllers[i], sizeof(XINPUT_STATE));
		//get the state of the controller
		XINPUT_STATE state;
		DWORD result = XInputGetState(i, &state);
		//store state in global controllers array
		if (result == 0) controllers[i] = state.Gamepad;
	}
}

//return mouse x movement
int Mouse_X()
{
	return mouse_state.lX;
}
//return mouse y movement
int Mouse_Y()
{
	return mouse_state.lY;
}
//return mouse button state
int Mouse_Button(int button)
{
	return mouse_state.rgbButtons[button] & 0x80;
}
//return key press state
int Key_Down(int key)
{
	return (keys[key] & 0x80);
}

//DirectInput shutdown
void DirectInput_Shutdown()
{
	if (dikeyboard)
	{
		dikeyboard->Unacquire();
		dikeyboard->Release();
		dikeyboard = NULL;
	}
	if (dimouse)
	{
		dimouse->Unacquire();
		dimouse->Release();
		dimouse = NULL;
	}
}

//returns true if controller is plugged in
bool XInput_Controller_Found()
{
	XINPUT_CAPABILITIES caps;
	ZeroMemory(&caps, sizeof(XINPUT_CAPABILITIES));
	XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps);
	if (caps.Type != 0) return false;

	return true;
}

//Vibrates the controller
void XInput_Vibrate(int contNum, int amount)
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = amount;
	vibration.wRightMotorSpeed = amount;
	XInputSetState(contNum, &vibration);
}

//get bitmap info
D3DXVECTOR2 GetBitmapSize(string filename)
{
	D3DXIMAGE_INFO info;
	D3DXVECTOR2 size = D3DXVECTOR2(0.0f, 0.0f);

	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);

	if (result == D3D_OK)
		size = D3DXVECTOR2((float)info.Width, (float)info.Height);
	else
		size = D3DXVECTOR2((float)info.Width, (float)info.Height);

	return size;
}
//loading image file onto d3dtexture
LPDIRECT3DTEXTURE9 LoadTexture(string filename, D3DCOLOR transcolor)
{
	LPDIRECT3DTEXTURE9 texture = NULL;
	//get width and height from bitmap file
	D3DXIMAGE_INFO info;
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result != D3D_OK)	return NULL;

	//create the new texture by loading a bitmao image file
	D3DXCreateTextureFromFileEx(
		d3ddev,
		filename.c_str(),
		info.Width,
		info.Height,
		1,
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		transcolor,
		&info,
		NULL,
		&texture);
	//make sure the bitmap texture wsa loaded correctly
	if (result != D3D_OK)	return NULL;
	return texture;
}

//sprite draw frame
void Sprite_Draw_Frame(LPDIRECT3DTEXTURE9 texture, int destx, int desty,
	 int framew, int frameh, int framenum, int colums)
{
	if (!texture)	return;

	D3DXVECTOR3 position((float)destx, (float)desty, 0);
	D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	RECT rect;

	rect.left = (framenum % colums) * framew;
	rect.top = (framenum / colums) * frameh;
	rect.bottom = rect.top + frameh;
	rect.right = rect.left + framew;

	spriteobj->Draw(texture, &rect, NULL, &position, white);
}
//Control frame delay time
void Sprite_Animate(int &frame, int startframe, int endframe,
	int direction, int &starttime, int delay)
{
	if ((int)GetTickCount() > starttime + delay)
	{
		starttime = GetTickCount();
		frame += direction;
		if (frame > endframe)	frame = startframe;
		if (frame < startframe) frame = endframe;
	}
}
//Sprite transform and draw
void Sprite_Transfrom_Draw(LPDIRECT3DTEXTURE9 image, int x, int y, int width, int height,
	int frame, int columns, float rotation, float scaling, D3DCOLOR color)
{
	//create a scale vector
	D3DXVECTOR2 scale(scaling, scaling);
	//create a translate vector
	D3DXVECTOR2 trans(x, y);
	//set center by dividing width and height by two
	D3DXVECTOR2 center((float)(width * scaling) / 2, (float)(height * scaling) / 2);
	//create 2D transformation martix
	D3DXMATRIX mat;
	D3DXMatrixTransformation2D(&mat, NULL, 0, &scale, &center, rotation, &trans);

	//tell sprite object to use the transform
	spriteobj->SetTransform(&mat);
	//calculate frame location in source image
	int fx = (frame % columns) * width;
	int fy = (frame / columns) * height;
	RECT srcRect = { fx, fy, fx + width, fy + height };
	//draw the sprite frame
	spriteobj->Draw(image, &srcRect, NULL, NULL, color);
}
//To radians
double toRadians(double degrees)
{
	return degrees * PI_over_180;
}
//To degrees
double toDegrees(double radians)
{
	return radians * PI_under_180;
}

int Collision(SPRITE sprite1, SPRITE sprite2)
{
	RECT rect1;
	rect1.left = (long)sprite1.x;
	rect1.top = (long)sprite1.y;
	rect1.right = (long)sprite1.x + sprite1.width * sprite1.scaling;
	rect1.bottom = (long)sprite1.y + sprite1.height * sprite1.scaling;

	RECT rect2;
	rect2.left = (long)sprite2.x;
	rect2.top = (long)sprite2.y;
	rect2.right = (long)sprite2.x + sprite2.width * sprite2.scaling;
	rect2.bottom = (long)sprite2.y + sprite2.height * sprite2.scaling;

	RECT dest;//ignored
	return IntersectRect(&dest, &rect1, &rect2);
}
//Test collision by distance
bool CollisionD(SPRITE sprite1, SPRITE sprite2)
{
	double radius1, radius2;
	//calculate radius1
	if (sprite1.width > sprite1.height)
		radius1 = (sprite1.width * sprite1.scaling) / 2.0f;
	else
		radius1 = (sprite1.height * sprite1.scaling) / 2.0f;
	//center point 1
	double x1 = sprite1.x + radius1;
	double y1 = sprite1.y + radius1;

	//calculate radius 2
	if (sprite2.width > sprite2.height)
		radius2 = (sprite2.width * sprite2.scaling) / 2.0f;
	else
		radius2 = (sprite2.height * sprite2.scaling) / 2.0f;

	//center point 2
	double x2 = radius2 + sprite2.x;
	double y2 = radius2 + sprite2.y;

	//calculate distance
	double deltax = x1 - x2;
	double deltay = y1 - y2;
	double dist = sqrt((deltax*deltax) + (deltay*deltay));
	//return distance comparison
	return (dist < radius1 + radius2);
}
//Make a font instance
LPD3DXFONT MakeFont(string name, int size)
{
	LPD3DXFONT font = NULL;
	D3DXFONT_DESC desc = {
		size,
		0,
		0,
		0,
		false,
		DEFAULT_CHARSET,
		OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_PITCH,
		""};

	strcpy(desc.FaceName, name.c_str());
	D3DXCreateFontIndirect(d3ddev, &desc, &font);

	return font;
}

void FontPrint(LPD3DXFONT font, string text, int x, int y, D3DCOLOR color)
{
	//figure out the text boundary
	RECT rect = { x,y,0,0};
	font->DrawText(NULL, text.c_str(), text.length(), &rect, DT_CALCRECT, color);
	//print the text;
	font->DrawText(spriteobj, text.c_str(), text.length(), &rect, DT_LEFT, color);
}

//Initialize dsound
bool DirectSound_Init(HWND hwnd)
{
	//create DirectSound manager object
	dsound = new CSoundManager();
	//initialize DirectSound
	HRESULT result;
	result = dsound->Initialize(hwnd, DSSCL_PRIORITY);
	if (result != DS_OK)	return false;

	//set the primary buffer format
	result = dsound->SetPrimaryBufferFormat(2, 22050, 16);
	if (result != DS_OK)	return false;
	//return success
	return true;
}
//Delete dsound
void DirectSound_Shutdown()
{
	if (dsound)	delete dsound;
}
//Load sound file
CSound * LoadSound(string filename)
{
	HRESULT result;
	//create local reference to wave data
	CSound *wave = NULL;
	//attempt to load the wave file
	char s[255];
	sprintf(s, "%s", filename.c_str());
	result = dsound->Create(&wave, s);
	if (result != DS_OK)	wave = NULL;
	//return the wave
	return wave;
}
//Play sound
void MPlaySound(CSound * sound)
{
	sound->Play();
}

void MLoopSound(CSound * sound)
{
	sound->Play(0, DSBPLAY_LOOPING);
}

void MStopSound(CSound * sound)
{
	sound->Stop();
}

void DrawModel(MODEL * model)
{
	//any materials in this mesh?
	if (model->material_count == 0)
	{
		model->mesh->DrawSubset(0);
	}
	else
	{
		//draw each mesh subset
		for (DWORD i = 0; i < model->material_count; ++i)
		{
			//Set the material and texture for this subset
			d3ddev->SetMaterial(&model->materials[i]);
			if (model->textures[i])
			{
				if (model->textures[i]->GetType() == D3DRTYPE_TEXTURE)
				{
					D3DSURFACE_DESC desc;
					model->textures[i]->GetLevelDesc(0, &desc);
					if (desc.Width > 0)
					{
						d3ddev->SetTexture(0, model->textures[i]);
					}
				}
			}
			//Draw the mesh subset
			model->mesh->DrawSubset(i);
		}	
	}
}

void DeleteModel(MODEL * model)
{
	//remove materials from memory
	if (model->materials != NULL)
	{
		delete[] model->materials;
	}
	//remove texture from memory
	if (model->textures != NULL)
	{
		for (DWORD i = 0; i < model->material_count; ++i)
		{
			if (model->textures[i] != NULL)
				model->textures[i]->Release();
		}
		delete[] model->textures;
	}
	//remove mesh from memory
	if (model->mesh != NULL)
		model->mesh->Release();
	//remove model from memory
	if (model != NULL)
		free(model);
}

MODEL * LoadModel(string filename)
{
	MODEL *model = (MODEL*)malloc(sizeof(MODEL));
	LPD3DXBUFFER matBuffer;
	HRESULT result;
	//load mesh from the specified file
	result = D3DXLoadMeshFromX(
		filename.c_str(),
		D3DXMESH_SYSTEMMEM,
		d3ddev,
		NULL,
		&matBuffer,
		NULL,
		&model->material_count,
		&model->mesh
	);

	if (result != D3D_OK)
	{
		MessageBox(NULL, "Error loading model file", "Fuck!!!", NULL);
		return NULL;
	}
	//extract msterisl properties and texture names from material buffer
	LPD3DXMATERIAL d3dxMaterials = (LPD3DXMATERIAL)matBuffer->GetBufferPointer();
	model->materials = new D3DMATERIAL9[model->material_count];
	model->textures = new LPDIRECT3DTEXTURE9[model->material_count];
	//create the materials and texture
	for (DWORD i = 0; i < model->material_count; ++i)
	{
		//grab the material
		model->materials[i] = d3dxMaterials[i].MatD3D;
		//set ambient color for material
		model->materials[i].Ambient = model->materials[i].Diffuse;
		model->textures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL)
		{
			string filename = d3dxMaterials[i].pTextureFilename;
			if (FindFile(&filename))
			{
				result = D3DXCreateTextureFromFile(
					d3ddev, filename.c_str(), &model->textures[i]);
				if (result != D3D_OK)
				{
					MessageBox(0, "Could not find texture", "Fuck!!!", NULL);
					return false;
				}
			}
		}
	}
	//done using material buffer
	matBuffer->Release();
	return model;
}

bool FindFile(string * filename)
{
	if (!filename)	return false;
	//look for file using original filename and path
	if (DoesFileExist(*filename))	return true;
	//since the file was not found, try removing the path
	string pathOnly;
	string filenameOnly;
	SplitPath(*filename, &pathOnly, &filenameOnly);
	//is file found in current folder, wothout the path
	if (DoesFileExist(filenameOnly))
	{
		*filename = filenameOnly;
		return true;
	}
	//not found 
	return false;
}

bool DoesFileExist(const string & filename)
{
	return (_access(filename.c_str(), 0) != -1);
}

void SplitPath(const string & inputPath, string * pathOnly, string * filenameOnly)
{
	string fullPath(inputPath);
	replace(fullPath.begin(), fullPath.end(), '\\', '/');
	string::size_type lastSlashPos = fullPath.find_last_of("/");
	//check for there being no path element in the input
	if (lastSlashPos == string::npos)
	{
		*pathOnly = "";
		*filenameOnly = fullPath;
	}
	else
	{
		if (pathOnly)
		{
			*pathOnly = fullPath.substr(0, lastSlashPos);
		}
		if (filenameOnly)
		{
			*filenameOnly = fullPath.substr(lastSlashPos + 1,
				fullPath.size() - lastSlashPos - 1);
		}
	}
}

void SetCamera(float posx, float posy, float posz, float lookx, float looky, float lookz)
{
	float fov = D3DX_PI / 4.0;
	float aspectRatio = SCREENW / SCREENH;
	float nearRange = 1.0;
	float farRange = 2000.0;
	D3DXVECTOR3 updir = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 position = D3DXVECTOR3(posx, posy, posz);
	D3DXVECTOR3 target = D3DXVECTOR3(lookx, looky, lookz);
	//set the perspective
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, fov, aspectRatio, nearRange, farRange);
	d3ddev->SetTransform(D3DTS_PROJECTION, &matProj);
	//set up the camera view matrix
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &position, &target, &updir);
	d3ddev->SetTransform(D3DTS_VIEW, &matView);
}
