#include "planewar.h"

SDL_RWops* MainGame::getResource(HINSTANCE hInst, LPCWSTR name, LPCWSTR type)
{
	HRSRC hRsrc = FindResource(hInst, name, type);
	DWORD size = SizeofResource(hInst, hRsrc);
	LPVOID data = LockResource(LoadResource(hInst, hRsrc));
	return SDL_RWFromConstMem(data, size);
}

SDL_Surface* MainGame::loadSurface(int id)
{
	SDL_RWops* src = getResource(hInstance, MAKEINTRESOURCE(id), TEXT("PNG"));
	SDL_Surface* originSurface = IMG_LoadPNG_RW(src);
	SDL_Surface* convertSurface = SDL_ConvertSurface(originSurface, image.format, NULL);
	SDL_FreeSurface(originSurface);
	SDL_FreeRW(src);
	return convertSurface;
}

void MainGame::initWindow()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	hInstance = GetModuleHandle(0);
	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	keyStatus = SDL_GetKeyboardState(NULL);
	SDL_GetWindowSize(window, &screen.w, &screen.h);
}

void MainGame::initData()
{
	enemy0Data = { ENEMY0_ID, ENEMY0_HP, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_SPEED, ENEMY0_INDEX_MAX, ENEMY0_APPEND_SCORE, P_ENEMY0 };
	enemy1Data = { ENEMY1_ID, ENEMY1_HP, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_SPEED, ENEMY1_INDEX_MAX, ENEMY1_APPEND_SCORE, P_ENEMY1 };
	enemy2Data = { ENEMY2_ID, ENEMY2_HP, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_SPEED, ENEMY2_INDEX_MAX, ENEMY2_APPEND_SCORE, P_ENEMY2 };

	heroBulletData = { HERO_BULLET_ID, HERO_BULLET_WIDTH, HERO_BULLET_HEIGHT, HERO_BULLET_SPEED, HERO_BULLET_DAMAGE };
	enemy1BulletData = { ENEMY1_BULLET_ID, ENEMY1_BULLET_WIDTH, ENEMY1_BULLET_HEIGHT, ENEMY1_BULLET_SPEED, ENEMY1_BULLET_DAMAGE };
	enemy2BulletData = { ENEMY2_BULLET_ID, ENEMY2_BULLET_WIDTH, ENEMY2_BULLET_HEIGHT, ENEMY2_BULLET_SPEED, ENEMY2_BULLET_DAMAGE };
}

void MainGame::initColor()
{
	color.black = COLOR_BLACK;
	color.red = COLOR_RED;
}

void MainGame::loadImage()
{
	image.format = SDL_AllocFormat(IMG_FORMAT);
	image.surface = SDL_GetWindowSurface(window);
	image.background = loadSurface(IDB_PNG1);
	image.heroBullet = loadSurface(IDB_PNG34);
	image.enemy1Bullet = loadSurface(IDB_PNG32);
	image.enemy2Bullet = loadSurface(IDB_PNG33);

	for (int i = 0; i < HERO_INDEX_MAX; i++) { image.hero[i] = loadSurface(IDB_PNG25 + i); }
	for (int i = 0; i < ENEMY0_INDEX_MAX; i++) { image.enemy0[i] = loadSurface(IDB_PNG2 + i); }
	for (int i = 0; i < ENEMY1_INDEX_MAX; i++) { image.enemy1[i] = loadSurface(IDB_PNG8 + i); }
	for (int i = 0; i < ENEMY2_INDEX_MAX; i++) { image.enemy2[i] = loadSurface(IDB_PNG15 + i); }
}

