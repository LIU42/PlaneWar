#include "planewar.h"

using namespace std;

SDL_RWops* Window::get_resource(HINSTANCE hinst, LPCWSTR name, LPCWSTR type)
{
	HRSRC hrsrc = FindResource(hinst, name, type);
	DWORD size = SizeofResource(hinst, hrsrc);
	LPVOID data = LockResource(LoadResource(hinst, hrsrc));
	return SDL_RWFromConstMem(data, size);
}

SDL_Surface* Window::load_surface(DWORD ID)
{
	SDL_RWops* src = get_resource(hinstance, MAKEINTRESOURCE(ID), TEXT("PNG"));
	SDL_Surface* origin_image = IMG_LoadPNG_RW(src);
	SDL_Surface* convert_image = SDL_ConvertSurface(origin_image, format, NULL);
	SDL_FreeSurface(origin_image);
	SDL_FreeRW(src);
	return convert_image;
}

void Window::overflow_blit(SDL_Surface* image, SDL_Rect rect)
{
	SDL_Rect part = { 0, -rect.y, rect.w, rect.h };
	SDL_Rect dst = { rect.x, 0, rect.w, rect.h + rect.y };
	SDL_BlitSurface(image, &part, surface, &dst);
}

void Window::text(const char* text, TTF_Font* type, int x, int y, SDL_Color color)
{
	text_surface = TTF_RenderText_Blended(type, text, color);
	text_rect = { x,y,TEXT_RECT_WIDTH,TEXT_RECT_HEIGHT };
	SDL_BlitSurface(text_surface, NULL, surface, &text_rect);
	SDL_FreeSurface(text_surface);
}

void Window::init()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	hinstance = GetModuleHandle(0);
	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	format = SDL_AllocFormat(IMG_FORMAT);
	surface_rect = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };
	keystatus = SDL_GetKeyboardState(NULL);
}

void Window::init_color()
{
	black = BLACK;
	red = RED;
}

void Window::load_image()
{
	background = load_surface(IDB_PNG1);
	hero_bullet_img = load_surface(IDB_PNG34);
	enemy1_bullet_img = load_surface(IDB_PNG32);
	enemy2_bullet_img = load_surface(IDB_PNG33);

	for (int i = 0; i < HERO_IMG_MAX; i++) { hero_img[i] = load_surface(IDB_PNG25 + i); }
	for (int i = 0; i < ENEMY0_IMG_MAX; i++) { enemy0_img[i] = load_surface(IDB_PNG2 + i); }
	for (int i = 0; i < ENEMY1_IMG_MAX; i++) { enemy1_img[i] = load_surface(IDB_PNG8 + i); }
	for (int i = 0; i < ENEMY2_IMG_MAX; i++) { enemy2_img[i] = load_surface(IDB_PNG15 + i); }
}

