#ifndef __PLANEWAR_H__
#define __PLANEWAR_H__

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <Windows.h>
#include <time.h>
#include <list>

#include "config.h"
#include "resource.h"
#include "plane.h"

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
	SDL_TimerID planeAnimate;
	SDL_TimerID planeFire;
	SDL_TimerID planeDown;
	SDL_TimerID addAliveScore;
};

struct Image
{
	SDL_PixelFormat* format;
	SDL_Surface* surface;
	SDL_Surface* background;
	SDL_Surface* heroBullet;
	SDL_Surface* enemy1Bullet;
	SDL_Surface* enemy2Bullet;
	SDL_Surface* hero[HERO_INDEX_MAX];
	SDL_Surface* enemy0[ENEMY0_INDEX_MAX];
	SDL_Surface* enemy1[ENEMY1_INDEX_MAX];
	SDL_Surface* enemy2[ENEMY2_INDEX_MAX];
};

class MainGame
{
	public:
		HINSTANCE hInstance;
		SDL_Window* window;
		SDL_Event event;
		SDL_Rect screen;
		const Uint8* keyStatus;

	public:
		Image image;
		Font font;
		Color color;
		Timer timer;

	public:
		EnemyData enemy0Data;
		EnemyData enemy1Data;
		EnemyData enemy2Data;
		BulletData heroBulletData;
		BulletData enemy1BulletData;
		BulletData enemy2BulletData;

	public:
		Hero hero;
		list <Enemy> enemy0;
		list <Enemy> enemy1;
		list <Enemy> enemy2;
		list <Bullet> heroBullet;
		list <Bullet> enemy1Bullet;
		list <Bullet> enemy2Bullet;

	public:
		int status;
		int score;
		int bestScore;

	public:
		SDL_RWops* getResource(HINSTANCE, LPCWSTR, LPCWSTR);
		SDL_Surface* loadSurface(int);

	public:
		void initGame();
		void initData();
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
		void update();
		void events();
		void display();

	public:
		void addEnemy(list <Enemy>&, EnemyData&);
		void updateEnemy(list <Enemy>&, EnemyData&);
		void updateBullet(list <Bullet>&);

	public:
		void overflowBlit(SDL_Surface*, SDL_Rect&);
		void displayText(const char*, TTF_Font*, Point, SDL_Color);
		void displayBackground();
		void displayPlane();
		void displayInfo();
};

extern MainGame game;
#endif