void MainGame::loadFonts()
{
	TTF_Init();
	font.title = TTF_OpenFontRW(getResource(hInstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, TITLE_FONT_SIZE);
	font.info = TTF_OpenFontRW(getResource(hInstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, INFO_FONT_SIZE);
}

Uint32 planeAnimateCallback(Uint32 interval, void* param)
{
	static Uint32 tick = interval;

	if (game.status == PLAYING)
	{
		if (tick % HERO_CHANGE_INTERVAL == 0) { game.hero.animate(); }
		if (tick % ENEMY2_CHANGE_INTERVAL == 0) { for (auto it = game.enemy2.begin(); it != game.enemy2.end(); ++it) { it->animate(); } }
	}
	tick += interval;
	return interval;
}

Uint32 planeFireCallback(Uint32 interval, void* param)
{
	static Uint32 tick = interval;

	if (game.status == PLAYING)
	{
		if (tick % HERO_FIRE_INTERVAL == 0) { game.hero.fire(); }
		if (tick % ENEMY1_FIRE_INTERVAL == 0) { for (auto it = game.enemy1.begin(); it != game.enemy1.end(); ++it) { it->fire(); } }
		if (tick % ENEMY2_FIRE_INTERVAL == 0) { for (auto it = game.enemy2.begin(); it != game.enemy2.end(); ++it) { it->fire(); } }
	}
	tick += interval;
	return interval;
}

Uint32 planeDownCallback(Uint32 interval, void* param)
{
	if (game.status == PLAYING)
	{
		game.hero.down();
		for (auto it = game.enemy0.begin(); it != game.enemy0.end(); ++it) { it->down(); }
		for (auto it = game.enemy1.begin(); it != game.enemy1.end(); ++it) { it->down(); }
		for (auto it = game.enemy2.begin(); it != game.enemy2.end(); ++it) { it->down(); }
	}
	return interval;
}

Uint32 addAliveScoreCallback(Uint32 interval, void* param)
{
	if (game.hero.isAlive && game.status == PLAYING) { game.score += ALIVE_SCORE; }
	return interval;
}

void MainGame::addTimer()
{
	timer.planeAnimate = SDL_AddTimer(PLANE_ANIMATE_INTERVAL, planeAnimateCallback, NULL);
	timer.planeFire = SDL_AddTimer(PLANE_FIRE_INTERVAL, planeFireCallback, NULL);
	timer.planeDown = SDL_AddTimer(PLANE_DOWN_INTERVAL, planeDownCallback, NULL);
	timer.addAliveScore = SDL_AddTimer(ALIVE_SCORE_INTERVAL, addAliveScoreCallback, NULL);
}

void MainGame::freeImage()
{
	SDL_FreeFormat(image.format);
	SDL_FreeSurface(image.background);
	SDL_FreeSurface(image.heroBullet);
	SDL_FreeSurface(image.enemy1Bullet);
	SDL_FreeSurface(image.enemy2Bullet);

	for (int i = 0; i < HERO_INDEX_MAX; i++) { SDL_FreeSurface(image.hero[i]); }
	for (int i = 0; i < ENEMY0_INDEX_MAX; i++) { SDL_FreeSurface(image.enemy0[i]); }
	for (int i = 0; i < ENEMY1_INDEX_MAX; i++) { SDL_FreeSurface(image.enemy1[i]); }
	for (int i = 0; i < ENEMY2_INDEX_MAX; i++) { SDL_FreeSurface(image.enemy2[i]); }
}

void MainGame::freeFont()
{
	TTF_CloseFont(font.title);
	TTF_CloseFont(font.info);
}

void MainGame::removeTimer()
{
	SDL_RemoveTimer(timer.planeAnimate);
	SDL_RemoveTimer(timer.planeFire);
	SDL_RemoveTimer(timer.planeDown);
	SDL_RemoveTimer(timer.addAliveScore);
}

void MainGame::close()
{
	SDL_DestroyWindow(window);
	removeTimer();
	freeFont();
	freeImage();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

void MainGame::initGame()
{
	score = 0;
	hero.init();
	enemy0.clear();
	enemy1.clear();
	enemy2.clear();
	heroBullet.clear();
	enemy1Bullet.clear();
	enemy2Bullet.clear();
}

void MainGame::addEnemy(list <Enemy>& enemy, EnemyData& data)
{
	if (score >= data.appendScore)
	{
		if (fmod(rand(), RAND_MOD) / RAND_MOD < data.probability)
		{
			int rangeMax = SCREEN_WIDTH - BORDER_X - data.width;
			int rangeMin = BORDER_X;

			enemy.push_back(Enemy({ rand() % (rangeMax - rangeMin) + rangeMin, -data.height }, data));
		}
	}
}

void MainGame::updateEnemy(list <Enemy>& enemy, EnemyData& data)
{
	for (auto it = enemy.begin(); it != enemy.end();)
	{
		it->move();
		it->miss();

		(it->index == data.indexMax) ? (it = enemy.erase(it)) : (++it);
	}
}

void MainGame::updateBullet(list <Bullet>& bullet)
{
	for (auto it = bullet.begin(); it != bullet.end();)
	{
		it->move();
		it->miss();
		it->hitHero();

		(it->isAlive) ? (++it) : (it = bullet.erase(it));
	}
}

void MainGame::update()
{
	if (status == PLAYING)
	{
		addEnemy(enemy0, enemy0Data);
		addEnemy(enemy1, enemy1Data);
		addEnemy(enemy2, enemy2Data);

		hero.crash(enemy0, ENEMY0_SCORE);
		hero.crash(enemy1, ENEMY1_SCORE);
		hero.crash(enemy2, ENEMY2_SCORE);

		updateEnemy(enemy0, enemy0Data);
		updateEnemy(enemy1, enemy1Data);
		updateEnemy(enemy2, enemy2Data);

		updateBullet(enemy1Bullet);
		updateBullet(enemy2Bullet);

		for (auto it = heroBullet.begin(); it != heroBullet.end();)
		{
			it->move();
			it->miss();
			it->hitEnemy(enemy0, ENEMY0_SCORE);
			it->hitEnemy(enemy1, ENEMY1_SCORE);
			it->hitEnemy(enemy2, ENEMY2_SCORE);

			(it->isAlive) ? (++it) : (it = heroBullet.erase(it));
		}
	}
	if (status == END && score > bestScore) { bestScore = score; }
}

void MainGame::events()
{
	if (status == PLAYING && hero.isAlive) { hero.move(); }
	if (hero.index == HERO_INDEX_MAX) { status = END; }

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) { status = EXIT; }
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (status == END)
			{
				initGame();
				status = PLAYING;
			}
			else if (status == START || status == PAUSE) { status = PLAYING; }
		}
		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p && status == PLAYING) { status = PAUSE; }
		if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_b && status == PLAYING && hero.bombCount > 0)
		{
			hero.releaseBomb(enemy0, ENEMY0_SCORE);
			hero.releaseBomb(enemy1, ENEMY1_SCORE);
			hero.releaseBomb(enemy2, ENEMY2_SCORE);
			hero.bombCount -= 1;
		}
	}
}

