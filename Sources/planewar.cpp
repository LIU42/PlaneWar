#include "planewar.h"

SDL_RWops* MainGame::getResource(LPCWSTR name, LPCWSTR type)
{
	HINSTANCE hInst = windowInfo.info.win.hinstance;
	HRSRC hRsrc = FindResource(hInst, name, type);
	DWORD size = SizeofResource(hInst, hRsrc);
	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	LPVOID data = LockResource(hGlobal);
	return SDL_RWFromConstMem(data, size);
}

SDL_Surface* MainGame::loadSurface(Uint32 id)
{
	SDL_RWops* pResource = getResource(MAKEINTRESOURCE(id), TEXT("PNG"));
	SDL_Surface* pOriginalSurface = IMG_LoadPNG_RW(pResource);
	SDL_Surface* pConvertedSurface = SDL_ConvertSurface(pOriginalSurface, pFormat, NULL);
	SDL_FreeSurface(pOriginalSurface);
	SDL_FreeRW(pResource);
	return pConvertedSurface;
}

Uint32 addAliveScoreCallback(Uint32 interval, void* pParam)
{
	((MainGame*)pParam)->addAliveScore();
	return interval;
}

int MainGame::getRandomInRange(int enemyWidth)
{
	int maxValue = SCREEN_WIDTH - BORDER_X - enemyWidth;
	int minValue = BORDER_X;

	return rand() % (maxValue - minValue) + minValue;
}

void MainGame::getVersion()
{
	windowInfo.version.major = SDL_MAJOR_VERSION;
	windowInfo.version.minor = SDL_MINOR_VERSION;
	windowInfo.version.patch = SDL_PATCHLEVEL;
}

void MainGame::initSystem()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
}

void MainGame::initWindow()
{
	pWindow = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	pSurface = SDL_GetWindowSurface(pWindow);
	pFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
	pKeyStatus = SDL_GetKeyboardState(NULL);
	screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	SDL_GetWindowWMInfo(pWindow, &windowInfo);
}

void MainGame::loadImages()
{
	images.pBackground = loadSurface(IDB_PNG1);
	images.pHeroBullet = loadSurface(IDB_PNG34);
	images.pEnemy1Bullet = loadSurface(IDB_PNG32);
	images.pEnemy2Bullet = loadSurface(IDB_PNG33);

	for (int i = 0; i < Hero::IMAGE_INDEX_MAX; i++) { images.pHero[i] = loadSurface(IDB_PNG25 + i); }
	for (int i = 0; i < Enemy0::IMAGE_INDEX_MAX; i++) { images.pEnemy0[i] = loadSurface(IDB_PNG2 + i); }
	for (int i = 0; i < Enemy1::IMAGE_INDEX_MAX; i++) { images.pEnemy1[i] = loadSurface(IDB_PNG8 + i); }
	for (int i = 0; i < Enemy2::IMAGE_INDEX_MAX; i++) { images.pEnemy2[i] = loadSurface(IDB_PNG15 + i); }
}

void MainGame::loadFonts()
{
	fonts.pTitle = TTF_OpenFontRW(getResource(MAKEINTRESOURCE(IDR_FONT1), RT_FONT), true, TITLE_FONT_SIZE);
	fonts.pInfo = TTF_OpenFontRW(getResource(MAKEINTRESOURCE(IDR_FONT1), RT_FONT), true, INFO_FONT_SIZE);
}

void MainGame::addTimers()
{
	timers.addAliveScore = SDL_AddTimer(ADD_ALIVE_SCORE_INTERVAL, addAliveScoreCallback, this);
}

void MainGame::freeImages()
{
	SDL_FreeSurface(images.pBackground);
	SDL_FreeSurface(images.pHeroBullet);
	SDL_FreeSurface(images.pEnemy1Bullet);
	SDL_FreeSurface(images.pEnemy2Bullet);

	for (int i = 0; i < Hero::IMAGE_INDEX_MAX; i++) { SDL_FreeSurface(images.pHero[i]); }
	for (int i = 0; i < Enemy0::IMAGE_INDEX_MAX; i++) { SDL_FreeSurface(images.pEnemy0[i]); }
	for (int i = 0; i < Enemy1::IMAGE_INDEX_MAX; i++) { SDL_FreeSurface(images.pEnemy1[i]); }
	for (int i = 0; i < Enemy2::IMAGE_INDEX_MAX; i++) { SDL_FreeSurface(images.pEnemy2[i]); }
}

