#ifndef __PLANEWAR_H__
#define __PLANEWAR_H__

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <Windows.h>
#include <vector>
#include <random>

#include "config.h"
#include "resource.h"

using namespace std;

class Window
{
	public:
		HINSTANCE hInstance;
		SDL_Window* window;
		SDL_Event events;
		SDL_PixelFormat* format;
		const Uint8* keyStatus;

	public:
		SDL_Surface* image;
		SDL_Surface* surface;
		SDL_Surface* background;
		SDL_Surface* heroBulletImg;
		SDL_Surface* enemy1BulletImg;
		SDL_Surface* enemy2BulletImg;
		SDL_Surface* heroImg[HERO_IMG_MAX];
		SDL_Surface* enemy0Img[ENEMY0_IMG_MAX];
		SDL_Surface* enemy1Img[ENEMY1_IMG_MAX];
		SDL_Surface* enemy2Img[ENEMY2_IMG_MAX];

	public:
		SDL_Rect srceenRect;
		SDL_Rect backgroundRectSelf;
		SDL_Rect backgroundRectDst;
		SDL_Rect overflowRectSelf;
		SDL_Rect overflowRectDst;

	public:
		TTF_Font* titleFont;
		TTF_Font* infoFont;
		SDL_Color black;
		SDL_Color red;

	public:
		SDL_RWops* getResource(HINSTANCE, LPCWSTR, LPCWSTR);
		SDL_Surface* loadSurface(DWORD);

	public:
		void text(const char*, TTF_Font*, int, int, SDL_Color);
		void overflowBlit(SDL_Surface*, SDL_Rect);
		void init();
		void initColor();
		void loadImage();
		void loadFonts();
		void freeImage();
		void freeFont();
		void close();
};

class Game
{
	public:
		int status;
		int score;
		int bestScore;
		int backgroundPosition;
		char text[INFO_MAX_LEN];

	public:
		SDL_TimerID heroChange;
		SDL_TimerID heroFire;
		SDL_TimerID enemy1Fire;
		SDL_TimerID enemy2Change;
		SDL_TimerID enemy2Fire;
		SDL_TimerID aircraftDown;
		SDL_TimerID alive;

	public:
		default_random_engine random;
		uniform_real_distribution <double> randP;

	public:
		Game();
		void init();
		void addTimer();
		void removeTimer();
		template <class Enemy>
		void addEnemy(vector <Enemy>&, double, int, int, int);
		void update();
		void events();
		void displayBackground();
		void displayPlane();
		void displayInfo();
		void display();
};

class Aircraft
{
	public:
		SDL_Rect rect;
		int hp;
		int status;
		int appearance;

	public:
		void changeAppearance();
		void move(int);
		void down(int);
		void miss(int);
};

class Bullet
{
	public:
		SDL_Rect rect;
		int status;

	public:
		void move(int);
		void miss();
		void hit(int, int, int);
		void display(SDL_Surface*);
};

class Hero : public Aircraft
{
	public:
		int bombCount;

	public:
		void init();
		void move();
		void fire();
		template <class Enemy>
		void releaseBomb(vector <Enemy>&, int);
		template <class Enemy>
		void crash(vector <Enemy>&, int, int, int);
		void display();
};

class Enemy0 : public Aircraft
{
	public:
		Enemy0(int, int);
		void display();
};

class Enemy1 : public Aircraft
{
	public:
		Enemy1(int, int);
		void fire();
		void display();
};

class Enemy2 : public Aircraft
{
	public:
		Enemy2(int, int);
		void fire();
		void display();
};

class HeroBullet : public Bullet
{
	public:
		HeroBullet(int, int);
		void move();
		void miss();
		template <class Enemy>
		void hit(vector <Enemy>&, int, int, int);
		void display();
};

class Enemy1Bullet : public Bullet { public: Enemy1Bullet(int, int); };
class Enemy2Bullet : public Bullet { public: Enemy2Bullet(int, int); };

extern Window window;
extern Game game;
extern Hero hero;

extern vector <Enemy0> enemy0;
extern vector <Enemy1> enemy1;
extern vector <Enemy2> enemy2;

extern vector <HeroBullet> heroBullet;
extern vector <Enemy1Bullet> enemy1Bullet;
extern vector <Enemy2Bullet> enemy2Bullet;
#endif