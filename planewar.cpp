#include "planewar.h"

using namespace std;

SDL_RWops* Window::getResource(HINSTANCE hInst, LPCWSTR name, LPCWSTR type)
{
	HRSRC hRsrc = FindResource(hInst, name, type);
	DWORD size = SizeofResource(hInst, hRsrc);
	LPVOID data = LockResource(LoadResource(hInst, hRsrc));
	return SDL_RWFromConstMem(data, size);
}

SDL_Surface* Window::loadSurface(DWORD ID)
{
	SDL_RWops* src = getResource(hInstance, MAKEINTRESOURCE(ID), TEXT("PNG"));
	SDL_Surface* originImage = IMG_LoadPNG_RW(src);
	SDL_Surface* convertImage = SDL_ConvertSurface(originImage, format, NULL);
	SDL_FreeSurface(originImage);
	SDL_FreeRW(src);
	return convertImage;
}

void Window::overflowBlit(SDL_Surface* image, SDL_Rect rect)
{
	SDL_Rect part = { 0, -rect.y, rect.w, rect.h };
	SDL_Rect dst = { rect.x, 0, rect.w, rect.h + rect.y };
	SDL_BlitSurface(image, &part, surface, &dst);
}

void Window::text(const char* text, TTF_Font* type, int x, int y, SDL_Color color)
{
	SDL_Surface* textSurface = TTF_RenderText_Blended(type, text, color);
	SDL_Rect textRect = { x, y, TEXT_RECT_WIDTH, TEXT_RECT_HEIGHT };
	SDL_BlitSurface(textSurface, NULL, surface, &textRect);
	SDL_FreeSurface(textSurface);
}

void Window::init()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	hInstance = GetModuleHandle(0);
	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	format = SDL_AllocFormat(IMG_FORMAT);
	srceenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	keyStatus = SDL_GetKeyboardState(NULL);
}

void Window::initColor()
{
	black = BLACK;
	red = RED;
}

void Window::loadImage()
{
	backgroundImg = loadSurface(IDB_PNG1);
	heroBulletImg = loadSurface(IDB_PNG34);
	enemy1BulletImg = loadSurface(IDB_PNG32);
	enemy2BulletImg = loadSurface(IDB_PNG33);

	for (int i = 0; i < HERO_IMG_MAX; i++) { heroImg[i] = loadSurface(IDB_PNG25 + i); }
	for (int i = 0; i < ENEMY0_IMG_MAX; i++) { enemy0Img[i] = loadSurface(IDB_PNG2 + i); }
	for (int i = 0; i < ENEMY1_IMG_MAX; i++) { enemy1Img[i] = loadSurface(IDB_PNG8 + i); }
	for (int i = 0; i < ENEMY2_IMG_MAX; i++) { enemy2Img[i] = loadSurface(IDB_PNG15 + i); }
}

