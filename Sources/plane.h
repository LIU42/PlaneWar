#ifndef __PLANE_H__
#define __PLANE_H__

#include <SDL.h>
#include <list>

#include "bullet.h"

using namespace std;

enum HeroMoveDirect { UP, DOWN, LEFT, RIGHT };
enum PlaneAppearance { APPEARANCE1, APPEARANCE2 };

class Plane : public SDL_Rect
{
	protected:
		static const int DOWN_INTERVAL = 100;

	protected:
		int health;
		int speed;
		int imageIndex;
		int imageIndexMax;
		int downWaitTime;
		bool isAlive;

	protected:
		Plane(int, int, int, int, int);

	public:
		void down(int);
		void hit(int);

	public:
		bool getIsAlive();
		bool getIsDestroyed();
		bool getIsOutOfRange(int);
		int getHealth();
		int getCenterX();
		int getCenterY();
		int getImageIndex();
};

class Hero : public Plane
{
	public:
		static const int WIDTH = 50;
		static const int HEIGHT = 62;
		static const int SPEED = 6;
		static const int HEALTH = 100;
		static const int HEALTH_ALERT = 30;
		static const int CRASH_DIFF = 20;

	public:
		static const int INIT_BOTTOM = 40;
		static const int BOMB_INIT_COUNT = 3;
		static const int IMAGE_INDEX_MAX = 6;
		static const int FIRE_INTERVAL = 100;
		static const int ANIMATE_INTERVAL = 250;

	private:
		HeroBulletList* pBulletList;

	private:
		int bombCount;
		int fireWaitTime;
		int animateWaitTime;

	public:
		Hero(HeroBulletList*);

	public:
		void init(int, int);
		void move(HeroMoveDirect);
		void fire(int);
		void releaseBomb();
		void animateEffect(int);

	public:
		int getBombCount();
		int getUpper();
		int getLower();
		int getLeft();
		int getRight();
};

class Enemy : public Plane
{
	protected:
		Enemy(int, int, int, int, int);

	public:
		void move();
};

class Enemy0 : public Enemy
{
	public:
		static const int WIDTH = 25;
		static const int HEIGHT = 20;
		static const int SPEED = 4;
		static const int HEALTH = 100;

	public:
		static const int APPEND_PROBABILITY = 25;
		static const int DESTROY_SCORE = 100;
		static const int APPEND_SCORE = 30;
		static const int IMAGE_INDEX_MAX = 5;

	public:
		Enemy0(int, int);
};

class Enemy1 : public Enemy
{
	public:
		static const int WIDTH = 40;
		static const int HEIGHT = 49;
		static const int SPEED = 3;
		static const int HEALTH = 200;
		static const int HURT_HEALTH = 100;

	public:
		static const int APPEND_PROBABILITY = 15;
		static const int DESTROY_SCORE = 1000;
		static const int APPEND_SCORE = 1000;
		static const int IMAGE_INDEX_MAX = 6;
		static const int IMAGE_INDEX_HURT = 1;
		static const int FIRE_INTERVAL = 400;

	private:
		Enemy1BulletList* pBulletList;

	private:
		int fireWaitTime;
		bool isHurt;

	public:
		Enemy1(int, int, Enemy1BulletList*);

	public:
		void fire(int);
		void down(int);
};

class Enemy2 : public Enemy
{
	public:
		static const int WIDTH = 83;
		static const int HEIGHT = 128;
		static const int SPEED = 2;
		static const int HEALTH = 1000;
		static const int HURT_HEALTH = 500;

	public:
		static const int APPEND_PROBABILITY = 2;
		static const int DESTROY_SCORE = 10000;
		static const int APPEND_SCORE = 10000;
		static const int IMAGE_INDEX_MAX = 9;
		static const int IMAGE_INDEX_HURT = 2;
		static const int FIRE_INTERVAL = 800;
		static const int ANIMATE_INTERVAL = 200;

	private:
		Enemy2BulletList* pBulletList;

	private:
		int fireWaitTime;
		int animateWaitTime;
		bool isHurt;

	public:
		Enemy2(int, int, Enemy2BulletList*);

	public:
		void fire(int);
		void down(int);
		void animateEffect(int);
};

typedef list <Enemy0> Enemy0List;
typedef list <Enemy1> Enemy1List;
typedef list <Enemy2> Enemy2List;

#endif