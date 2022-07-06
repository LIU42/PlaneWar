#ifndef __PLANEWAR_H__
#define __PLANEWAR_H__

#include "header.h"
#include "config.h"
#include "plane.h"
#include "resource.h"

using namespace std;

struct Font
{
	TTF_Font* title;
	TTF_Font* info;
};

struct Color
{
	SDL_Color black;
	SDL_Color red;
};

struct Timer
{
	SDL_TimerID planeChange;
	SDL_TimerID planeFire;
	SDL_TimerID planeDown;
	SDL_TimerID aliveScore;
};

struct Image
{
	SDL_PixelFormat* format;
	SDL_Surface* surface;
	SDL_Surface* background;
	SDL_Surface* heroBullet;
	SDL_Surface* enemy1Bullet;
	SDL_Surface* enemy2Bullet;
	SDL_Surface* hero[HERO_IMG_MAX];
	SDL_Surface* enemy0[ENEMY0_IMG_MAX];
	SDL_Surface* enemy1[ENEMY1_IMG_MAX];
	SDL_Surface* enemy2[ENEMY2_IMG_MAX];
};

class MainGame
{
	public:
		HINSTANCE hInstance;
		SDL_Window* window;
		SDL_Event events;
		SDL_Rect screen;
		const Uint8* keyStatus;

	public:
		Image image;
		Font font;
		Color color;
		Timer timer;

	public:
		Hero hero;
		vector <Enemy> enemy0;
		vector <Enemy> enemy1;
		vector <Enemy> enemy2;
		vector <Bullet> heroBullet;
		vector <Bullet> enemy1Bullet;
		vector <Bullet> enemy2Bullet;

	public:
		int status;
		int score;
		int bestScore;
		int backgroundY;

	public:
		SDL_RWops* getResource(HINSTANCE, LPCWSTR, LPCWSTR);
		SDL_Surface* loadSurface(int);

	public:
		void initGame();
		void initWindow();
		void initColor();
		void loadImage();
		void loadFonts();
		void addTimer();
		void freeImage();
		void freeFont();
		void removeTimer();
		void close();

	public:
		void addEnemy(vector <Enemy>&, double, int, int, int, int, int);
		void updateEnemy(vector <Enemy>&, int, int);
		void updateBullet(vector <Bullet>&, int, int);
		void update();
		void control();
		void overflowBlit(SDL_Surface*, SDL_Rect);
		void displayText(const char*, TTF_Font*, int, int, SDL_Color);
		void displayBackground();
		void displayPlane();
		void displayInfo();
		void display();
};

extern MainGame game;
#endif