void Window::loadFonts()
{
	TTF_Init();
	titleFont = TTF_OpenFontRW(getResource(hInstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, TITLE_FONT_SIZE);
	infoFont = TTF_OpenFontRW(getResource(hInstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, INFO_FONT_SIZE);
}

void Window::freeImage()
{
	SDL_FreeSurface(backgroundImg);
	SDL_FreeSurface(heroBulletImg);
	SDL_FreeSurface(enemy1BulletImg);
	SDL_FreeSurface(enemy2BulletImg);

	for (int i = 0; i < HERO_IMG_MAX; i++) { SDL_FreeSurface(heroImg[i]); }
	for (int i = 0; i < ENEMY0_IMG_MAX; i++) { SDL_FreeSurface(enemy0Img[i]); }
	for (int i = 0; i < ENEMY1_IMG_MAX; i++) { SDL_FreeSurface(enemy1Img[i]); }
	for (int i = 0; i < ENEMY2_IMG_MAX; i++) { SDL_FreeSurface(enemy2Img[i]); }
}

void Window::freeFont()
{
	TTF_CloseFont(titleFont);
	TTF_CloseFont(infoFont);
}

void Window::close()
{
	SDL_DestroyWindow(window);
	SDL_FreeFormat(format);
	game.removeTimer();
	freeFont();
	freeImage();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

Uint32 heroChangeFunction(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { hero.changeAppearance(); }
	return interval;
}

Uint32 enemy2ChangeFunction(Uint32 interval, void* param)
{
	for (int i = 0; i < enemy2.size() && game.status == PLAYING; i++) { enemy2[i].changeAppearance(); }
	return interval;
}

Uint32 heroFireFunction(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { hero.fire(); }
	return interval;
}

Uint32 enemy1FireFunction(Uint32 interval, void* param)
{
	for (int i = 0; i < enemy1.size() && game.status == PLAYING; i++) { enemy1[i].fire(); }
	return interval;
}

Uint32 enemy2FireFunction(Uint32 interval, void* param)
{
	for (int i = 0; i < enemy2.size() && game.status == PLAYING; i++) { enemy2[i].fire(); }
	return interval;
}
Uint32 aliveFunction(Uint32 interval, void* param)
{
	game.score += (hero.status == ALIVE_STATUS && game.status == PLAYING) ? ALIVE_SCORE : 0;
	return interval;
}

Uint32 aircraftDownFunction(Uint32 interval, void* param)
{
	if (game.status == PLAYING)
	{
		hero.down(HERO_STATUS_MAX);
		for (int i = 0; i < enemy0.size(); i++) { enemy0[i].down(ENEMY0_STATUS_MAX); }
		for (int i = 0; i < enemy1.size(); i++) { enemy1[i].down(ENEMY1_STATUS_MAX); }
		for (int i = 0; i < enemy2.size(); i++) { enemy2[i].down(ENEMY2_STATUS_MAX); }
	}
	return interval;
}

Game::Game() : random((unsigned)time(NULL)), randP(0.0, 1.0)
{
	status = START;
	bestScore = 0;
	backgroundY = 0;
}

void Game::init()
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

void Game::addTimer()
{
	heroChange = SDL_AddTimer(HERO_CHANGE_INTERVAL, heroChangeFunction, NULL);
	enemy2Change = SDL_AddTimer(ENEMY2_CHANGE_INTERVAL, enemy2ChangeFunction, NULL);
	heroFire = SDL_AddTimer(HERO_FIRE_INTERVAL, heroFireFunction, NULL);
	enemy1Fire = SDL_AddTimer(ENEMY1_FIRE_INTERVAL, enemy1FireFunction, NULL);
	enemy2Fire = SDL_AddTimer(ENEMY2_FIRE_INTERVAL, enemy2FireFunction, NULL);
	aircraftDown = SDL_AddTimer(AIRCRAFT_DOWN_INTERVAL, aircraftDownFunction, NULL);
	alive = SDL_AddTimer(ALIVE_INTERVAL, aliveFunction, NULL);
}

void Game::removeTimer()
{
	SDL_RemoveTimer(heroChange);
	SDL_RemoveTimer(heroFire);
	SDL_RemoveTimer(enemy1Fire);
	SDL_RemoveTimer(enemy2Change);
	SDL_RemoveTimer(enemy2Fire);
	SDL_RemoveTimer(aircraftDown);
	SDL_RemoveTimer(alive);
}

template <class Enemy> void Game::addEnemy(vector <Enemy>& enemy, double appendP, int width, int height, int appendScore)
{
	if (score >= appendScore)
	{
		if (randP(random) < appendP)
		{
			int enemyX = (int)(randP(random) * (SCREEN_WIDTH - width - 2 * BORDER_X) + BORDER_X);
			enemy.push_back(Enemy(enemyX, -height));
		}
	}
}

template <class Enemy> void Game::updateEnemy(vector <Enemy>& enemy, int speed, int statusMax)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		enemy[i].move(speed);
		enemy[i].miss(statusMax);
		if (enemy[i].status == statusMax) { enemy.erase(enemy.begin() + i--); }
	}
}

template <class Bullet> void Game::updateBullet(vector <Bullet>& bullet, int speed, int damage, int width, int height)
{
	for (int i = 0; i < bullet.size(); i++)
	{
		bullet[i].move(speed);
		bullet[i].miss();
		bullet[i].hit(damage, width, height);
		if (bullet[i].status == DOWN_STATUS) { bullet.erase(bullet.begin() + i--); }
	}
}

void Game::update()
{
	if (status == PLAYING)
	{
		addEnemy(enemy0, P_ENEMY0, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_APPEND_SCORE);
		addEnemy(enemy1, P_ENEMY1, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_APPEND_SCORE);
		addEnemy(enemy2, P_ENEMY2, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_APPEND_SCORE);

		hero.crash(enemy0, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_SCORE);
		hero.crash(enemy1, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_SCORE);
		hero.crash(enemy2, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_SCORE);

		updateEnemy(enemy0, ENEMY0_SPEED, ENEMY0_STATUS_MAX);
		updateEnemy(enemy1, ENEMY1_SPEED, ENEMY1_STATUS_MAX);
		updateEnemy(enemy2, ENEMY2_SPEED, ENEMY2_STATUS_MAX);

		updateBullet(enemy1Bullet, ENEMY1_BULLET_SPEED, ENEMY1_BULLET_DAMAGE, ENEMY1_BULLET_WIDTH, ENEMY1_BULLET_HEIGHT);
		updateBullet(enemy2Bullet, ENEMY2_BULLET_SPEED, ENEMY2_BULLET_DAMAGE, ENEMY2_BULLET_WIDTH, ENEMY2_BULLET_HEIGHT);

		for (int i = 0; i < heroBullet.size(); i++)
		{
			heroBullet[i].move();
			heroBullet[i].miss();
			heroBullet[i].hit(enemy0, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_SCORE);
			heroBullet[i].hit(enemy1, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_SCORE);
			heroBullet[i].hit(enemy2, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_SCORE);
			if (heroBullet[i].status == DOWN_STATUS) { heroBullet.erase(heroBullet.begin() + i--); }
		}
	}
	if (status == END && score > bestScore) { bestScore = score; }
}

void Game::events()
{
	if (status == PLAYING && hero.status == ALIVE_STATUS) { hero.move(); }
	if (hero.status == HERO_STATUS_MAX) { status = END; }

	while (SDL_PollEvent(&window.events))
	{
		if (window.events.type == SDL_QUIT) { status = EXIT; }
		if (window.events.type == SDL_MOUSEBUTTONDOWN)
		{
			if (status == START || status == PAUSE) { status = PLAYING; }
			else if (status == END)
			{
				init();
				status = PLAYING;
			}
		}
		if (window.events.type == SDL_KEYDOWN && window.events.key.keysym.sym == SDLK_p && status == PLAYING) { status = PAUSE; }
		if (window.events.type == SDL_KEYUP && window.events.key.keysym.sym == SDLK_b && status == PLAYING && hero.bombCount > 0)
		{
			hero.releaseBomb(enemy0, ENEMY0_SCORE);
			hero.releaseBomb(enemy1, ENEMY1_SCORE);
			hero.releaseBomb(enemy2, ENEMY2_SCORE);
			hero.bombCount -= 1;
		}
	}
}

void Game::displayBackground()
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
	SDL_BlitSurface(window.backgroundImg, &selfRect, window.surface, &dstRect);

	selfRect = { 0, SCREEN_HEIGHT - backgroundY, SCREEN_WIDTH, backgroundY };
	dstRect = { 0, 0, SCREEN_WIDTH, backgroundY };
	SDL_BlitSurface(window.backgroundImg, &selfRect, window.surface, &dstRect);
}

