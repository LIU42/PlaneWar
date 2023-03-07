#ifndef __PLANEWAR_H__
#define __PLANEWAR_H__

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <time.h>

#include "bullet.h"
#include "plane.h"
#include "resource.h"

enum Status { START, PLAYING, PAUSE, OVER, EXIT };

struct Fonts
{
	TTF_Font* pTitle;
	TTF_Font* pInfo;
};

struct Images
{
	SDL_Surface* pBackground;
	SDL_Surface* pHeroBullet;
	SDL_Surface* pEnemy1Bullet;
	SDL_Surface* pEnemy2Bullet;
	SDL_Surface* pHero[Hero::IMAGE_INDEX_MAX];
	SDL_Surface* pEnemy0[Enemy0::IMAGE_INDEX_MAX];
	SDL_Surface* pEnemy1[Enemy1::IMAGE_INDEX_MAX];
	SDL_Surface* pEnemy2[Enemy2::IMAGE_INDEX_MAX];
};

struct Timers
{
	SDL_TimerID addAliveScore;
};

class MainGame
{
	private:
		static constexpr auto TITLE = "Plane War";

	private:
		static const int SCREEN_WIDTH = 450;
		static const int SCREEN_HEIGHT = 700;
		static const int GAME_FPS = 60;

	private:
		static const int BORDER_X = 20;
		static const int BORDER_Y = 30;
		static const int BORDER_TEXT = 5;

	private:
		static const int RAND_MOD = 1000;
		static const int ALIVE_SCORE = 10;
		static const int ADD_ALIVE_SCORE_INTERVAL = 1000;
		static const int BACKGROUND_SCROLL_SPEED = 1;

	private:
		static const int TITLE_FONT_SIZE = 25;
		static const int INFO_FONT_SIZE = 17;
		static const int INFO_MAX_LENGTH = 30;

	private:
		static const int TITLE_WELCOME_WIDTH = 260;
		static const int TITLE_PAUSE_WIDTH = 70;
		static const int TITLE_OVER_WIDTH = 120;
		static const int INFO_WIDTH = 220;
		static const int SCORE_WIDTH = 140;
		static const int HEALTH_WIDTH = 80;

	private:
		static const int TITLE_UPPER = 210;
		static const int INFO_UPPER = 525;
		static const int SCORE_UPPER = 280;
		static const int BEST_SCORE_UPPER = 315;

	private:
		static constexpr SDL_Color BLACK = { 0, 0, 0 };
		static constexpr SDL_Color RED = { 255, 0, 0 };

	private:
		SDL_Window* pWindow;
		SDL_Surface* pSurface;
		SDL_PixelFormat* pFormat;
		SDL_SysWMinfo windowInfo;
		SDL_Rect screenRect;
		SDL_Event event;
		const Uint8* pKeyStatus;

	private:
		Images images;
		Fonts fonts;
		Timers timers;

	private:
		HeroBulletList heroBulletList;
		Enemy1BulletList enemy1BulletList;
		Enemy2BulletList enemy2BulletList;

	private:
		Hero hero;
		Enemy0List enemy0List;
		Enemy1List enemy1List;
		Enemy2List enemy2List;

	private:
		Status status;
		int score;
		int bestScore;
		int backgroundScrollUpper;

	private:
		static Uint32 addAliveScoreCallback(Uint32, void*);

	private:
		SDL_RWops* getResource(LPCWSTR, LPCWSTR);
		SDL_Surface* loadSurface(Uint32);

	private:
		int getRandomInRange(int);

	private:
		void getVersion();
		void initSystem();
		void initWindow();
		void initGame();
		void loadImages();
		void loadFonts();
		void addTimers();

	private:
		void freeImages();
		void freeFonts();
		void removeTimers();
		void closeWindow();
		void closeSystem();

	private:
		void restart();
		void addEnemy();
		void updateBackground();
		void updateHero();
		void updateEnemy();
		void updateBullet();

	private:
		void heroBulletHitDetection();
		void enemyBulletHitDetection();
		void heroCrashDetection();

	private:
		void displayText(const char*, TTF_Font*, int, int, SDL_Color);
		void displayBackground();
		void displayPlane();
		void displayBullet();
		void displayInfo();

	public:
		MainGame();
		~MainGame();

	public:
		void addAliveScore();
		void displayOverflow(SDL_Surface*, SDL_Rect&);

	public:
		bool isRunning();
		void update();
		void events();
		void display();
		void delay(Uint32, Uint32);
};
#endif