void Window::load_fonts()
{
	TTF_Init();
	font_title = TTF_OpenFontRW(get_resource(hinstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, TITLE_FONT_SIZE);
	font_info = TTF_OpenFontRW(get_resource(hinstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, INFO_FONT_SIZE);
}

void Window::free_image()
{
	SDL_FreeSurface(background);
	SDL_FreeSurface(hero_bullet_img);
	SDL_FreeSurface(enemy1_bullet_img);
	SDL_FreeSurface(enemy2_bullet_img);

	for (int i = 0; i < HERO_IMG_MAX; i++) { SDL_FreeSurface(hero_img[i]); }
	for (int i = 0; i < ENEMY0_IMG_MAX; i++) { SDL_FreeSurface(enemy0_img[i]); }
	for (int i = 0; i < ENEMY1_IMG_MAX; i++) { SDL_FreeSurface(enemy1_img[i]); }
	for (int i = 0; i < ENEMY2_IMG_MAX; i++) { SDL_FreeSurface(enemy2_img[i]); }
}

void Window::free_font()
{
	TTF_CloseFont(font_title);
	TTF_CloseFont(font_info);
}

void Window::close()
{
	SDL_DestroyWindow(window);
	SDL_FreeFormat(format);
	game.remove_timer();
	free_font();
	free_image();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

Uint32 function_hero_change(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { hero.change_appearance(); }
	return interval;
}

Uint32 function_enemy2_change(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { for (int i = 0; i < enemy2.size(); i++) { enemy2[i].change_appearance(); } }
	return interval;
}

Uint32 function_hero_fire(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { hero.fire(); }
	return interval;
}

Uint32 function_enemy1_fire(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { for (int i = 0; i < enemy1.size(); i++) { enemy1[i].fire(); } }
	return interval;
}

Uint32 function_enemy2_fire(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { for (int i = 0; i < enemy2.size(); i++) { enemy2[i].fire(); } }
	return interval;
}
Uint32 function_alive(Uint32 interval, void* param)
{
	if (hero.status == ALIVE_STATUS && game.status == PLAYING) { game.score += ALIVE_SCORE; }
	return interval;
}

Uint32 function_aircraft_down(Uint32 interval, void* param)
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

Game::Game() : random((unsigned)time(NULL)), rand_p(0.0, 1.0)
{
	status = START;
	score_best = 0;
	background_position = 0;
}

void Game::init()
{
	score = 0;
	hero.init();
	enemy0.clear();
	enemy1.clear();
	enemy2.clear();
	hero_bullet.clear();
	enemy1_bullet.clear();
	enemy2_bullet.clear();
}

void Game::add_timer()
{
	hero_change = SDL_AddTimer(HERO_CHANGE_INTERVAL, function_hero_change, NULL);
	enemy2_change = SDL_AddTimer(ENEMY2_CHANGE_INTERVAL, function_enemy2_change, NULL);
	hero_fire = SDL_AddTimer(HERO_FIRE_INTERVAL, function_hero_fire, NULL);
	enemy1_fire = SDL_AddTimer(ENEMY1_FIRE_INTERVAL, function_enemy1_fire, NULL);
	enemy2_fire = SDL_AddTimer(ENEMY2_FIRE_INTERVAL, function_enemy2_fire, NULL);
	aircraft_down = SDL_AddTimer(AIRCRAFT_DOWN_INTERVAL, function_aircraft_down, NULL);
	alive = SDL_AddTimer(ALIVE_INTERVAL, function_alive, NULL);
}

void Game::remove_timer()
{
	SDL_RemoveTimer(hero_change);
	SDL_RemoveTimer(hero_fire);
	SDL_RemoveTimer(enemy1_fire);
	SDL_RemoveTimer(enemy2_change);
	SDL_RemoveTimer(enemy2_fire);
	SDL_RemoveTimer(aircraft_down);
	SDL_RemoveTimer(alive);
}

template <class Enemy>
void Game::add_enemy(vector <Enemy>& enemy, double p, int width, int height, int num)
{
	if (score >= num)
	{
		double n = rand_p(random);
		if (n < p)
		{
			int x = (int)(rand_p(random) * (SCREEN_WIDTH - width - 2 * BORDER_X) + BORDER_X);
			enemy.push_back(Enemy(x, -height));
		}
	}
}

void Game::update()
{
	if (status == PLAYING)
	{
		add_enemy(enemy0, P_ENEMY0, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_APPEND_SCORE);
		add_enemy(enemy1, P_ENEMY1, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_APPEND_SCORE);
		add_enemy(enemy2, P_ENEMY2, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_APPEND_SCORE);

		hero.crash(enemy0, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_SCORE);
		hero.crash(enemy1, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_SCORE);
		hero.crash(enemy2, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_SCORE);

		for (int i = 0; i < enemy0.size(); i++)
		{
			enemy0[i].move(ENEMY0_SPEED);
			enemy0[i].miss(ENEMY0_STATUS_MAX);
			if (enemy0[i].status == ENEMY0_STATUS_MAX) { enemy0.erase(enemy0.begin() + i--); }
		}
		for (int i = 0; i < enemy1.size(); i++)
		{
			enemy1[i].move(ENEMY1_SPEED);
			enemy1[i].miss(ENEMY1_STATUS_MAX);
			if (enemy1[i].status == ENEMY1_STATUS_MAX) { enemy1.erase(enemy1.begin() + i--); }
		}
		for (int i = 0; i < enemy2.size(); i++)
		{
			enemy2[i].move(ENEMY2_SPEED);
			enemy2[i].miss(ENEMY2_STATUS_MAX);
			if (enemy2[i].status == ENEMY2_STATUS_MAX) { enemy2.erase(enemy2.begin() + i--); }
		}
		for (int i = 0; i < hero_bullet.size(); i++)
		{
			hero_bullet[i].move();
			hero_bullet[i].miss();
			hero_bullet[i].hit(enemy0, ENEMY0_WIDTH, ENEMY0_HEIGHT, ENEMY0_SCORE);
			hero_bullet[i].hit(enemy1, ENEMY1_WIDTH, ENEMY1_HEIGHT, ENEMY1_SCORE);
			hero_bullet[i].hit(enemy2, ENEMY2_WIDTH, ENEMY2_HEIGHT, ENEMY2_SCORE);
			if (hero_bullet[i].status == DOWN_STATUS) { hero_bullet.erase(hero_bullet.begin() + i--); }
		}
		for (int i = 0; i < enemy1_bullet.size(); i++)
		{
			enemy1_bullet[i].move(ENEMY1_BULLET_SPEED);
			enemy1_bullet[i].miss();
			enemy1_bullet[i].hit(ENEMY1_BULLET_DAMAGE, ENEMY1_BULLET_WIDTH, ENEMY1_BULLET_HEIGHT);
			if (enemy1_bullet[i].status == DOWN_STATUS) { enemy1_bullet.erase(enemy1_bullet.begin() + i--); }
		}
		for (int i = 0; i < enemy2_bullet.size(); i++)
		{
			enemy2_bullet[i].move(ENEMY2_BULLET_SPEED);
			enemy2_bullet[i].miss();
			enemy2_bullet[i].hit(ENEMY2_BULLET_DAMAGE, ENEMY2_BULLET_WIDTH, ENEMY2_BULLET_HEIGHT);
			if (enemy2_bullet[i].status == DOWN_STATUS) { enemy2_bullet.erase(enemy2_bullet.begin() + i--); }
		}
	}
	if (status == END) { if (score > score_best) { score_best = score; } }
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
		if (window.events.type == SDL_KEYUP && window.events.key.keysym.sym == SDLK_b && status == PLAYING && hero.bomb_count > 0)
		{
			hero.release_bomb(enemy0, ENEMY0_SCORE);
			hero.release_bomb(enemy1, ENEMY1_SCORE);
			hero.release_bomb(enemy2, ENEMY2_SCORE);
			hero.bomb_count -= 1;
		}
	}
}

void Game::display_background()
{
	if (status == PLAYING)
	{
		if (background_position == SCREEN_HEIGHT) { background_position = 0; }
		background_position += BACKGROUND_SCROLL_SPEED;
	}
	window.background_rect_self = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - background_position };
	window.background_rect_dst = { 0, background_position, SCREEN_WIDTH, SCREEN_HEIGHT - background_position };
	SDL_BlitSurface(window.background, &window.background_rect_self, window.surface, &window.background_rect_dst);

	window.background_rect_self = { 0, SCREEN_HEIGHT - background_position, SCREEN_WIDTH, background_position };
	window.background_rect_dst = { 0, 0, SCREEN_WIDTH, background_position };
	SDL_BlitSurface(window.background, &window.background_rect_self, window.surface, &window.background_rect_dst);
}

void Game::display_plane()
{
	if (status == PLAYING)
	{
		hero.display();
		for (int i = 0; i < enemy0.size(); i++) { enemy0[i].display(); }
		for (int i = 0; i < enemy1.size(); i++) { enemy1[i].display(); }
		for (int i = 0; i < enemy2.size(); i++) { enemy2[i].display(); }
		for (int i = 0; i < hero_bullet.size(); i++) { hero_bullet[i].display(); }
		for (int i = 0; i < enemy1_bullet.size(); i++) { enemy1_bullet[i].display(window.enemy1_bullet_img); }
		for (int i = 0; i < enemy2_bullet.size(); i++) { enemy2_bullet[i].display(window.enemy2_bullet_img); }
	}
}

void Game::display_info()
{
	if (status == START)
	{
		window.text("Welcome to PlaneWar", window.font_title, SCREEN_WIDTH / 2 - 125, TITLE_POSITION, window.black);
		window.text("Click anywhere to START...", window.font_info, SCREEN_WIDTH / 2 - 110, INFO_POSITION, window.black);
	}
	else if (status == PLAYING)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "score: %d", score);
		window.text(text, window.font_info, BORDER_TEXT, SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), window.black);
		SDL_snprintf(text, INFO_MAX_LEN, "HP: %d%%", hero.hp);
		window.text(text, window.font_info, SCREEN_WIDTH - (80 + BORDER_TEXT), SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), ((hero.hp > 30) ? window.black : window.red));
		SDL_snprintf(text, INFO_MAX_LEN, "BOMB: %d", hero.bomb_count);
		window.text(text, window.font_info, BORDER_TEXT, BORDER_TEXT, window.black);
	}
	else if (status == PAUSE)
	{
		window.text("PAUSE", window.font_title, SCREEN_WIDTH / 2 - 40, TITLE_POSITION, window.black);
		window.text("Click anywhere to RESUME...", window.font_info, SCREEN_WIDTH / 2 - 110, INFO_POSITION, window.black);
	}
	else if (status == END)
	{
		SDL_snprintf(text, INFO_MAX_LEN, "Your score: %d", score);
		window.text(text, window.font_info, SCREEN_WIDTH / 2 - 70, SCORE_POSITION, window.black);
		SDL_snprintf(text, INFO_MAX_LEN, "Best score: %d", score_best);
		window.text(text, window.font_info, SCREEN_WIDTH / 2 - 70, BEST_SCORE_POSITION, window.black);
		window.text("Gameover!", window.font_title, SCREEN_WIDTH / 2 - 60, TITLE_POSITION, window.black);
		window.text("Click anywhere to RESTART...", window.font_info, SCREEN_WIDTH / 2 - 110, INFO_POSITION, window.black);
	}
}

