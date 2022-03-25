#ifndef __PLANEWAR_H__
#define __PLANEWAR_H__

#include <SDL.h>
#include <SDL_gfx.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <Windows.h>
#include <vector>
#include <random>

using namespace std;

class Game
{
	public:
		int status;
		int score;
		int score_best;
		int background_position;
		const Uint8* keystatus;

	public:
		Game();
		void init();
		void set_window();
		void load_image();
		void load_fonts();
		void add_timer();
		void exit_game();
		void update();
		void events();
		void display();
};

class Aircraft
{
	public:
		SDL_Rect rect;
		int hp;
		int status;
		int change;

	public:
		void change_img();
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

class Enemy1_bullet : public Bullet
{
	public:
		Enemy1_bullet(int, int);
};

class Enemy2_bullet : public Bullet
{
	public:
		Enemy2_bullet(int, int);
};

extern Game game;
#endif