void Game::displayPlane()
{
	if (status == PLAYING)
	{
		hero.display();
		for (int i = 0; i < enemy0.size(); i++) { enemy0[i].display(); }
		for (int i = 0; i < enemy1.size(); i++) { enemy1[i].display(); }
		for (int i = 0; i < enemy2.size(); i++) { enemy2[i].display(); }
		for (int i = 0; i < heroBullet.size(); i++) { heroBullet[i].display(); }
		for (int i = 0; i < enemy1Bullet.size(); i++) { enemy1Bullet[i].display(window.enemy1BulletImg); }
		for (int i = 0; i < enemy2Bullet.size(); i++) { enemy2Bullet[i].display(window.enemy2BulletImg); }
	}
}

void Game::displayInfo()
{
	static char text[INFO_MAX_LEN];

	if (status == START)
	{
		window.text("Welcome to PlaneWar", window.titleFont, SCREEN_WIDTH / 2 - 125, TITLE_POSITION, window.black);
		window.text("Click anywhere to START...", window.infoFont, SCREEN_WIDTH / 2 - 110, INFO_POSITION, window.black);
	}
	else if (status == PLAYING)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "score: %d", score);
		window.text(text, window.infoFont, BORDER_TEXT, SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), window.black);
		SDL_snprintf(text, INFO_MAX_LEN, "HP: %d%%", hero.health);
		window.text(text, window.infoFont, SCREEN_WIDTH - (80 + BORDER_TEXT), SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), ((hero.health > 30) ? window.black : window.red));
		SDL_snprintf(text, INFO_MAX_LEN, "BOMB: %d", hero.bombCount);
		window.text(text, window.infoFont, BORDER_TEXT, BORDER_TEXT, window.black);
	}
	else if (status == PAUSE)
	{
		window.text("PAUSE", window.titleFont, SCREEN_WIDTH / 2 - 40, TITLE_POSITION, window.black);
		window.text("Click anywhere to RESUME...", window.infoFont, SCREEN_WIDTH / 2 - 110, INFO_POSITION, window.black);
	}
	else if (status == END)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "Your score: %d", score);
		window.text(text, window.infoFont, SCREEN_WIDTH / 2 - 70, SCORE_POSITION, window.black);
		SDL_snprintf(text, INFO_MAX_LEN, "Best score: %d", bestScore);
		window.text(text, window.infoFont, SCREEN_WIDTH / 2 - 70, BEST_SCORE_POSITION, window.black);
		window.text("Gameover!", window.titleFont, SCREEN_WIDTH / 2 - 60, TITLE_POSITION, window.black);
		window.text("Click anywhere to RESTART...", window.infoFont, SCREEN_WIDTH / 2 - 110, INFO_POSITION, window.black);
	}
}

