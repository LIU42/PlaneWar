#ifndef __PLANE_H__
#define __PLANE_H__

using namespace std;

struct Point
{
	int x;
	int y;
};

struct EnemyData
{
	int identity;
	int health;
	int width;
	int height;
	int speed;
	int indexMax;
	int appendScore;
	double probability;
};

struct BulletData
{
	int identity;
	int width;
	int height;
	int speed;
	int damage;
};

class Plane
{
	public:
		SDL_Rect rect;
		int identity;
		int health;
		int speed;
		int index;
		int indexMax;
		bool isAlive;
		bool isHit;

	public:
		void animate();
		void move();
		void down();
		void miss();
};

class Enemy : public Plane
{
	public:
		Enemy(Point, EnemyData&);
		void down();
		void fire();
		void display();
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
		SDL_Rect rect;
		int identity;
		int speed;
		int damage;
		bool isAlive;

	public:
		Bullet(Point, BulletData&);
		void move();
		void miss();
		void hitHero();
		void hitEnemy(vector <Enemy>&, int);
		void display();
};

#endif