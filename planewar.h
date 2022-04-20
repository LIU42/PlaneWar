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
		HINSTANCE hinstance;
		SDL_Window* window;
		SDL_Event event;
		SDL_PixelFormat* format;
		const Uint8* keystatus;

	public:
		SDL_Surface* image;
		SDL_Surface* surface;
		SDL_Surface* background;
		SDL_Surface* text_surface;
		SDL_Surface* hero_bullet_img;
		SDL_Surface* enemy1_bullet_img;
		SDL_Surface* enemy2_bullet_img;
		SDL_Surface* hero_img[7];
		SDL_Surface* enemy0_img[6];
		SDL_Surface* enemy1_img[7];
		SDL_Surface* enemy2_img[10];

	public:
		SDL_Rect text_rect;
		SDL_Rect surface_rect;
		SDL_Rect background_rect_self;
		SDL_Rect background_rect_dst;
		SDL_Rect overflow_rect_self;
		SDL_Rect overflow_rect_dst;

	public:
		TTF_Font* font_title;
		TTF_Font* font_info;
		SDL_Color black;
		SDL_Color red;

	public:
		SDL_RWops* get_resource(HINSTANCE, LPCWSTR, LPCWSTR);
		SDL_Surface* load_surface(DWORD);

	public:
		void text(const char*, TTF_Font*, int, int, SDL_Color);
		void overflow_blit(SDL_Surface*, SDL_Rect);
		void init();
		void init_color();
		void load_image();
		void load_fonts();
		void free_image();
		void close_font();
		void close();
};

class Game
{
	public:
		int status;
		int score;
		int score_best;
		int background_position;

	public:
		SDL_TimerID hero_change;
		SDL_TimerID hero_fire;
		SDL_TimerID enemy1_fire;
		SDL_TimerID enemy2_change;
		SDL_TimerID enemy2_fire;
		SDL_TimerID aircraft_down;
		SDL_TimerID alive;

	public:
		default_random_engine random;
		uniform_real_distribution <double> randdouble;

	public:
		Game();
		void init();
		void add_timer();
		template <class Enemy>
		void add_enemy(vector <Enemy>&, double, int, int, int);
		void update();
		void event();
		void display_background();
		void display_plane();
		void display_info();
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
		void change_appearance();
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
		int bomb_count;

	public:
		Hero();
		void init();
		void move();
		void fire();
		template <class Enemy>
		void release_bomb(vector <Enemy>&, int);
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

class Hero_bullet : public Bullet
{
	public:
		Hero_bullet(int, int);
		void move();
		void miss();
		template <class Enemy>
		void hit(vector <Enemy>&, int, int, int);
		void display();
};

class Enemy1_bullet : public Bullet { public: Enemy1_bullet(int, int); };
class Enemy2_bullet : public Bullet { public: Enemy2_bullet(int, int); };

extern Window window;
extern Game game;
extern Hero hero;

extern vector <Enemy0> enemy0;
extern vector <Enemy1> enemy1;
extern vector <Enemy2> enemy2;

extern vector <Hero_bullet> hero_bullet;
extern vector <Enemy1_bullet> enemy1_bullet;
extern vector <Enemy2_bullet> enemy2_bullet;
#endif