void Game::display()
{
	displayBackground();
	displayPlane();
	displayInfo();
	SDL_UpdateWindowSurface(window.window);
}

void Aircraft::move(int speed) { rect.y += speed; }

void Aircraft::miss(int statusCode)
{
	if (rect.y > SCREEN_HEIGHT)
	{
		health = 0;
		status = statusCode;
	}
}

void Aircraft::down(int statusCode)
{
	if (health <= 0)
	{
		health = 0;
		status += (status < statusCode) ? 1 : 0;
	}
}

void Aircraft::changeAppearance()
{
	switch (appearance)
	{
		case APPEARANCE1: appearance = APPEARANCE2; break;
		case APPEARANCE2: appearance = APPEARANCE1; break;
	}
}

void Bullet::move(int speed) { rect.y += speed; }

void Bullet::miss() { status = (rect.y > SCREEN_HEIGHT) ? DOWN_STATUS : ALIVE_STATUS; }

void Bullet::display(SDL_Surface* image) { SDL_BlitSurface(image, NULL, window.surface, &rect); }

void Bullet::hit(int damage, int width, int height)
{
	int distanceX = SDL_abs((rect.x + width / 2) - (hero.rect.x + HERO_WIDTH / 2));
	int distanceY = SDL_abs((rect.y + height / 2) - (hero.rect.y + HERO_HEIGHT / 2));

	if (distanceX <= HERO_WIDTH / 2 - ENEMY_HIT_DEV && distanceY <= HERO_HEIGHT / 2 - ENEMY_HIT_DEV && hero.health > 0)
	{
		hero.health -= damage;
		status = DOWN_STATUS;
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
	if (window.keyStatus[SDL_SCANCODE_W] && rect.y >= BORDER_Y) { rect.y -= HERO_SPEED; }
	if (window.keyStatus[SDL_SCANCODE_S] && rect.y <= SCREEN_HEIGHT - HERO_HEIGHT - BORDER_Y) { rect.y += HERO_SPEED; }
	if (window.keyStatus[SDL_SCANCODE_A] && rect.x >= BORDER_X) { rect.x -= HERO_SPEED; }
	if (window.keyStatus[SDL_SCANCODE_D] && rect.x <= SCREEN_WIDTH - HERO_WIDTH - BORDER_X) { rect.x += HERO_SPEED; }
}

void Hero::fire()
{
	if (status == ALIVE_STATUS)
	{
		int bulletX = rect.x + HERO_WIDTH / 2 - HERO_BULLET_WIDTH / 2 + 1;
		int bulletY = rect.y - HERO_BULLET_HEIGHT;
		heroBullet.push_back(HeroBullet(bulletX, bulletY));
	}
}

template <class Enemy> void Hero::releaseBomb(vector <Enemy> &enemy, int score)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		enemy[i].health = 0;
		enemy[i].status = DOWN_STATUS;
		game.score += score;
	}
}

