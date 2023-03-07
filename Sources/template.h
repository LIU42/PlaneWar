#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#include "planewar.h"

template <typename EnemyList> void heroReleaseBomeTemplate(EnemyList& enemyList, int& score)
{
	for (auto enemyIt = enemyList.begin(); enemyIt != enemyList.end(); ++enemyIt)
	{
		enemyIt->hit(enemyIt->HEALTH);
		score += enemyIt->DESTROY_SCORE;
	}
}

template <typename BulletList> void updateBulletTemplate(BulletList& bulletList, int screenHeight)
{
	for (auto bulletIt = bulletList.begin(); bulletIt != bulletList.end();)
	{
		bulletIt->move();

		if (!bulletIt->getIsAlive() || bulletIt->getIsOutOfRange(screenHeight))
		{
			bulletIt = bulletList.erase(bulletIt);
		}
		else { ++bulletIt; }
	}
}

template <typename EnemyList> void heroBulletHitDetectionTemplate(HeroBulletList& heroBulletList, EnemyList& enemyList, int& score)
{
	for (auto bulletIt = heroBulletList.begin(); bulletIt != heroBulletList.end(); ++bulletIt)
	{
		for (auto enemyIt = enemyList.begin(); enemyIt != enemyList.end(); ++enemyIt)
		{
			int distanceX = SDL_abs(bulletIt->getCenterX() - enemyIt->getCenterX()) + HeroBullet::HIT_DIFF;
			int distanceY = SDL_abs(bulletIt->getCenterY() - enemyIt->getCenterY()) + HeroBullet::HIT_DIFF;

			if (distanceX <= enemyIt->WIDTH / 2 && distanceY <= enemyIt->HEIGHT / 2 && enemyIt->getIsAlive())
			{
				enemyIt->hit(HeroBullet::DAMAGE);
				bulletIt->hit();

				if (enemyIt->getHealth() <= 0) { score += enemyIt->DESTROY_SCORE; }
			}
		}
	}
}

template <typename EnemyBulletList> void enemyBulletHitDetectionTemplate(Hero& hero, EnemyBulletList& enemyBulletList)
{
	for (auto bulletIt = enemyBulletList.begin(); bulletIt != enemyBulletList.end(); ++bulletIt)
	{
		int distanceX = SDL_abs(bulletIt->getCenterX() - hero.getCenterX()) + bulletIt->HIT_DIFF;
		int distanceY = SDL_abs(bulletIt->getCenterY() - hero.getCenterY()) + bulletIt->HIT_DIFF;

		if (distanceX <= Hero::WIDTH / 2 && distanceY <= Hero::HEIGHT / 2 && hero.getIsAlive())
		{
			hero.hit(bulletIt->DAMAGE);
			bulletIt->hit();
		}
	}
}

template <typename EnemyList> void heroCrashDetectionTemplate(Hero& hero, EnemyList& enemyList, int& score)
{
	for (auto enemyIt = enemyList.begin(); enemyIt != enemyList.end(); ++enemyIt)
	{
		int distanceX = SDL_abs(hero.getCenterX() - enemyIt->getCenterX()) + Hero::CRASH_DIFF;
		int distanceY = SDL_abs(hero.getCenterY() - enemyIt->getCenterY()) + Hero::CRASH_DIFF;

		if (distanceX <= (Hero::WIDTH + enemyIt->WIDTH) / 2 && distanceY <= (Hero::HEIGHT + enemyIt->HEIGHT) / 2 && enemyIt->getIsAlive())
		{
			hero.hit(Hero::HEALTH);
			enemyIt->hit(enemyIt->HEALTH);
			score += enemyIt->DESTROY_SCORE;
		}
	}
}

template <typename EnemyList> void displayEnemyTemplate(MainGame* game, EnemyList& enemyList, SDL_Surface** pEnemySurface)
{
	for (auto enemyIt = enemyList.begin(); enemyIt != enemyList.end(); ++enemyIt)
	{
		if (!enemyIt->getIsDestroyed())
		{
			game->displayOverflow(pEnemySurface[enemyIt->getImageIndex()], *enemyIt);
		}
	}
}

template <typename BulletList> void displayBulletTemplate(MainGame* game, BulletList& bulletList, SDL_Surface* pBulletSurface)
{
	for (auto bulletIt = bulletList.begin(); bulletIt != bulletList.end(); ++bulletIt)
	{
		game->displayOverflow(pBulletSurface, *bulletIt);
	}
}
#endif