void Game::display()
{
	display_background();
	display_plane();
	display_info();
	SDL_UpdateWindowSurface(window.window);
}

void Aircraft::move(int speed) { rect.y += speed; }
void Aircraft::miss(int count) { if (rect.y > SCREEN_HEIGHT) { hp = 0; status = count; } }
void Aircraft::down(int count) { if (hp <= 0) { hp = 0; if (status < count) { status += 1; } } }

void Aircraft::change_appearance()
{
	if (appearance == APPEARANCE1) { appearance = APPEARANCE2; }
	else { appearance = APPEARANCE1; }
}

void Bullet::move(int speed) { rect.y += speed; }
void Bullet::miss() { if (rect.y > SCREEN_HEIGHT) { status = DOWN_STATUS; } }
void Bullet::display(SDL_Surface* image) { SDL_BlitSurface(image, NULL, window.surface, &rect); }

void Bullet::hit(int damage, int width, int height)
{
	double distance_x = fabs((rect.x + width / 2) - (hero.rect.x + HERO_WIDTH / 2));
	double distance_y = fabs((rect.y + height / 2) - (hero.rect.y + HERO_HEIGHT / 2));
	if (distance_x <= HERO_WIDTH / 2 - ENEMY_HIT_DEV && distance_y <= HERO_HEIGHT / 2 - ENEMY_HIT_DEV && hero.hp > 0)
	{
		hero.hp -= damage;
		status = DOWN_STATUS;
	}
}

