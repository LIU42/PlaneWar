#include "planewar.h"

using namespace std;

void Plane::changeAppearance()
{
	if (game.status == PLAYING)
	{
		switch (appearance)
		{
			case APPEARANCE1: appearance = APPEARANCE2; break;
			case APPEARANCE2: appearance = APPEARANCE1; break;
		}
	}
}

void Plane::move(int speed) { rect.y += speed; }

void Plane::miss(int statusMax)
{
	if (rect.y > SCREEN_HEIGHT)
	{
		health = 0;
		status = statusMax;
	}
}

void Plane::down(int statusMax)
{
	if (health <= 0 && game.status == PLAYING)
	{
		health = 0;
		status += (status < statusMax) ? 1 : 0;
	}
}

void Hero::init()
{
	rect = { SCREEN_WIDTH / 2 - HERO_WIDTH / 2, SCREEN_HEIGHT - HERO_HEIGHT - 40, HERO_WIDTH, HERO_HEIGHT };
	health = HERO_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
	bombCount = HERO_BOMB_INIT_COUNT;
}

void Hero::move()
{
	if (game.keyStatus[SDL_SCANCODE_W] && rect.y >= BORDER_Y) { rect.y -= HERO_SPEED; }
	if (game.keyStatus[SDL_SCANCODE_S] && rect.y <= SCREEN_HEIGHT - HERO_HEIGHT - BORDER_Y) { rect.y += HERO_SPEED; }
	if (game.keyStatus[SDL_SCANCODE_A] && rect.x >= BORDER_X) { rect.x -= HERO_SPEED; }
	if (game.keyStatus[SDL_SCANCODE_D] && rect.x <= SCREEN_WIDTH - HERO_WIDTH - BORDER_X) { rect.x += HERO_SPEED; }
}

void Hero::fire()
{
	if (status == ALIVE_STATUS && game.status == PLAYING)
	{
		int x = rect.x + HERO_WIDTH / 2 - HERO_BULLET_WIDTH / 2 + 1;
		int y = rect.y - HERO_BULLET_HEIGHT;

		game.heroBullet.push_back(Bullet(x, y, HERO_BULLET_ID, HERO_BULLET_WIDTH, HERO_BULLET_HEIGHT));
	}
}

void Hero::releaseBomb(vector <Enemy>& enemy, int score)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		enemy[i].health = 0;
		enemy[i].status = DOWN_STATUS;
		game.score += score;
	}
}

void Hero::crash(vector <Enemy>& enemy, int score)
{
	if (status == ALIVE_STATUS)
	{
		for (int i = 0; i < enemy.size(); i++)
		{
			int distanceX = SDL_abs((rect.x + HERO_WIDTH / 2) - (enemy[i].rect.x + enemy[i].rect.w / 2));
			int distanceY = SDL_abs((rect.y + HERO_HEIGHT / 2) - (enemy[i].rect.y + enemy[i].rect.h / 2));

			if (distanceX <= (HERO_WIDTH + enemy[i].rect.w) / 2 - CRASH_DEV && distanceY <= (HERO_HEIGHT + enemy[i].rect.h) / 2 - CRASH_DEV && enemy[i].status == ALIVE_STATUS)
			{
				health = 0;
				enemy[i].health = 0;
				game.score += score;
			}
		}
	}
}

void Hero::display()
{
	if (status < HERO_STATUS_MAX)
	{
		switch (bool(status == ALIVE_STATUS))
		{
			case true: image = game.image.hero[appearance]; break;
			case false: image = game.image.hero[status + 1]; break;
		}
		SDL_BlitSurface(image, NULL, game.image.surface, &rect);
	}
}

Enemy::Enemy(int x, int y, int id, int width, int height, int health)
{
	this->rect = { x, y, width, height };
	this->id = id;
	this->health = health;
	this->status = ALIVE_STATUS;
	this->appearance = APPEARANCE1;
}

