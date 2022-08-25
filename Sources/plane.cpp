#include "planewar.h"

void Plane::animate()
{
	if (!isHit)
	{
		switch (index)
		{
			case APPEARANCE1: index = APPEARANCE2; break;
			case APPEARANCE2: index = APPEARANCE1; break;
		}
	}
}

void Plane::move()
{
	rect.y += speed;
}

void Plane::miss()
{
	if (rect.y > SCREEN_HEIGHT)
	{
		isAlive = false;
		index = indexMax;
	}
}

void Plane::down()
{
	if (hp <= 0 || !isAlive)
	{
		isAlive = false;
		hp = 0;

		if (index < indexMax) { index += 1; }
	}
}

void Hero::init()
{
	rect = { SCREEN_WIDTH / 2 - HERO_WIDTH / 2, SCREEN_HEIGHT - HERO_HEIGHT - 40, HERO_WIDTH, HERO_HEIGHT };
	hp = HERO_HP;
	isAlive = true;
	isHit = false;
	speed = HERO_SPEED;
	index = 0;
	indexMax = HERO_INDEX_MAX;
	bombCount = HERO_BOMB_INIT_COUNT;
}

void Hero::move()
{
	if (game.keyStatus[SDL_SCANCODE_W] && rect.y >= BORDER_Y) { rect.y -= speed; }
	if (game.keyStatus[SDL_SCANCODE_S] && rect.y <= SCREEN_HEIGHT - BORDER_Y - rect.h) { rect.y += speed; }
	if (game.keyStatus[SDL_SCANCODE_A] && rect.x >= BORDER_X) { rect.x -= speed; }
	if (game.keyStatus[SDL_SCANCODE_D] && rect.x <= SCREEN_WIDTH - BORDER_X - rect.w) { rect.x += speed; }
}

void Hero::fire()
{
	static Point pos;

	if (isAlive)
	{
		pos.x = rect.x + rect.w / 2 - HERO_BULLET_WIDTH / 2 + 1;
		pos.y = rect.y - HERO_BULLET_HEIGHT;

		game.heroBullet.push_back(Bullet(pos, game.heroBulletData));
	}
}

void Hero::releaseBomb(vector <Enemy>& enemy, int score)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		enemy[i].isAlive = false;
		game.score += score;
	}
}

void Hero::crash(vector <Enemy>& enemy, int score)
{
	if (isAlive)
	{
		for (int i = 0; i < enemy.size(); i++)
		{
			int distanceX = SDL_abs((rect.x + rect.w / 2) - (enemy[i].rect.x + enemy[i].rect.w / 2)) + CRASH_DIFF;
			int distanceY = SDL_abs((rect.y + rect.h / 2) - (enemy[i].rect.y + enemy[i].rect.h / 2)) + CRASH_DIFF;

			if (distanceX <= (HERO_WIDTH + enemy[i].rect.w) / 2 && distanceY <= (HERO_HEIGHT + enemy[i].rect.h) / 2 && enemy[i].isAlive)
			{
				isAlive = false;
				enemy[i].isAlive = false;
				game.score += score;
			}
		}
	}
}

void Hero::display()
{
	if (index < indexMax)
	{
		SDL_BlitSurface(game.image.hero[index], NULL, game.image.surface, &rect);
	}
}

Enemy::Enemy(Point pos, EnemyData& data)
{
	rect = { pos.x, pos.y, data.width, data.height };
	id = data.id;
	hp = data.hp;
	index = 0;
	speed = data.speed;
	indexMax = data.indexMax;
	isAlive = true;
	isHit = false;
}

void Enemy::down()
{
	switch (id)
	{
		case ENEMY1_ID: if (hp <= ENEMY1_HP / 2 && isAlive) { isHit = true; index = ENEMY1_INDEX_HIT; } break;
		case ENEMY2_ID: if (hp <= ENEMY2_HP / 2 && isAlive) { isHit = true; index = ENEMY2_INDEX_HIT; } break;
	}
	Plane::down();
}

void Enemy::fire()
{
	static Point pos;

	if (isAlive)
	{
		if (id == ENEMY1_ID)
		{
			pos.x = rect.x + rect.w / 2 - ENEMY1_BULLET_WIDTH / 2 + 1;
			pos.y = rect.y + rect.h;

			game.enemy1Bullet.push_back(Bullet(pos, game.enemy1BulletData));
		}
		else if (id == ENEMY2_ID && !isHit)
		{
			pos.x = rect.x + rect.w / 2 - ENEMY2_BULLET_WIDTH / 2 + 1;
			pos.y = rect.y + rect.h;

			game.enemy2Bullet.push_back(Bullet(pos, game.enemy2BulletData));
		}
	}
}

void Enemy::display()
{
	static SDL_Surface* surface;

	if (index < indexMax)
	{
		switch (id)
		{
			case ENEMY0_ID: surface = game.image.enemy0[index]; break;
			case ENEMY1_ID: surface = game.image.enemy1[index]; break;
			case ENEMY2_ID: surface = game.image.enemy2[index]; break;
		}
		game.overflowBlit(surface, rect);
	}
}

Bullet::Bullet(Point pos, BulletData& data)
{
	rect = { pos.x, pos.y, data.width, data.height };
	id = data.id;
	speed = data.speed;
	damage = data.damage;
	isAlive = true;
}

void Bullet::move()
{
	if (id == HERO_BULLET_ID)
	{
		rect.y -= speed;
	}
	else { rect.y += speed; }
}

void Bullet::miss()
{
	if (id == HERO_BULLET_ID && rect.y <= -rect.h)
	{
		isAlive = false;
	}
	else if (rect.y > SCREEN_HEIGHT) { isAlive = false; }
}

void Bullet::hit()
{
	int distanceX = SDL_abs((rect.x + rect.w / 2) - (game.hero.rect.x + game.hero.rect.w / 2)) + ENEMY_HIT_DIFF;
	int distanceY = SDL_abs((rect.y + rect.h / 2) - (game.hero.rect.y + game.hero.rect.h / 2)) + ENEMY_HIT_DIFF;

	if (distanceX <= HERO_WIDTH / 2 && distanceY <= HERO_HEIGHT / 2 && game.hero.hp > 0)
	{
		game.hero.hp -= damage;
		isAlive = false;
	}
}

void Bullet::hit(vector <Enemy>& enemy, int score)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		int distanceX = SDL_abs((rect.x + rect.w / 2) - (enemy[i].rect.x + enemy[i].rect.w / 2)) + HERO_HIT_DIFF;
		int distanceY = SDL_abs((rect.y + rect.h / 2) - (enemy[i].rect.y + enemy[i].rect.h / 2)) + HERO_HIT_DIFF;

		if (distanceX <= enemy[i].rect.w / 2 && distanceY <= enemy[i].rect.h / 2 && enemy[i].hp > 0)
		{
			enemy[i].hp -= damage;
			isAlive = false;

			if (enemy[i].hp <= 0) { game.score += score; }
		}
	}
}

void Bullet::display()
{
	static SDL_Surface* surface;

	switch (id)
	{
		case HERO_BULLET_ID: surface = game.image.heroBullet; break;
		case ENEMY1_BULLET_ID: surface = game.image.enemy1Bullet; break;
		case ENEMY2_BULLET_ID: surface = game.image.enemy2Bullet; break;
	}
	game.overflowBlit(surface, rect);
}