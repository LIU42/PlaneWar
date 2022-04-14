#include "planewar.h"

using namespace std;

Game game;

SDL_RWops* get_resource(HINSTANCE hinst, LPCWSTR name, LPCWSTR type)
{
	HRSRC hrsrc = FindResource(hinst, name, type);
	DWORD size = SizeofResource(hinst, hrsrc);
	LPVOID data = LockResource(LoadResource(hinst, hrsrc));
	return SDL_RWFromConstMem(data, size);
}

SDL_Surface* load_surface(DWORD ID)
{
	SDL_RWops* src = get_resource(game.hinstance, MAKEINTRESOURCE(ID), TEXT("PNG"));
	SDL_Surface* origin_image = IMG_LoadPNG_RW(src);
	SDL_Surface* convert_image = SDL_ConvertSurface(origin_image, game.format, NULL);
	SDL_FreeSurface(origin_image);
	SDL_FreeRW(src);
	return convert_image;
}

Uint32 create_hero_change(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { game.hero.change_appearance(); }
	return interval;
}

Uint32 create_enemy2_change(Uint32 interval,void* param)
{
	if (game.status == PLAYING) { for (int i = 0; i < game.enemy2.size(); i++) { game.enemy2[i].change_appearance(); } }
	return interval;
}

Uint32 create_hero_fire(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { game.hero.fire(); }
	return interval;
}

Uint32 create_enemy1_fire(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { for (int i = 0; i < game.enemy1.size(); i++) { game.enemy1[i].fire(); } }
	return interval;
}

Uint32 create_enemy2_fire(Uint32 interval, void* param)
{
	if (game.status == PLAYING) { for (int i = 0; i < game.enemy2.size(); i++) { game.enemy2[i].fire(); } }
	return interval;
}
Uint32 create_alive(Uint32 interval, void* param)
{
	if (game.hero.status == ALIVE_STATUS && game.status == PLAYING) { game.score += ALIVE_SCORE; }
	return interval;
}

Uint32 create_aircraft_down(Uint32 interval, void* param)
{
	if (game.status == PLAYING)
	{
		game.hero.down(HERO_STATUS_MAX);
		for (int i = 0; i < game.enemy0.size(); i++) { game.enemy0[i].down(ENEMY0_STATUS_MAX); }
		for (int i = 0; i < game.enemy1.size(); i++) { game.enemy1[i].down(ENEMY1_STATUS_MAX); }
		for (int i = 0; i < game.enemy2.size(); i++) { game.enemy2[i].down(ENEMY2_STATUS_MAX); }
	}
	return interval;
}

template <class Enemy>
void add_enemy(vector <Enemy>& enemy, double p, int width, int height, int num)
{
	if (game.score >= num)
	{
		double n = game.randdouble(game.random);
		if (n < p)
		{
			int x = (int)(game.randdouble(game.random) * (SCREEN_WIDTH - width - 2 * BORDER_X) + BORDER_X);
			enemy.push_back(Enemy(x, -height));
		}
	}
}

void overflow_blit(SDL_Surface* image, SDL_Rect rect)
{
	SDL_Rect part = { 0,-rect.y,rect.w,rect.h };
	SDL_Rect dst = { rect.x,0,rect.w,rect.h + rect.y };
	SDL_BlitSurface(image, &part, game.surface, &dst);
}

void display_text(const char* text, TTF_Font* type, int x, int y, SDL_Color color)
{
	game.text_surface = TTF_RenderText_Blended(type, text, color);
	game.text_rect = { x,y,TEXT_RECT_WIDTH,TEXT_RECT_HEIGHT };
	SDL_BlitSurface(game.text_surface, NULL, game.surface, &game.text_rect);
	SDL_FreeSurface(game.text_surface);
}

void display_background()
{
	if (game.status == PLAYING)
	{
		if (game.background_position == SCREEN_HEIGHT)
		{
			game.background_position = 0;
		}
		game.background_position += BACKGROUND_SCROLL_SPEED;
	}
	game.background_rect_self = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT - game.background_position };
	game.background_rect_dst = { 0,game.background_position,SCREEN_WIDTH,SCREEN_HEIGHT - game.background_position };
	SDL_BlitSurface(game.background, &game.background_rect_self, game.surface, &game.background_rect_dst);

	game.background_rect_self = { 0,SCREEN_HEIGHT - game.background_position,SCREEN_WIDTH,game.background_position };
	game.background_rect_dst = { 0,0,SCREEN_WIDTH,game.background_position };
	SDL_BlitSurface(game.background, &game.background_rect_self, game.surface, &game.background_rect_dst);
}

