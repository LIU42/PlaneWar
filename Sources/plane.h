#ifndef __PLANE_H__
#define __PLANE_H__

#include "header.h"

using namespace std;

class Plane
{
	public:
		SDL_Surface* image;
		SDL_Rect rect;
		int id;
		int health;
		int status;
		int appearance;

	public:
		void changeAppearance();
		void move(int);
		void down(int);
		void miss(int);
};

class Enemy : public Plane
{
	public:
		Enemy(int, int, int, int, int, int);
		void fire();
		void display(int);
};

class Hero : public Plane
{
	public:
		int bombCount;

	public:
		void init();
		void move();
		void fire();
		void releaseBomb(vector <Enemy>&, int);
		void crash(vector <Enemy>&, int);
		void display();
};

class Bullet
{
	public:
		SDL_Surface* image;
		SDL_Rect rect;
		int id;
		int status;

	public:
		Bullet(int, int, int, int, int);
		void move(int);
		void miss();
		void hit(int);
		void hit(vector <Enemy>&, int);
		void display();
};
#endif