void MainGame::overflowBlit(SDL_Surface* surface, SDL_Rect& rect)
{
	static SDL_Rect selfRect;
	static SDL_Rect desRect;

	if (rect.y < 0)
	{
		selfRect = { 0, -rect.y, rect.w, rect.h };
		desRect = { rect.x, 0, rect.w, rect.h + rect.y };
		SDL_BlitSurface(surface, &selfRect, image.surface, &desRect);
	}
	else { SDL_BlitSurface(surface, NULL, image.surface, &rect); }
}

void MainGame::displayText(const char* text, TTF_Font* font, Point pos, SDL_Color color)
{
	static SDL_Surface* textSurface;
	static SDL_Rect textRect;

	textSurface = TTF_RenderText_Blended(font, text, color);
	textRect.x = pos.x;
	textRect.y = pos.y;

	SDL_BlitSurface(textSurface, NULL, image.surface, &textRect);
	SDL_FreeSurface(textSurface);
}

void MainGame::displayBackground()
{
	static SDL_Rect selfRect;
	static SDL_Rect desRect;
	static int backgroundScrollPos;

	if (status == PLAYING)
	{
		if (backgroundScrollPos == SCREEN_HEIGHT) { backgroundScrollPos = 0; }
		backgroundScrollPos += BACKGROUND_SCROLL_SPEED;
	}
	selfRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - backgroundScrollPos };
	desRect = { 0, backgroundScrollPos, SCREEN_WIDTH, SCREEN_HEIGHT - backgroundScrollPos };
	SDL_BlitSurface(image.background, &selfRect, image.surface, &desRect);

	selfRect = { 0, SCREEN_HEIGHT - backgroundScrollPos, SCREEN_WIDTH, backgroundScrollPos };
	desRect = { 0, 0, SCREEN_WIDTH, backgroundScrollPos };
	SDL_BlitSurface(image.background, &selfRect, image.surface, &desRect);
}