void Hero::init()
{
	rect = { SCREEN_WIDTH / 2 - HERO_WIDTH / 2, SCREEN_HEIGHT - HERO_HEIGHT - 40, HERO_WIDTH, HERO_HEIGHT };
	hp = HERO_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
	bomb_count = HERO_BOMB_INIT_COUNT;
}

void Hero::move()
{
	if (window.keystatus[SDL_SCANCODE_W] && rect.y >= BORDER_Y) { rect.y -= HERO_SPEED; }
	if (window.keystatus[SDL_SCANCODE_S] && rect.y <= SCREEN_HEIGHT - HERO_HEIGHT - BORDER_Y) { rect.y += HERO_SPEED; }
	if (window.keystatus[SDL_SCANCODE_A] && rect.x >= BORDER_X) { rect.x -= HERO_SPEED; }
	if (window.keystatus[SDL_SCANCODE_D] && rect.x <= SCREEN_WIDTH - HERO_WIDTH - BORDER_X) { rect.x += HERO_SPEED; }
}

void Hero::fire()
{
	if (status == ALIVE_STATUS)
	{
		int bullet_x = rect.x + HERO_WIDTH / 2 - HERO_BULLET_WIDTH / 2 + 1;
		int bullet_y = rect.y - HERO_BULLET_HEIGHT;
		hero_bullet.push_back(Hero_bullet(bullet_x, bullet_y));
	}
}