Game::Game() : random((unsigned)time(NULL)), randdouble(0.0, 1.0)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	status = START;
	score = 0;
	score_best = 0;
	background_position = 0;
	keystatus = SDL_GetKeyboardState(NULL);
	format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
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

void Game::set_window()
{
	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	surface_rect = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };
	hinstance = GetModuleHandle(0);
}

void Game::load_image()
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

void Game::load_fonts()
{
	font_title = TTF_OpenFontRW(get_resource(hinstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, TITLE_FONT_SIZE);
	font_info = TTF_OpenFontRW(get_resource(hinstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, INFO_FONT_SIZE);
	black = BLACK;
	red = RED;
}

void Game::add_timer()
{
	hero_change = SDL_AddTimer(HERO_CHANGE_INTERVAL, create_hero_change, NULL);
	enemy2_change = SDL_AddTimer(ENEMY2_CHANGE_INTERVAL, create_enemy2_change, NULL);
	hero_fire = SDL_AddTimer(HERO_FIRE_INTERVAL, create_hero_fire, NULL);
	enemy1_fire = SDL_AddTimer(ENEMY1_FIRE_INTERVAL, create_enemy1_fire, NULL);
	enemy2_fire = SDL_AddTimer(ENEMY2_FIRE_INTERVAL, create_enemy2_fire, NULL);
	aircraft_down = SDL_AddTimer(AIRCRAFT_DOWN_INTERVAL, create_aircraft_down, NULL);
	alive = SDL_AddTimer(ALIVE_INTERVAL, create_alive, NULL);
}

void Game::exit_game()
{
	SDL_DestroyWindow(window);
	TTF_CloseFont(font_title);
	TTF_CloseFont(font_info);
	SDL_FreeFormat(format);

	SDL_FreeSurface(background);
	SDL_FreeSurface(hero_bullet_img);
	SDL_FreeSurface(enemy1_bullet_img);
	SDL_FreeSurface(enemy2_bullet_img);

	for (int i = 0; i < HERO_IMG_MAX; i++) { SDL_FreeSurface(hero_img[i]); }
	for (int i = 0; i < ENEMY0_IMG_MAX; i++) { SDL_FreeSurface(enemy0_img[i]); }
	for (int i = 0; i < ENEMY1_IMG_MAX; i++) { SDL_FreeSurface(enemy1_img[i]); }
	for (int i = 0; i < ENEMY2_IMG_MAX; i++) { SDL_FreeSurface(enemy2_img[i]); }

	SDL_Quit();
	exit(0);
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
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) { exit_game(); }
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (status == START || status == PAUSE) { status = PLAYING; }
			else if (status == END)
			{
				init();
				status = PLAYING;
			}
		}
		if (event.key.keysym.sym == SDLK_p && status == PLAYING) { status = PAUSE; }
		if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_b && status == PLAYING && hero.bomb_count > 0)
		{
			hero.release_bomb(enemy0, ENEMY0_SCORE);
			hero.release_bomb(enemy1, ENEMY1_SCORE);
			hero.release_bomb(enemy2, ENEMY2_SCORE);
			hero.bomb_count -= 1;
		}
	}
}