void MainGame::displayPlane()
{
	if (status == PLAYING)
	{
		hero.display();
		for (auto it = enemy0.begin(); it != enemy0.end(); ++it) { it->display(); }
		for (auto it = enemy1.begin(); it != enemy1.end(); ++it) { it->display(); }
		for (auto it = enemy2.begin(); it != enemy2.end(); ++it) { it->display(); }
		for (auto it = heroBullet.begin(); it != heroBullet.end(); ++it) { it->display(); }
		for (auto it = enemy1Bullet.begin(); it != enemy1Bullet.end(); ++it) { it->display(); }
		for (auto it = enemy2Bullet.begin(); it != enemy2Bullet.end(); ++it) { it->display(); }
	}
}

void MainGame::displayInfo()
{
	static char text[INFO_MAX_LEN];

	if (status == START)
	{
		displayText("Welcome to PlaneWar", font.title, { SCREEN_WIDTH / 2 - TITLE_WELCOME_LENGTH, TITLE_POSITION }, color.black);
		displayText("Click anywhere to START...", font.info, { SCREEN_WIDTH / 2 - INFO_LENGTH, INFO_POSITION }, color.black);
	}
	else if (status == PLAYING)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "score: %d", score);
		displayText(text, font.info, { BORDER_TEXT, SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE) }, color.black);
		SDL_snprintf(text, INFO_MAX_LEN, "HP: %d%%", hero.health);
		displayText(text, font.info, { SCREEN_WIDTH - (HP_LENGTH + BORDER_TEXT), SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE) }, ((hero.health > HERO_HP_ALERT) ? color.black : color.red));
		SDL_snprintf(text, INFO_MAX_LEN, "BOMB: %d", hero.bombCount);
		displayText(text, font.info, { BORDER_TEXT, BORDER_TEXT }, color.black);
	}
	else if (status == PAUSE)
	{
		displayText("PAUSE", font.title, { SCREEN_WIDTH / 2 - TITLE_PAUSE_LENGTH, TITLE_POSITION }, color.black);
		displayText("Click anywhere to RESUME...", font.info, { SCREEN_WIDTH / 2 - INFO_LENGTH, INFO_POSITION }, color.black);
	}
	else if (status == END)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "Your score: %d", score);
		displayText(text, font.info, { SCREEN_WIDTH / 2 - SCORE_LENGTH, SCORE_POSITION }, color.black);
		SDL_snprintf(text, INFO_MAX_LEN, "Best score: %d", bestScore);
		displayText(text, font.info, { SCREEN_WIDTH / 2 - SCORE_LENGTH, BEST_SCORE_POSITION }, color.black);
		displayText("Gameover!", font.title, { SCREEN_WIDTH / 2 - TITLE_OVER_LENGTH, TITLE_POSITION }, color.black);
		displayText("Click anywhere to RESTART...", font.info, { SCREEN_WIDTH / 2 - INFO_LENGTH, INFO_POSITION }, color.black);
	}
}

void MainGame::display()
{
	displayBackground();
	displayPlane();
	displayInfo();
	SDL_UpdateWindowSurface(window);
}