void MainGame::freeFonts()
{
	TTF_CloseFont(fonts.pTitle);
	TTF_CloseFont(fonts.pInfo);
}

void MainGame::initGame()
{
	status = START;
	bestScore = 0;
	backgroundScrollUpper = 0;
}

void MainGame::removeTimers()
{
	SDL_RemoveTimer(timers.addAliveScore);
}

void MainGame::closeWindow()
{
	SDL_DestroyWindow(pWindow);
	SDL_FreeFormat(pFormat);
}

void MainGame::closeSystem()
{
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void MainGame::addAliveScore()
{
	if (status == PLAYING && hero.getIsAlive())
	{
		score += ALIVE_SCORE;
	}
}

void MainGame::restart()
{
	score = 0;
	hero.init(SCREEN_WIDTH, SCREEN_HEIGHT);
	enemy0List.clear();
	enemy1List.clear();
	enemy2List.clear();
	heroBulletList.clear();
	enemy1BulletList.clear();
	enemy2BulletList.clear();
}

void MainGame::addEnemy()
{
	if (score >= Enemy0::APPEND_SCORE && rand() % RAND_MOD <= Enemy0::APPEND_PROBABILITY)
	{
		enemy0List.push_back(Enemy0(getRandomInRange(Enemy0::WIDTH), -Enemy0::HEIGHT));
	}
	if (score >= Enemy1::APPEND_SCORE && rand() % RAND_MOD <= Enemy1::APPEND_PROBABILITY)
	{
		enemy1List.push_back(Enemy1(getRandomInRange(Enemy1::WIDTH), -Enemy1::HEIGHT, &enemy1BulletList));
	}
	if (score >= Enemy2::APPEND_SCORE && rand() % RAND_MOD <= Enemy2::APPEND_PROBABILITY)
	{
		enemy2List.push_back(Enemy2(getRandomInRange(Enemy2::WIDTH), -Enemy2::HEIGHT, &enemy2BulletList));
	}
}

void MainGame::updateBackground()
{
	if (status == PLAYING)
	{
		if (backgroundScrollUpper == SCREEN_HEIGHT)
		{
			backgroundScrollUpper = 0;
		}
		backgroundScrollUpper += BACKGROUND_SCROLL_SPEED;
	}
}

void MainGame::updateHero()
{
	hero.fire(GAME_FPS);
	hero.down(GAME_FPS);
	hero.animateEffect(GAME_FPS);

	if (hero.getIsDestroyed()) { status = OVER; }
}

void MainGame::updateEnemy()
{
	for (auto enemyIter = enemy0List.begin(); enemyIter != enemy0List.end();)
	{
		enemyIter->move();
		enemyIter->down(GAME_FPS);

		if (enemyIter->getIsDestroyed() || enemyIter->getIsOutOfRange(SCREEN_HEIGHT))
		{
			enemyIter = enemy0List.erase(enemyIter);
		}
		else { ++enemyIter; }
	}
	for (auto enemyIter = enemy1List.begin(); enemyIter != enemy1List.end();)
	{
		enemyIter->move();
		enemyIter->fire(GAME_FPS);
		enemyIter->down(GAME_FPS);
		
		if (enemyIter->getIsDestroyed() || enemyIter->getIsOutOfRange(SCREEN_HEIGHT))
		{
			enemyIter = enemy1List.erase(enemyIter);
		}
		else { ++enemyIter; }
	}
	for (auto enemyIter = enemy2List.begin(); enemyIter != enemy2List.end();)
	{
		enemyIter->move();
		enemyIter->fire(GAME_FPS);
		enemyIter->down(GAME_FPS);
		enemyIter->animateEffect(GAME_FPS);
		
		if (enemyIter->getIsDestroyed() || enemyIter->getIsOutOfRange(SCREEN_HEIGHT))
		{
			enemyIter = enemy2List.erase(enemyIter);
		}
		else { ++enemyIter; }
	}
}

template<class EnemyList> void heroBomeAttackTemplate(EnemyList& enemyList, int& score)
{
	for (auto enemyIter = enemyList.begin(); enemyIter != enemyList.end(); ++enemyIter)
	{
		enemyIter->hit(enemyIter->HEALTH);
		score += enemyIter->DESTROY_SCORE;
	}
}

template<class BulletList> void updateBulletTemplate(BulletList& bulletList, int screenHeight)
{
	for (auto bulletIter = bulletList.begin(); bulletIter != bulletList.end();)
	{
		bulletIter->move();

		if (!bulletIter->getIsAlive() || bulletIter->getIsOutOfRange(screenHeight))
		{
			bulletIter = bulletList.erase(bulletIter);
		}
		else { ++bulletIter; }
	}
}

template<class EnemyList> void heroBulletHitDetectTemplate(HeroBulletList& heroBulletList, EnemyList& enemyList, int& score)
{
	for (auto bulletIter = heroBulletList.begin(); bulletIter != heroBulletList.end(); ++bulletIter)
	{
		for (auto enemyIter = enemyList.begin(); enemyIter != enemyList.end(); ++enemyIter)
		{
			int distanceX = SDL_abs(bulletIter->getCenterX() - enemyIter->getCenterX());
			int distanceY = SDL_abs(bulletIter->getCenterY() - enemyIter->getCenterY());

			int rangeX = enemyIter->WIDTH / 2 - HeroBullet::HIT_DIFF;
			int rangeY = enemyIter->HEIGHT / 2 - HeroBullet::HIT_DIFF;

			if (distanceX <= rangeX && distanceY <= rangeY && enemyIter->getIsAlive())
			{
				enemyIter->hit(HeroBullet::DAMAGE);
				bulletIter->hit();

				if (enemyIter->getHealth() <= 0) { score += enemyIter->DESTROY_SCORE; }
			}
		}
	}
}

template<class EnemyBulletList> void enemyBulletHitDetectTemplate(Hero& hero, EnemyBulletList& enemyBulletList)
{
	for (auto bulletIter = enemyBulletList.begin(); bulletIter != enemyBulletList.end(); ++bulletIter)
	{
		int distanceX = SDL_abs(bulletIter->getCenterX() - hero.getCenterX());
		int distanceY = SDL_abs(bulletIter->getCenterY() - hero.getCenterY());

		int rangeX = Hero::WIDTH / 2 - bulletIter->HIT_DIFF;
		int rangeY = Hero::HEIGHT / 2 - bulletIter->HIT_DIFF;

		if (distanceX <= Hero::WIDTH / 2 && distanceY <= Hero::HEIGHT / 2 && hero.getIsAlive())
		{
			hero.hit(bulletIter->DAMAGE);
			bulletIter->hit();
		}
	}
}

template<class EnemyList> void heroCrashDetectionTemplate(Hero& hero, EnemyList& enemyList, int& score)
{
	for (auto enemyIter = enemyList.begin(); enemyIter != enemyList.end(); ++enemyIter)
	{
		int distanceX = SDL_abs(hero.getCenterX() - enemyIter->getCenterX());
		int distanceY = SDL_abs(hero.getCenterY() - enemyIter->getCenterY());

		int rangeX = (Hero::WIDTH + enemyIter->WIDTH) / 2 - Hero::CRASH_DIFF;
		int rangeY = (Hero::HEIGHT + enemyIter->HEIGHT) / 2 - Hero::CRASH_DIFF;

		if (distanceX <= rangeX && distanceY <= rangeY && enemyIter->getIsAlive())
		{
			hero.hit(Hero::HEALTH);
			enemyIter->hit(enemyIter->HEALTH);
			score += enemyIter->DESTROY_SCORE;
		}
	}
}

template<class EnemyList> void displayEnemyTemplate(MainGame* pGame, EnemyList& enemyList, SDL_Surface* pEnemySurfaceList[])
{
	for (auto enemyIter = enemyList.begin(); enemyIter != enemyList.end(); ++enemyIter)
	{
		if (!enemyIter->getIsDestroyed())
		{
			pGame->displayOverflow(pEnemySurfaceList[enemyIter->getImageIndex()], *enemyIter);
		}
	}
}

template<class BulletList> void displayBulletTemplate(MainGame* pGame, BulletList& bulletList, SDL_Surface* pBulletSurface)
{
	for (auto bulletIter = bulletList.begin(); bulletIter != bulletList.end(); ++bulletIter)
	{
		pGame->displayOverflow(pBulletSurface, *bulletIter);
	}
}

void MainGame::updateBullet()
{
	updateBulletTemplate(heroBulletList, SCREEN_HEIGHT);
	updateBulletTemplate(enemy1BulletList, SCREEN_HEIGHT);
	updateBulletTemplate(enemy2BulletList, SCREEN_HEIGHT);
}

void MainGame::heroBulletHitDetect()
{
	heroBulletHitDetectTemplate(heroBulletList, enemy0List, score);
	heroBulletHitDetectTemplate(heroBulletList, enemy1List, score);
	heroBulletHitDetectTemplate(heroBulletList, enemy2List, score);
}

void MainGame::enemyBulletHitDetect()
{
	enemyBulletHitDetectTemplate(hero, enemy1BulletList);
	enemyBulletHitDetectTemplate(hero, enemy2BulletList);
}

void MainGame::heroCrashDetect()
{
	if (hero.getIsAlive())
	{
		heroCrashDetectionTemplate(hero, enemy0List, score);
		heroCrashDetectionTemplate(hero, enemy1List, score);
		heroCrashDetectionTemplate(hero, enemy2List, score);
	}
}

void MainGame::displayText(const char* pText, TTF_Font* pFont, int x, int y, SDL_Color color)
{
	SDL_Surface* pTextSurface = TTF_RenderText_Blended(pFont, pText, color);
	SDL_Rect textRect = { x, y };

	SDL_BlitSurface(pTextSurface, NULL, pSurface, &textRect);
	SDL_FreeSurface(pTextSurface);
}

void MainGame::displayOverflow(SDL_Surface* pTargetSurface, SDL_Rect& targetRect)
{
	if (targetRect.y < 0)
	{
		SDL_Rect selfRect = { 0, -targetRect.y, targetRect.w, targetRect.h };
		SDL_Rect desRect = { targetRect.x, 0, targetRect.w, targetRect.h + targetRect.y };

		SDL_BlitSurface(pTargetSurface, &selfRect, pSurface, &desRect);
		return;
	}
	SDL_BlitSurface(pTargetSurface, NULL, pSurface, &targetRect);
}

void MainGame::displayBackground()
{
	SDL_Rect upperRect = { 0, backgroundScrollUpper - SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT };
	SDL_Rect lowerRect = { 0, backgroundScrollUpper, SCREEN_WIDTH, SCREEN_HEIGHT };

	displayOverflow(images.pBackground, upperRect);
	displayOverflow(images.pBackground, lowerRect);
}

void MainGame::displayPlane()
{
	if (status == PLAYING)
	{
		if (!hero.getIsDestroyed())
		{
			displayOverflow(images.pHero[hero.getImageIndex()], hero);
		}
		displayEnemyTemplate(this, enemy0List, images.pEnemy0);
		displayEnemyTemplate(this, enemy1List, images.pEnemy1);
		displayEnemyTemplate(this, enemy2List, images.pEnemy2);
	}
}

void MainGame::displayBullet()
{
	if (status == PLAYING)
	{
		displayBulletTemplate(this, heroBulletList, images.pHeroBullet);
		displayBulletTemplate(this, enemy1BulletList, images.pEnemy1Bullet);
		displayBulletTemplate(this, enemy2BulletList, images.pEnemy2Bullet);
	}
}

void MainGame::displayInfo()
{
	static char text[INFO_MAX_LENGTH];

	if (status == START)
	{
		displayText("Welcome to PlaneWar", fonts.pTitle, (SCREEN_WIDTH - TITLE_WELCOME_WIDTH) / 2, TITLE_UPPER, BLACK);
		displayText("Click anywhere to START...", fonts.pInfo, (SCREEN_WIDTH - INFO_WIDTH) / 2, INFO_UPPER, BLACK);
	}
	else if (status == PLAYING)
	{
		SDL_snprintf(text, INFO_MAX_LENGTH, "score: %d", score);
		displayText(text, fonts.pInfo, BORDER_TEXT, SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), BLACK);

		int textX = SCREEN_WIDTH - (HEALTH_WIDTH + BORDER_TEXT);
		int textY = SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE);

		SDL_snprintf(text, INFO_MAX_LENGTH, "HP: %d%%", hero.getHealth());
		displayText(text, fonts.pInfo, textX, textY, ((hero.getHealth() > Hero::HEALTH_ALERT) ? BLACK : RED));

		SDL_snprintf(text, INFO_MAX_LENGTH, "BOMB: %d", hero.getBombCount());
		displayText(text, fonts.pInfo, BORDER_TEXT, BORDER_TEXT, BLACK);
	}
	else if (status == PAUSE)
	{
		displayText("PAUSE", fonts.pTitle, (SCREEN_WIDTH - TITLE_PAUSE_WIDTH) / 2, TITLE_UPPER, BLACK);
		displayText("Click anywhere to RESUME...", fonts.pInfo, (SCREEN_WIDTH - INFO_WIDTH) / 2, INFO_UPPER, BLACK);
	}
	else if (status == OVER)
	{
		SDL_snprintf(text, INFO_MAX_LENGTH, "Your score: %d", score);
		displayText(text, fonts.pInfo, (SCREEN_WIDTH - SCORE_WIDTH) / 2, SCORE_UPPER, BLACK);

		SDL_snprintf(text, INFO_MAX_LENGTH, "Best score: %d", bestScore);
		displayText(text, fonts.pInfo, (SCREEN_WIDTH - SCORE_WIDTH) / 2, BEST_SCORE_UPPER, BLACK);

		displayText("Gameover!", fonts.pTitle, (SCREEN_WIDTH - TITLE_OVER_WIDTH) / 2, TITLE_UPPER, BLACK);
		displayText("Click anywhere to RESTART...", fonts.pInfo, (SCREEN_WIDTH - INFO_WIDTH) / 2, INFO_UPPER, BLACK);
	}
}