void Game::display()
{
	char info[30];
	display_background();
	if (status == START)
	{
		display_text("Welcome to PlaneWar", font_title, SCREEN_WIDTH / 2 - 125, (int)(SCREEN_HEIGHT * TITLE_POSITION), black);
		display_text("Click anywhere to START...", font_info, SCREEN_WIDTH / 2 - 110, (int)(SCREEN_HEIGHT * INFO_POSITION), black);
	}
	else if (status == PLAYING)
	{
		hero.display();
		for (int i = 0; i < enemy0.size(); i++) { enemy0[i].display(); }
		for (int i = 0; i < enemy1.size(); i++) { enemy1[i].display(); }
		for (int i = 0; i < enemy2.size(); i++) { enemy2[i].display(); }
		for (int i = 0; i < hero_bullet.size(); i++) { hero_bullet[i].display(); }
		for (int i = 0; i < enemy1_bullet.size(); i++) { enemy1_bullet[i].display(enemy1_bullet_img); }
		for (int i = 0; i < enemy2_bullet.size(); i++) { enemy2_bullet[i].display(enemy2_bullet_img); }

		sprintf_s(info, "score: %d", score);
		display_text(info, font_info, BORDER_TEXT, SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), black);
		sprintf_s(info, "HP: %d%%", hero.hp);
		display_text(info, font_info, SCREEN_WIDTH - (80 + BORDER_TEXT), SCREEN_HEIGHT - (BORDER_TEXT + INFO_FONT_SIZE), ((hero.hp > 30) ? black : red));
		sprintf_s(info, "BOMB: %d", hero.bomb_count);
		display_text(info, font_info, BORDER_TEXT, BORDER_TEXT, black);
	}
	else if (status == PAUSE)
	{
		display_text("PAUSE", font_title, SCREEN_WIDTH / 2 - 40, (int)(SCREEN_HEIGHT * TITLE_POSITION), black);
		display_text("Click anywhere to RESUME...", font_info, SCREEN_WIDTH / 2 - 110, (int)(SCREEN_HEIGHT * INFO_POSITION), black);
	}
	else if (status == END)
	{
		sprintf_s(info, "Your score: %d", score);
		display_text(info, font_info, SCREEN_WIDTH / 2 - 70, (int)(SCREEN_HEIGHT * SCORE_POSITION), black);
		sprintf_s(info, "Best score: %d", score_best);
		display_text(info, font_info, SCREEN_WIDTH / 2 - 70, (int)(SCREEN_HEIGHT * BEST_SCORE_POSITION), black);
		display_text("Gameover!", font_title, SCREEN_WIDTH / 2 - 60, (int)(SCREEN_HEIGHT * TITLE_POSITION), black);
		display_text("Click anywhere to RESTART...", font_info, SCREEN_WIDTH / 2 - 110, (int)(SCREEN_HEIGHT * INFO_POSITION), black);
	}
	SDL_UpdateWindowSurface(window);
}

void Aircraft::move(int speed) { rect.y += speed; }

void Aircraft::miss(int count)
{
	if (rect.y > SCREEN_HEIGHT)
	{
		hp = 0;
		status = count;
	}
}

void Aircraft::down(int count)
{
	if (hp <= 0)
	{
		hp = 0;
		if (status < count) { status += 1; }
	}
}

void Aircraft::change_appearance()
{
	if (appearance == APPEARANCE1) { appearance = APPEARANCE2; }
	else { appearance = APPEARANCE1; }
}

void Bullet::move(int speed) { rect.y += speed; }
void Bullet::miss() { if (rect.y > SCREEN_HEIGHT) { status = DOWN_STATUS; } }
void Bullet::display(SDL_Surface* image) { SDL_BlitSurface(image, NULL, game.surface, &rect); }

void Bullet::hit(int damage, int width, int height)
{
	double distance_x = fabs((rect.x + width / 2) - (game.hero.rect.x + HERO_WIDTH / 2));
	double distance_y = fabs((rect.y + height / 2) - (game.hero.rect.y + HERO_HEIGHT / 2));
	if (distance_x <= HERO_WIDTH / 2 - ENEMY_HIT_DEV && distance_y <= HERO_HEIGHT / 2 - ENEMY_HIT_DEV && game.hero.hp > 0)
	{
		game.hero.hp -= damage;
		status = DOWN_STATUS;
	}
}

Hero::Hero() { init(); }
void Hero::init()
{
	rect = { SCREEN_WIDTH / 2 - HERO_WIDTH / 2,SCREEN_HEIGHT - HERO_HEIGHT - 40,HERO_WIDTH,HERO_HEIGHT };
	hp = HERO_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
	bomb_count = HERO_BOMB_INIT_COUNT;
}

