#include "bullet.h"

Bullet::Bullet(int x, int y, int width, int height) : SDL_Rect({ x, y, width, height })
{
	this->isAlive = true;
}

void Bullet::move()
{
	SDL_Rect::y += speed;
}

void Bullet::hit()
{
	isAlive = false;
}

bool Bullet::getIsAlive()
{
	return isAlive;
}

bool Bullet::getIsOutOfRange(int screenHeight)
{
	return SDL_Rect::y < -SDL_Rect::h || SDL_Rect::y > screenHeight;
}

int Bullet::getCenterX()
{
	return SDL_Rect::x + SDL_Rect::w / 2;
}

int Bullet::getCenterY()
{
	return SDL_Rect::y + SDL_Rect::h / 2;
}

HeroBullet::HeroBullet(int x, int y) : Bullet(x, y, WIDTH, HEIGHT)
{
	this->speed = SPEED;
	this->damage = DAMAGE;
}

Enemy1Bullet::Enemy1Bullet(int x, int y) : Bullet(x, y, WIDTH, HEIGHT)
{
	this->speed = SPEED;
	this->damage = DAMAGE;
}

Enemy2Bullet::Enemy2Bullet(int x, int y) : Bullet(x, y, WIDTH, HEIGHT)
{
	this->speed = SPEED;
	this->damage = DAMAGE;
}