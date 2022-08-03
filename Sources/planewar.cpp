#include "planewar.h"

using namespace std;

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

void MainGame::initColor()
{
	color.black = BLACK;
	color.red = RED;
}

void MainGame::loadImage()
{
	image.format = SDL_AllocFormat(IMG_FORMAT);
	image.surface = SDL_GetWindowSurface(window);
	image.background = loadSurface(IDB_PNG1);
	image.heroBullet = loadSurface(IDB_PNG34);
	image.enemy1Bullet = loadSurface(IDB_PNG32);
	image.enemy2Bullet = loadSurface(IDB_PNG33);

	for (int i = 0; i < HERO_IMG_MAX; i++) { image.hero[i] = loadSurface(IDB_PNG25 + i); }
	for (int i = 0; i < ENEMY0_IMG_MAX; i++) { image.enemy0[i] = loadSurface(IDB_PNG2 + i); }
	for (int i = 0; i < ENEMY1_IMG_MAX; i++) { image.enemy1[i] = loadSurface(IDB_PNG8 + i); }
	for (int i = 0; i < ENEMY2_IMG_MAX; i++) { image.enemy2[i] = loadSurface(IDB_PNG15 + i); }
}

void MainGame::loadFonts()
{
	TTF_Init();
	font.title = TTF_OpenFontRW(getResource(hInstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, TITLE_FONT_SIZE);
	font.info = TTF_OpenFontRW(getResource(hInstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, INFO_FONT_SIZE);
}

Uint32 planeChangeCallback(Uint32 interval, void* param)
{
	static Uint32 tick = interval;

	if (tick % HERO_CHANGE_INTERVAL == 0) { game.hero.changeAppearance(); }
	if (tick % ENEMY2_CHANGE_INTERVAL == 0) { for (int i = 0; i < game.enemy2.size(); i++) { game.enemy2[i].changeAppearance(); } }

	tick += interval;
	return interval;
}

Uint32 planeFireCallback(Uint32 interval, void* param)
{
	static Uint32 tick = interval;

	if (tick % HERO_FIRE_INTERVAL == 0) { game.hero.fire(); }
	if (tick % ENEMY1_FIRE_INTERVAL == 0) { for (int i = 0; i < game.enemy1.size(); i++) { game.enemy1[i].fire(); } }
	if (tick % ENEMY2_FIRE_INTERVAL == 0) { for (int i = 0; i < game.enemy2.size(); i++) { game.enemy2[i].fire(); } }

	tick += interval;
	return interval;
}

Uint32 planeDownCallback(Uint32 interval, void* param)
{
	game.hero.down(HERO_STATUS_MAX);
	for (int i = 0; i < game.enemy0.size(); i++) { game.enemy0[i].down(ENEMY0_STATUS_MAX); }
	for (int i = 0; i < game.enemy1.size(); i++) { game.enemy1[i].down(ENEMY1_STATUS_MAX); }
	for (int i = 0; i < game.enemy2.size(); i++) { game.enemy2[i].down(ENEMY2_STATUS_MAX); }
	return interval;
}

Uint32 aliveScoreCallback(Uint32 interval, void* param)
{
	if (game.hero.status == ALIVE_STATUS && game.status == PLAYING) { game.score += ALIVE_SCORE; }
	return interval;
}

void MainGame::addTimer()
{
	timer.planeChange = SDL_AddTimer(PLANE_CHANGE_INTERVAL, planeChangeCallback, NULL);
	timer.planeDown = SDL_AddTimer(PLANE_DOWN_INTERVAL, planeDownCallback, NULL);
	timer.aliveScore = SDL_AddTimer(ALIVE_SCORE_INTERVAL, aliveScoreCallback, NULL);
	timer.planeFire = SDL_AddTimer(PLANE_FIRE_INTERVAL, planeFireCallback, NULL);
}

void MainGame::freeImage()
{
	SDL_FreeFormat(image.format);
	SDL_FreeSurface(image.background);
	SDL_FreeSurface(image.heroBullet);
	SDL_FreeSurface(image.enemy1Bullet);
	SDL_FreeSurface(image.enemy2Bullet);

	for (int i = 0; i < HERO_IMG_MAX; i++) { SDL_FreeSurface(image.hero[i]); }
	for (int i = 0; i < ENEMY0_IMG_MAX; i++) { SDL_FreeSurface(image.enemy0[i]); }
	for (int i = 0; i < ENEMY1_IMG_MAX; i++) { SDL_FreeSurface(image.enemy1[i]); }
	for (int i = 0; i < ENEMY2_IMG_MAX; i++) { SDL_FreeSurface(image.enemy2[i]); }
}

void MainGame::freeFont()
{
	TTF_CloseFont(font.title);
	TTF_CloseFont(font.info);
}

void MainGame::removeTimer()
{
	SDL_RemoveTimer(timer.planeChange);
	SDL_RemoveTimer(timer.planeFire);
	SDL_RemoveTimer(timer.planeDown);
	SDL_RemoveTimer(timer.aliveScore);
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

void MainGame::addEnemy(vector <Enemy>& enemy, double appendP, int id, int width, int height, int health, int appendScore)
{
	if (score >= appendScore)
	{
		if ((rand() % (int)1E4) / 1E4 < appendP)
		{
			int x = rand() % (SCREEN_WIDTH - width - 2 * BORDER_X) + BORDER_X;
			enemy.push_back(Enemy(x, -height, id, width, height, health));
		}
	}
}

void MainGame::updateEnemy(vector <Enemy>& enemy, int speed, int statusMax)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		enemy[i].move(speed);
		enemy[i].miss(statusMax);
		if (enemy[i].status == statusMax) { enemy.erase(enemy.begin() + i--); }
	}
}

void MainGame::updateBullet(vector <Bullet>& bullet, int speed, int damage)
{
	for (int i = 0; i < bullet.size(); i++)
	{
		bullet[i].move(speed);
		bullet[i].miss();
		bullet[i].hit(damage);
		if (bullet[i].status == DOWN_STATUS) { bullet.erase(bullet.begin() + i--); }
	}
}

void MainGame::update()
{
	if (status == PLAYING)
	{
		addEnemy(enemy0, P_ENEMY0, ENEMY0_ID, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_HP, ENEMY0_APPEND_SCORE);
		addEnemy(enemy1, P_ENEMY1, ENEMY1_ID, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_HP, ENEMY1_APPEND_SCORE);
		addEnemy(enemy2, P_ENEMY2, ENEMY2_ID, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_HP, ENEMY2_APPEND_SCORE);

		hero.crash(enemy0, ENEMY0_SCORE);
		hero.crash(enemy1, ENEMY1_SCORE);
		hero.crash(enemy2, ENEMY2_SCORE);

		updateEnemy(enemy0, ENEMY0_SPEED, ENEMY0_STATUS_MAX);
		updateEnemy(enemy1, ENEMY1_SPEED, ENEMY1_STATUS_MAX);
		updateEnemy(enemy2, ENEMY2_SPEED, ENEMY2_STATUS_MAX);

		updateBullet(enemy1Bullet, ENEMY1_BULLET_SPEED, ENEMY1_BULLET_DAMAGE);
		updateBullet(enemy2Bullet, ENEMY2_BULLET_SPEED, ENEMY2_BULLET_DAMAGE);

		for (int i = 0; i < heroBullet.size(); i++)
		{
			heroBullet[i].move(HERO_BULLET_SPEED);
			heroBullet[i].miss();
			heroBullet[i].hit(enemy0, ENEMY0_SCORE);
			heroBullet[i].hit(enemy1, ENEMY1_SCORE);
			heroBullet[i].hit(enemy2, ENEMY2_SCORE);
			if (heroBullet[i].status == DOWN_STATUS) { heroBullet.erase(heroBullet.begin() + i--); }
		}
	}
	if (status == END && score > bestScore) { bestScore = score; }
}

void MainGame::control()
{
	if (status == PLAYING && hero.status == ALIVE_STATUS) { hero.move(); }
	if (hero.status == HERO_STATUS_MAX) { status = END; }

	while (SDL_PollEvent(&events))
	{
		if (events.type == SDL_QUIT) { status = EXIT; }
		if (events.type == SDL_MOUSEBUTTONDOWN)
		{
			if (status == START || status == PAUSE) { status = PLAYING; }
			else if (status == END)
			{
				initGame();
				status = PLAYING;
			}
		}
		if (events.type == SDL_KEYDOWN && events.key.keysym.sym == SDLK_p && status == PLAYING) { status = PAUSE; }
		if (events.type == SDL_KEYUP && events.key.keysym.sym == SDLK_b && status == PLAYING && hero.bombCount > 0)
		{
			hero.releaseBomb(enemy0, ENEMY0_SCORE);
			hero.releaseBomb(enemy1, ENEMY1_SCORE);
			hero.releaseBomb(enemy2, ENEMY2_SCORE);
			hero.bombCount -= 1;
		}
	}
}

void MainGame::overflowBlit(SDL_Surface* surface, SDL_Rect rect)
{
	SDL_Rect part = { 0, -rect.y, rect.w, rect.h };
	SDL_Rect dst = { rect.x, 0, rect.w, rect.h + rect.y };
	SDL_BlitSurface(surface, &part, image.surface, &dst);
}

void MainGame::displayText(const char* text, TTF_Font* font, int x, int y, SDL_Color color)
{
	static SDL_Surface* textSurface = nullptr;
	static SDL_Rect textRect = SDL_Rect();

	textSurface = TTF_RenderText_Blended(font, text, color);
	textRect.x = x;
	textRect.y = y;

	SDL_BlitSurface(textSurface, NULL, image.surface, &textRect);
	SDL_FreeSurface(textSurface);
}

void MainGame::displayBackground()
{
	static SDL_Rect selfRect;
	static SDL_Rect dstRect;

	if (status == PLAYING)
	{
		if (backgroundY == SCREEN_HEIGHT) { backgroundY = 0; }
		backgroundY += BACKGROUND_SCROLL_SPEED;
	}
	selfRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - backgroundY };
	dstRect = { 0, backgroundY, SCREEN_WIDTH, SCREEN_HEIGHT - backgroundY };
	SDL_BlitSurface(image.background, &selfRect, image.surface, &dstRect);

	selfRect = { 0, SCREEN_HEIGHT - backgroundY, SCREEN_WIDTH, backgroundY };
	dstRect = { 0, 0, SCREEN_WIDTH, backgroundY };
	SDL_BlitSurface(image.background, &selfRect, image.surface, &dstRect);
}