template <class Enemy>
void Hero::release_bomb(vector <Enemy> &enemy, int num)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		enemy[i].hp = 0;
		enemy[i].status = DOWN_STATUS;
		game.score += num;
	}
}

template <class Enemy>
void Hero::crash(vector <Enemy> &enemy, int width, int height, int num)
{
	if (status == ALIVE_STATUS)
	{
		for (int i = 0; i < enemy.size(); i++)
		{
			int distance_x = fabs((rect.x + HERO_WIDTH / 2) - (enemy[i].rect.x + width / 2));
			int distance_y = fabs((rect.y + HERO_HEIGHT / 2) - (enemy[i].rect.y + height / 2));

			if (distance_x <= (HERO_WIDTH + width) / 2 - CRASH_DEV && distance_y <= (HERO_HEIGHT + height) / 2 - CRASH_DEV && enemy[i].status == ALIVE_STATUS)
			{
				hp = 0;
				enemy[i].hp = 0;
				game.score += num;
			}
		}
	}
}

void Hero::display()
{
	if (status == ALIVE_STATUS)
	{
		if (appearance == APPEARANCE1) { window.image = window.hero_img[0]; }
		else if (appearance = APPEARANCE2) { window.image = window.hero_img[1]; }
	}
	else { window.image = window.hero_img[status + 1]; }
	SDL_BlitSurface(window.image, NULL, window.surface, &rect);
}

Enemy0::Enemy0(int enemy_x, int enemy_y)
{
	rect = { enemy_x, enemy_y, ENEMY0_WIDTH, ENEMY0_HEIGHT };
	hp = ENEMY0_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
}

void Enemy0::display()
{
	if (rect.y < 0) { window.overflow_blit(window.enemy0_img[status], rect); }
	else { SDL_BlitSurface(window.enemy0_img[status], NULL, window.surface, &rect); }
}

Enemy1::Enemy1(int enemy_x, int enemy_y)
{
	rect = { enemy_x, enemy_y, ENEMY1_WIDTH, ENEMY1_HEIGHT };
	hp = ENEMY1_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
}

