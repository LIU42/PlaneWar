#ifndef __BULLET_H__
#define __BULLET_H__

#include <SDL.h>
#include <list>

using namespace std;

class Bullet : public SDL_Rect
{
	protected:
		int speed;
		int damage;
		bool isAlive;

	protected:
		Bullet(int, int, int, int);

	public:
		void move();
		void hit();

	public:
		bool getIsAlive();
		bool getIsOutOfRange(int);
		int getCenterX();
		int getCenterY();
};

class HeroBullet : public Bullet
{
	public:
		static const int WIDTH = 5;
		static const int HEIGHT = 10;
		static const int SPEED = -10;
		static const int DAMAGE = 100;
		static const int HIT_DIFF = 0;

	public:
		HeroBullet(int, int);
};

class Enemy1Bullet : public Bullet
{
	public:
		static const int WIDTH = 5;
		static const int HEIGHT = 10;
		static const int SPEED = 7;
		static const int DAMAGE = 40;
		static const int HIT_DIFF = 10;

	public:
		Enemy1Bullet(int, int);
};

class Enemy2Bullet : public Bullet
{
	public:
		static const int WIDTH = 20;
		static const int HEIGHT = 20;
		static const int SPEED = 15;
		static const int DAMAGE = 200;
		static const int HIT_DIFF = 10;

	public:
		Enemy2Bullet(int, int);
};

typedef list<HeroBullet> HeroBulletList;
typedef list<Enemy1Bullet> Enemy1BulletList;
typedef list<Enemy2Bullet> Enemy2BulletList;

#endif