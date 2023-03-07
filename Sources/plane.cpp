#include "plane.h"

Plane::Plane(int x, int y, int width, int height, int imageIndexMax) : SDL_Rect({ x, y, width, height })
{
	this->imageIndex = 0;
	this->imageIndexMax = imageIndexMax;
	this->downWaitTime = 0;
	this->isAlive = true;
}

void Plane::down(int gameFPS)
{
	if (health <= 0)
	{
		isAlive = false;
		health = 0;

		if (downWaitTime >= DOWN_INTERVAL && imageIndex < imageIndexMax)
		{
			imageIndex += 1;
			downWaitTime = 0;
		}
		downWaitTime += 1000 / gameFPS;
	}
}

void Plane::hit(int damage)
{
	health -= damage;
}

bool Plane::getIsAlive()
{
	return isAlive;
}

bool Plane::getIsDestroyed()
{
	return imageIndex >= imageIndexMax;
}

bool Plane::getIsOutOfRange(int screenHeight)
{
	return SDL_Rect::y > screenHeight;
}

int Plane::getHealth()
{
	return health;
}

int Plane::getCenterX()
{
	return SDL_Rect::x + SDL_Rect::w / 2;
}

int Plane::getCenterY()
{
	return SDL_Rect::y + SDL_Rect::h / 2;
}

int Plane::getImageIndex()
{
	return imageIndex;
}

Hero::Hero(HeroBulletList* pBulletList) : Plane(0, 0, WIDTH, HEIGHT, IMAGE_INDEX_MAX)
{
	this->speed = SPEED;
	this->pBulletList = pBulletList;
}

void Hero::init(int screenWidth, int screenHeight)
{
	SDL_Rect::x = (screenWidth - WIDTH) / 2;
	SDL_Rect::y = screenHeight - HEIGHT - INIT_BOTTOM;

	isAlive = true;
	health = HEALTH;
	fireWaitTime = 0;
	animateWaitTime = 0;
	bombCount = BOMB_INIT_COUNT;
	imageIndex = 0;
}

void Hero::move(HeroMoveDirect direct)
{
	switch (direct)
	{
		case UP: SDL_Rect::y -= SPEED; break;
		case DOWN: SDL_Rect::y += SPEED; break;
		case LEFT: SDL_Rect::x -= SPEED; break;
		case RIGHT: SDL_Rect::x += SPEED; break;
	}
}

void Hero::fire(int gameFPS)
{
	if (fireWaitTime >= FIRE_INTERVAL && isAlive)
	{
		int x = getCenterX() - HeroBullet::WIDTH / 2 + 1;
		int y = getCenterY() - HEIGHT / 2 - HeroBullet::HEIGHT;

		pBulletList->push_back(HeroBullet(x, y));
		fireWaitTime = 0;
	}
	fireWaitTime += 1000 / gameFPS;
}

void Hero::releaseBomb()
{
	if (bombCount > 0 && isAlive)
	{
		bombCount -= 1;
	}
}

void Hero::animateEffect(int gameFPS)
{
	if (animateWaitTime >= ANIMATE_INTERVAL && isAlive)
	{
		switch (imageIndex)
		{
			case APPEARANCE1: imageIndex = APPEARANCE2; break;
			case APPEARANCE2: imageIndex = APPEARANCE1; break;
		}
		animateWaitTime = 0;
	}
	animateWaitTime += 1000 / gameFPS;
}

int Hero::getBombCount()
{
	return bombCount;
}

int Hero::getUpper()
{
	return SDL_Rect::y;
}

int Hero::getLower()
{
	return SDL_Rect::y + Hero::HEALTH;
}

int Hero::getLeft()
{
	return SDL_Rect::x;
}

int Hero::getRight()
{
	return SDL_Rect::x + Hero::WIDTH;
}

Enemy::Enemy(int x, int y, int width, int height, int imageIndexMax) : Plane(x, y, width, height, imageIndexMax) {}

void Enemy::move()
{
	SDL_Rect::y += speed;
}

Enemy0::Enemy0(int x, int y) : Enemy(x, y, WIDTH, HEIGHT, IMAGE_INDEX_MAX)
{
	this->speed = SPEED;
	this->health = HEALTH;
}

Enemy1::Enemy1(int x, int y, Enemy1BulletList* pBulletList) : Enemy(x, y, WIDTH, HEIGHT, IMAGE_INDEX_MAX)
{
	this->pBulletList = pBulletList;
	this->speed = SPEED;
	this->health = HEALTH;
	this->fireWaitTime = 0;
	this->isHurt = false;
}

void Enemy1::fire(int gameFPS)
{
	if (fireWaitTime >= FIRE_INTERVAL && isAlive)
	{
		int x = getCenterX() - Enemy1Bullet::WIDTH / 2 + 1;
		int y = getCenterY() + HEIGHT / 2;

		pBulletList->push_back(Enemy1Bullet(x, y));
		fireWaitTime = 0;
	}
	fireWaitTime += 1000 / gameFPS;
}

void Enemy1::down(int gameFPS)
{
	if (health <= HURT_HEALTH && isAlive)
	{
		isHurt = true;
		imageIndex = IMAGE_INDEX_HURT;
	}
	Plane::down(gameFPS);
}

Enemy2::Enemy2(int x, int y, Enemy2BulletList* pBulletList) : Enemy(x, y, WIDTH, HEIGHT, IMAGE_INDEX_MAX)
{
	this->pBulletList = pBulletList;
	this->speed = SPEED;
	this->health = HEALTH;
	this->fireWaitTime = 0;
	this->animateWaitTime = 0;
	this->isHurt = false;
}

void Enemy2::fire(int gameFPS)
{
	if (fireWaitTime >= FIRE_INTERVAL && isAlive && !isHurt)
	{
		int x = getCenterX() - Enemy2Bullet::WIDTH / 2 + 1;
		int y = getCenterY() + HEIGHT / 2;

		pBulletList->push_back(Enemy2Bullet(x, y));
		fireWaitTime = 0;
	}
	fireWaitTime += 1000 / gameFPS;
}

void Enemy2::down(int gameFPS)
{
	if (health <= HURT_HEALTH && isAlive)
	{
		isHurt = true;
		imageIndex = IMAGE_INDEX_HURT;
	}
	Plane::down(gameFPS);
}

void Enemy2::animateEffect(int gameFPS)
{
	if (animateWaitTime >= ANIMATE_INTERVAL && isAlive && !isHurt)
	{
		switch (imageIndex)
		{
			case APPEARANCE1: imageIndex = APPEARANCE2; break;
			case APPEARANCE2: imageIndex = APPEARANCE1; break;
		}
		animateWaitTime = 0;
	}
	animateWaitTime += 1000 / gameFPS;
}