template <class Enemy> void Hero::crash(vector <Enemy> &enemy, int width, int height, int score)
{
	if (status == ALIVE_STATUS)
	{
		for (int i = 0; i < enemy.size(); i++)
		{
			int distanceX = SDL_abs((rect.x + HERO_WIDTH / 2) - (enemy[i].rect.x + width / 2));
			int distanceY = SDL_abs((rect.y + HERO_HEIGHT / 2) - (enemy[i].rect.y + height / 2));

			if (distanceX <= (HERO_WIDTH + width) / 2 - CRASH_DEV && distanceY <= (HERO_HEIGHT + height) / 2 - CRASH_DEV && enemy[i].status == ALIVE_STATUS)
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
		if (status == ALIVE_STATUS)
		{
			if (appearance == APPEARANCE1) { window.temp = window.heroImg[0]; }
			else if (appearance = APPEARANCE2) { window.temp = window.heroImg[1]; }
		}
		else { window.temp = window.heroImg[status + 1]; }
		SDL_BlitSurface(window.temp, NULL, window.surface, &rect);
	}
}

Enemy0::Enemy0(int enemyX, int enemyY)
{
	rect = { enemyX, enemyY, ENEMY0_WIDTH, ENEMY0_HEIGHT };
	health = ENEMY0_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
}

void Enemy0::display()
{
	if (status < ENEMY0_STATUS_MAX)
	{
		if (rect.y < 0) { window.overflowBlit(window.enemy0Img[status], rect); }
		else { SDL_BlitSurface(window.enemy0Img[status], NULL, window.surface, &rect); }
	}
}

Enemy1::Enemy1(int enemyX, int enemyY)
{
	rect = { enemyX, enemyY, ENEMY1_WIDTH, ENEMY1_HEIGHT };
	health = ENEMY1_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
}

void Enemy1::fire()
{
	if (status == ALIVE_STATUS)
	{
		int bulletX = rect.x + ENEMY1_WIDTH / 2 - ENEMY1_BULLET_WIDTH / 2 + 1;
		int bulletY = rect.y + ENEMY1_HEIGHT;
		enemy1Bullet.push_back(Enemy1Bullet(bulletX, bulletY));
	}
}

void Enemy1::display()
{
	if (status < ENEMY1_STATUS_MAX)
	{
		if (status == ALIVE_STATUS) { window.temp = window.enemy1Img[int(health <= ENEMY1_HP / 2)]; }
		else { window.temp = window.enemy1Img[status + 1]; }

		if (rect.y < 0) { window.overflowBlit(window.temp, rect); }
		else { SDL_BlitSurface(window.temp, NULL, window.surface, &rect); }
	}
}

Enemy2::Enemy2(int enemyX, int enemyY)
{
	rect = { enemyX, enemyY, ENEMY2_WIDTH, ENEMY2_HEIGHT };
	health = ENEMY2_HP;
	appearance = APPEARANCE1;
	status = ALIVE_STATUS;
}

void Enemy2::fire()
{
	if (status == ALIVE_STATUS && health > ENEMY2_HP / 2)
	{
		int bulletX = rect.x + ENEMY2_WIDTH / 2 - ENEMY2_BULLET_WIDTH / 2 + 1;
		int bulletY = rect.y + ENEMY2_HEIGHT;
		enemy2Bullet.push_back(Enemy2Bullet(bulletX, bulletY));
	}
}

void Enemy2::display()
{
	if (status < ENEMY2_STATUS_MAX)
	{
		if (status == ALIVE_STATUS)
		{
			switch (bool(health > ENEMY2_HP / 2))
			{
				case true: window.temp = window.enemy2Img[appearance]; break;
				case false: window.temp = window.enemy2Img[2]; break;
			}
		}
		else { window.temp = window.enemy2Img[status + 2]; }

		if (rect.y < 0) { window.overflowBlit(window.temp, rect); }
		else { SDL_BlitSurface(window.temp, NULL, window.surface, &rect); }
	}
}

HeroBullet::HeroBullet(int bulletX, int bulletY)
{
	rect = { bulletX, bulletY, HERO_BULLET_WIDTH, HERO_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}

void HeroBullet::move() { rect.y -= HERO_BULLET_SPEED; }

void HeroBullet::miss() { status = (rect.y <= -HERO_BULLET_HEIGHT) ? DOWN_STATUS : ALIVE_STATUS; }

void HeroBullet::display()
{
	if (rect.y < 0) { window.overflowBlit(window.heroBulletImg, rect); }
	else { SDL_BlitSurface(window.heroBulletImg, NULL, window.surface, &rect); }
}

template <class Enemy> void HeroBullet::hit(vector <Enemy> &enemy, int width, int height, int score)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		int distanceX = SDL_abs((rect.x + HERO_BULLET_WIDTH / 2) - (enemy[i].rect.x + width / 2));
		int distanceY = SDL_abs((rect.y + HERO_BULLET_HEIGHT / 2) - (enemy[i].rect.y + height / 2));

		if (distanceX <= width / 2 - HERO_HIT_DEV && distanceY <= height / 2 - HERO_HIT_DEV && enemy[i].health > 0)
		{
			enemy[i].health -= HERO_BULLET_DAMAGE;
			status = DOWN_STATUS;
			game.score += (enemy[i].health <= 0) ? score : 0;
		}
	}
}

Enemy1Bullet::Enemy1Bullet(int bulletX, int bulletY)
{
	rect = { bulletX, bulletY, ENEMY1_BULLET_WIDTH, ENEMY1_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}

Enemy2Bullet::Enemy2Bullet(int bulletX, int bulletY)
{
	rect = { bulletX, bulletY, ENEMY2_BULLET_WIDTH, ENEMY2_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}