MainGame::MainGame() : hero(&heroBulletList)
{
	srand((unsigned)time(NULL));
	getVersion();
	initSystem();
	initWindow();
	initGame();
	loadImages();
	loadFonts();
	addTimers();
	restart();
}

MainGame::~MainGame()
{
	freeImages();
	freeFonts();
	removeTimers();
	closeWindow();
	closeSystem();
}

bool MainGame::isRunning()
{
	return status != EXIT;
}

void MainGame::update()
{
	if (status == OVER && score > bestScore)
	{
		bestScore = score;
	}
	else if (status == PLAYING)
	{
		addEnemy();
		updateBackground();
		updateHero();
		updateEnemy();
		updateBullet();
		heroBulletHitDetect();
		enemyBulletHitDetect();
		heroCrashDetect();
	}
}

void MainGame::events()
{
	if (status == PLAYING && hero.getIsAlive())
	{
		if (pKeyStatus[SDL_SCANCODE_W] && hero.getUpper() >= BORDER_Y) { hero.move(UP); }
		if (pKeyStatus[SDL_SCANCODE_S] && hero.getLower() <= SCREEN_HEIGHT - BORDER_Y) { hero.move(DOWN); }
		if (pKeyStatus[SDL_SCANCODE_A] && hero.getLeft() >= BORDER_X) { hero.move(LEFT); }
		if (pKeyStatus[SDL_SCANCODE_D] && hero.getRight() <= SCREEN_WIDTH - BORDER_X) { hero.move(RIGHT); }
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) { status = EXIT; }
		if (event.type == SDL_MOUSEBUTTONDOWN && isRunning())
		{
			if (status == OVER) { restart(); }
			status = PLAYING;
		}
		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p && status == PLAYING) { status = PAUSE; }
		if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE && status == PLAYING && hero.getBombCount() > 0)
		{
			hero.releaseBomb();
			heroBomeAttackTemplate(enemy0List, score);
			heroBomeAttackTemplate(enemy1List, score);
			heroBomeAttackTemplate(enemy2List, score);
		}
	}
}

void MainGame::display()
{
	displayBackground();
	displayPlane();
	displayBullet();
	displayInfo();
	SDL_UpdateWindowSurface(pWindow);
}

void MainGame::delay(Uint32 startTick, Uint32 endTick)
{
	int deltaTick = endTick - startTick;
	int delayTick = 1000 / GAME_FPS - deltaTick;

	SDL_Delay((delayTick > 0) ? delayTick : 0);
}