void Enemy::fire()
{
	if (status == ALIVE_STATUS && game.status == PLAYING)
	{
		if (id == ENEMY1_ID)
		{
			int x = rect.x + ENEMY1_WIDTH / 2 - ENEMY1_BULLET_WIDTH / 2 + 1;
			int y = rect.y + ENEMY1_HEIGHT;

			game.enemy1Bullet.push_back(Bullet(x, y, ENEMY1_BULLET_ID, ENEMY1_BULLET_WIDTH, ENEMY1_BULLET_HEIGHT));
		}
		else if (id == ENEMY2_ID && health > ENEMY2_HP / 2)
		{
			int x = rect.x + ENEMY2_WIDTH / 2 - ENEMY2_BULLET_WIDTH / 2 + 1;
			int y = rect.y + ENEMY2_HEIGHT;

			game.enemy2Bullet.push_back(Bullet(x, y, ENEMY2_BULLET_ID, ENEMY2_BULLET_WIDTH, ENEMY2_BULLET_HEIGHT));
		}
	}
}

void Enemy::display(int statusMax)
{
	if (status < statusMax)
	{
		if (id == ENEMY0_ID)
		{
			image = game.image.enemy0[status];
		}
		else if (id == ENEMY1_ID)
		{
			switch (bool(status == ALIVE_STATUS))
			{
				case true: image = game.image.enemy1[int(health <= ENEMY1_HP / 2)]; break;
				case false: image = game.image.enemy1[status + 1]; break;
			}
		}
		else if (id == ENEMY2_ID)
		{
			if (status == ALIVE_STATUS)
			{
				switch (bool(health > ENEMY2_HP / 2))
				{
					case true: image = game.image.enemy2[appearance]; break;
					case false: image = game.image.enemy2[2]; break;
				}
			}
			else { image = game.image.enemy2[status + 2]; }
		}
		switch (bool(rect.y < 0))
		{
			case true: game.overflowBlit(image, rect); break;
			case false: SDL_BlitSurface(image, NULL, game.image.surface, &rect); break;
		}
	}
}

Bullet::Bullet(int x, int y, int id, int width, int height)
{
	this->rect = { x, y, width, height };
	this->id = id;
	this->status = ALIVE_STATUS;
}

void Bullet::move(int speed)
{
	switch (bool(id == HERO_BULLET_ID))
	{
		case true: rect.y -= speed; break;
		case false: rect.y += speed; break;
	}
}

void Bullet::miss()
{
	switch (bool(id == HERO_BULLET_ID))
	{
		case true: if (rect.y <= -HERO_BULLET_HEIGHT) { status = DOWN_STATUS; } break;
		case false: if (rect.y > SCREEN_HEIGHT) { status = DOWN_STATUS; } break;
	}
}

void Bullet::hit(int damage)
{
	int distanceX = SDL_abs((rect.x + rect.w / 2) - (game.hero.rect.x + HERO_WIDTH / 2));
	int distanceY = SDL_abs((rect.y + rect.h / 2) - (game.hero.rect.y + HERO_HEIGHT / 2));

	if (distanceX <= HERO_WIDTH / 2 - ENEMY_HIT_DEV && distanceY <= HERO_HEIGHT / 2 - ENEMY_HIT_DEV && game.hero.health > 0)
	{
		game.hero.health -= damage;
		status = DOWN_STATUS;
	}
}

void Bullet::hit(vector <Enemy>& enemy, int score)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		int distanceX = SDL_abs((rect.x + HERO_BULLET_WIDTH / 2) - (enemy[i].rect.x + enemy[i].rect.w / 2));
		int distanceY = SDL_abs((rect.y + HERO_BULLET_HEIGHT / 2) - (enemy[i].rect.y + enemy[i].rect.h / 2));

		if (distanceX <= enemy[i].rect.w / 2 - HERO_HIT_DEV && distanceY <= enemy[i].rect.h / 2 - HERO_HIT_DEV && enemy[i].health > 0)
		{
			enemy[i].health -= HERO_BULLET_DAMAGE;
			status = DOWN_STATUS;

			if (enemy[i].health <= 0) { game.score += score; }
		}
	}
}

void Bullet::display()
{
	switch (id)
	{
		case HERO_BULLET_ID: image = game.image.heroBullet; break;
		case ENEMY1_BULLET_ID: image = game.image.enemy1Bullet; break;
		case ENEMY2_BULLET_ID: image = game.image.enemy2Bullet; break;
	}
	switch (bool(rect.y < 0))
	{
		case true: game.overflowBlit(image, rect); break;
		case false: SDL_BlitSurface(image, NULL, game.image.surface, &rect); break;
	}
	SDL_BlitSurface(image, NULL, game.image.surface, &rect);
}