void Hero::move()
{
	if (game.keystatus[SDL_SCANCODE_W] && rect.y >= BORDER_Y) { rect.y -= HERO_SPEED; }
	if (game.keystatus[SDL_SCANCODE_S] && rect.y <= SCREEN_HEIGHT - HERO_HEIGHT - BORDER_Y) { rect.y += HERO_SPEED; }
	if (game.keystatus[SDL_SCANCODE_A] && rect.x >= BORDER_X) { rect.x -= HERO_SPEED; }
	if (game.keystatus[SDL_SCANCODE_D] && rect.x <= SCREEN_WIDTH - HERO_WIDTH - BORDER_X) { rect.x += HERO_SPEED; }
}

void Hero::fire()
{
	if (status == ALIVE_STATUS)
	{
		int bullet_x = rect.x + HERO_WIDTH / 2 - HERO_BULLET_WIDTH / 2 + 1;
		int bullet_y = rect.y - HERO_BULLET_HEIGHT;
		game.hero_bullet.push_back(Hero_bullet(bullet_x, bullet_y));
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
		if (appearance == APPEARANCE1) { game.image = game.hero_img[0]; }
		else if (appearance = APPEARANCE2) { game.image = game.hero_img[1]; }
	}
	else { game.image = game.hero_img[status + 1]; }
	SDL_BlitSurface(game.image, NULL, game.surface, &rect);
}

Enemy0::Enemy0(int enemy_x, int enemy_y)
{
	rect = { enemy_x,enemy_y,ENEMY0_WIDTH,ENEMY0_HEIGHT };
	hp = ENEMY0_HP;
	status = ALIVE_STATUS;
	appearance = APPEARANCE1;
}

void Enemy0::display()
{
	if (rect.y < 0) { overflow_blit(game.enemy0_img[status], rect); }
	else { SDL_BlitSurface(game.enemy0_img[status], NULL, game.surface, &rect); }
}

Enemy1::Enemy1(int enemy_x, int enemy_y)
{
	rect = { enemy_x,enemy_y,ENEMY1_WIDTH,ENEMY1_HEIGHT };
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
		game.enemy1_bullet.push_back(Enemy1_bullet(bullet_x, bullet_y));
	}
}

void Enemy1::display()
{
	if (status == ALIVE_STATUS && hp > ENEMY1_HP / 2) { game.image = game.enemy1_img[0]; }
	else if (status == ALIVE_STATUS && hp <= ENEMY1_HP / 2) { game.image = game.enemy1_img[1]; }
	else { game.image = game.enemy1_img[status + 1]; }

	if (rect.y < 0) { overflow_blit(game.image, rect); }
	else { SDL_BlitSurface(game.image, NULL, game.surface, &rect); }
}

Enemy2::Enemy2(int enemy_x, int enemy_y)
{
	rect = { enemy_x,enemy_y,ENEMY2_WIDTH,ENEMY2_HEIGHT };
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
		game.enemy2_bullet.push_back(Enemy2_bullet(bullet_x, bullet_y));
	}
}

void Enemy2::display()
{
	if (status == ALIVE_STATUS && hp > ENEMY2_HP / 2)
	{
		if (appearance == APPEARANCE1) { game.image = game.enemy2_img[0]; }
		else if (appearance == APPEARANCE2) { game.image = game.enemy2_img[1]; }
	}
	else if (status == 0 && hp <= ENEMY2_HP / 2) { game.image = game.enemy2_img[2]; }
	else { game.image = game.enemy2_img[status + 2]; }

	if (rect.y < 0) { overflow_blit(game.image, rect); }
	else { SDL_BlitSurface(game.image, NULL, game.surface, &rect); }
}

Hero_bullet::Hero_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x,bullet_y,HERO_BULLET_WIDTH,HERO_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}

void Hero_bullet::move() { rect.y -= HERO_BULLET_SPEED; }
void Hero_bullet::miss() { if (rect.y <= -HERO_BULLET_HEIGHT) { status = DOWN_STATUS; } }

void Hero_bullet::display()
{
	if (rect.y < 0) { overflow_blit(game.hero_bullet_img, rect); }
	else { SDL_BlitSurface(game.hero_bullet_img, NULL, game.surface, &rect); }
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
	rect = { bullet_x,bullet_y,ENEMY1_BULLET_WIDTH,ENEMY1_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}

Enemy2_bullet::Enemy2_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x,bullet_y,ENEMY2_BULLET_WIDTH,ENEMY2_BULLET_HEIGHT };
	status = ALIVE_STATUS;
}