void Enemy1::fire()
{
	if (status == ALIVE_STATUS)
	{
		int bullet_x = rect.x + ENEMY1_WIDTH / 2 - ENEMY1_BULLET_WIDTH / 2 + 1;
		int bullet_y = rect.y + ENEMY1_HEIGHT;
		enemy1_bullet.push_back(Enemy1_bullet(bullet_x, bullet_y));
	}
}

void Enemy1::display()
{
	if (status == ALIVE_STATUS && hp > ENEMY1_HP / 2) { window.image = window.enemy1_img[0]; }
	else if (status == ALIVE_STATUS && hp <= ENEMY1_HP / 2) { window.image = window.enemy1_img[1]; }
	else { window.image = window.enemy1_img[status + 1]; }

	if (rect.y < 0) { window.overflow_blit(window.image, rect); }
	else { SDL_BlitSurface(window.image, NULL, window.surface, &rect); }
}

Enemy2::Enemy2(int enemy_x, int enemy_y)
{
	rect = { enemy_x, enemy_y, ENEMY2_WIDTH, ENEMY2_HEIGHT };
	hp = ENEMY2_HP;
	appearance = APPEARANCE1;
	status = ALIVE_STATUS;
}

void Enemy2::fire()
{
	if (status == ALIVE_STATUS && hp > ENEMY2_HP / 2)
	{
		int bullet_x = rect.x + ENEMY2_WIDTH / 2 - ENEMY2_BULLET_WIDTH / 2 + 1;
		int bullet_y = rect.y + ENEMY2_HEIGHT;
		enemy2_bullet.push_back(Enemy2_bullet(bullet_x, bullet_y));
	}
}

void Enemy2::display()
{
	if (status == ALIVE_STATUS && hp > ENEMY2_HP / 2)
	{
		if (appearance == APPEARANCE1) { window.image = window.enemy2_img[0]; }
		else if (appearance == APPEARANCE2) { window.image = window.enemy2_img[1]; }
	}
	else if (status == 0 && hp <= ENEMY2_HP / 2) { window.image = window.enemy2_img[2]; }
	else { window.image = window.enemy2_img[status + 2]; }

	if (rect.y < 0) { window.overflow_blit(window.image, rect); }
	else { SDL_BlitSurface(window.image, NULL, window.surface, &rect); }
}

Hero_bullet::Hero_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x, bullet_y, HERO_BULLET_WIDTH, HERO_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}

void Hero_bullet::move() { rect.y -= HERO_BULLET_SPEED; }
void Hero_bullet::miss() { if (rect.y <= -HERO_BULLET_HEIGHT) { status = DOWN_STATUS; } }

void Hero_bullet::display()
{
	if (rect.y < 0) { window.overflow_blit(window.hero_bullet_img, rect); }
	else { SDL_BlitSurface(window.hero_bullet_img, NULL, window.surface, &rect); }
}

template <class Enemy>
void Hero_bullet::hit(vector <Enemy> &enemy, int width, int height, int num)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		int distance_x = fabs((rect.x + HERO_BULLET_WIDTH / 2) - (enemy[i].rect.x + width / 2));
		int distance_y = fabs((rect.y + HERO_BULLET_HEIGHT / 2) - (enemy[i].rect.y + height / 2));

		if (distance_x <= width / 2 - HERO_HIT_DEV && distance_y <= height / 2 - HERO_HIT_DEV && enemy[i].hp > 0)
		{
			enemy[i].hp -= HERO_BULLET_DAMAGE;
			status = DOWN_STATUS;
			if (enemy[i].hp <= 0) { game.score += num; }
		}
	}
}

Enemy1_bullet::Enemy1_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x, bullet_y, ENEMY1_BULLET_WIDTH, ENEMY1_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}

Enemy2_bullet::Enemy2_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x, bullet_y, ENEMY2_BULLET_WIDTH, ENEMY2_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}