void MainGame::displayPlane()
{
	if (status == PLAYING)
	{
		hero.display();
		for (int i = 0; i < enemy0.size(); i++) { enemy0[i].display(ENEMY0_STATUS_MAX); }
		for (int i = 0; i < enemy1.size(); i++) { enemy1[i].display(ENEMY1_STATUS_MAX); }
		for (int i = 0; i < enemy2.size(); i++) { enemy2[i].display(ENEMY2_STATUS_MAX); }
		for (int i = 0; i < heroBullet.size(); i++) { heroBullet[i].display(); }
		for (int i = 0; i < enemy1Bullet.size(); i++) { enemy1Bullet[i].display(); }
		for (int i = 0; i < enemy2Bullet.size(); i++) { enemy2Bullet[i].display(); }
	}
}

void MainGame::displayInfo()
{
	static char text[INFO_MAX_LEN];

	if (status == START)
	{
		displayText("Welcome to PlaneWar", font.title, SCREEN_WIDTH / 2 - 125, TITLE_POSITION, color.black);
		displayText("Click anywhere to START...", font.info, SCREEN_WIDTH / 2 - 110, INFO_POSITION, color.black);
	}
	else if (status == PLAYING)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "score: %d", score);
		displayText(text, font.info, BORDER_TEXT, SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), color.black);
		SDL_snprintf(text, INFO_MAX_LEN, "HP: %d%%", hero.hp);
		displayText(text, font.info, SCREEN_WIDTH - (80 + BORDER_TEXT), SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), ((hero.hp > 30) ? color.black : color.red));
		SDL_snprintf(text, INFO_MAX_LEN, "BOMB: %d", hero.bombCount);
		displayText(text, font.info, BORDER_TEXT, BORDER_TEXT, color.black);
	}
	else if (status == PAUSE)
	{
		displayText("PAUSE", font.title, SCREEN_WIDTH / 2 - 40, TITLE_POSITION, color.black);
		displayText("Click anywhere to RESUME...", font.info, SCREEN_WIDTH / 2 - 110, INFO_POSITION, color.black);
	}
	else if (status == END)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "Your score: %d", score);
		displayText(text, font.info, SCREEN_WIDTH / 2 - 70, SCORE_POSITION, color.black);
		SDL_snprintf(text, INFO_MAX_LEN, "Best score: %d", bestScore);
		displayText(text, font.info, SCREEN_WIDTH / 2 - 70, BEST_SCORE_POSITION, color.black);
		displayText("Gameover!", font.title, SCREEN_WIDTH / 2 - 60, TITLE_POSITION, color.black);
		displayText("Click anywhere to RESTART...", font.info, SCREEN_WIDTH / 2 - 110, INFO_POSITION, color.black);
	}
}

void MainGame::display()
{
	displayBackground();
	displayPlane();
	displayInfo();
	SDL_UpdateWindowSurface(window);
}