#include "planewar.h"
#include "config.h"
#include "global.h"
#include "resource.h"

using namespace std;

SDL_RWops* get_resource(HINSTANCE hinst, LPCWSTR name, LPCWSTR type)
{
	HRSRC hrsrc = FindResource(hinst, name, type);
	DWORD size = SizeofResource(hinst, hrsrc);
	LPVOID data = LockResource(LoadResource(hinst, hrsrc));
	return SDL_RWFromConstMem(data, size);
}

SDL_Surface* load_surface(DWORD ID, int width, int height, double img_width, double img_height)
{
	src = get_resource(hinstance, MAKEINTRESOURCE(ID), TEXT("PNG"));
	temp_origin = IMG_LoadPNG_RW(src);
	temp_zoom = rotozoomSurfaceXY(temp_origin, 0, width / img_width, height / img_height, 1);
	image = SDL_ConvertSurface(temp_zoom, format, NULL);
	SDL_FreeSurface(temp_origin);
	SDL_FreeSurface(temp_zoom);
	SDL_FreeRW(src);
	return image;
}

Uint32 create_hero_change(Uint32 interval, void* param)
{
	if (game.status == playing) { hero.change_img(); }
	return interval;
}

Uint32 create_enemy2_change(Uint32 interval,void* param)
{
	if (game.status == playing) { for (int i = 0; i < enemy2.size(); i++) { enemy2[i].change_img(); } }
	return interval;
}

Uint32 create_hero_fire(Uint32 interval, void* param)
{
	if (game.status == playing) { hero.fire(); }
	return interval;
}

Uint32 create_enemy1_fire(Uint32 interval, void* param)
{
	if (game.status == playing) { for (int i = 0; i < enemy1.size(); i++) { enemy1[i].fire(); } }
	return interval;
}

Uint32 create_enemy2_fire(Uint32 interval, void* param)
{
	if (game.status == playing) { for (int i = 0; i < enemy2.size(); i++) { enemy2[i].fire(); } }
	return interval;
}
Uint32 create_alive(Uint32 interval, void* param)
{
	if (hero.status == alive_status && game.status == playing) { game.score += alive_score; }
	return interval;
}

Uint32 create_aircraft_down(Uint32 interval, void* param)
{
	if (game.status == playing)
	{
		hero.down(hero_status_max);
		for (int i = 0; i < enemy0.size(); i++) { enemy0[i].down(enemy0_status_max); }
		for (int i = 0; i < enemy1.size(); i++) { enemy1[i].down(enemy1_status_max); }
		for (int i = 0; i < enemy2.size(); i++) { enemy2[i].down(enemy2_status_max); }
	}
	return interval;
}

template <class Enemy>
void add_enemy(vector <Enemy>& enemy, double p, int width, int height, int num)
{
	if (game.score >= num)
	{
		double n = randdouble(random);
		if (n < p)
		{
			int x = (int)(randdouble(random) * (screen_width - width - 2 * border_x) + border_x);
			enemy.push_back(Enemy(x, -height));
		}
	}
}

void overflow_blit(SDL_Surface* image, SDL_Rect rect)
{
	SDL_Rect part = { 0,-rect.y,rect.w,rect.h };
	SDL_Rect dst = { rect.x,0,rect.w,rect.h + rect.y };
	SDL_BlitSurface(image, &part, surface, &dst);
}

void display_text(const char* text, TTF_Font* type, int x, int y, SDL_Color color)
{
	text_surface = TTF_RenderText_Blended(type, text, color);
	text_rect = { x,y,text_rect_width,text_rect_height };
	SDL_BlitSurface(text_surface, NULL, surface, &text_rect);
	SDL_FreeSurface(text_surface);
}

void display_background()
{
	if (game.status == playing)
	{
		if (game.background_position == screen_height)
		{
			game.background_position = 0;
		}
		game.background_position += background_scroll_speed;
	}
	background_rect_self = { 0,0,screen_width,screen_height - game.background_position };
	background_rect_dst = { 0,game.background_position,screen_width,screen_height - game.background_position };
	SDL_BlitSurface(background, &background_rect_self, surface, &background_rect_dst);
	background_rect_self = { 0,screen_height - game.background_position,screen_width,game.background_position };
	background_rect_dst = { 0,0,screen_width,game.background_position };
	SDL_BlitSurface(background, &background_rect_self, surface, &background_rect_dst);
}

Game::Game()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	status = start;
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
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	surface_rect = { 0,0,screen_width,screen_height };
	hinstance = GetModuleHandle(0);
}

void Game::load_image()
{
	background = load_surface(IDB_PNG1, screen_width, screen_height, 480.0, 852.0);
	hero_bullet_img = load_surface(IDB_PNG34, hero_bullet_width, hero_bullet_height, 9.0, 21.0);
	enemy1_bullet_img = load_surface(IDB_PNG32, enemy1_bullet_width, enemy1_bullet_height, 9.0, 21.0);
	enemy2_bullet_img = load_surface(IDB_PNG33, enemy2_bullet_width, enemy2_bullet_height, 22.0, 22.0);

	for (int i = 0; i < hero_img_max; i++) { hero_img[i] = load_surface(IDB_PNG25 + i, hero_width, hero_height, 100.0, 124.0); }
	for (int i = 0; i < enemy0_img_max; i++) { enemy0_img[i] = load_surface(IDB_PNG2 + i, enemy0_width, enemy0_height, 51.0, 39.0); }
	for (int i = 0; i < enemy1_img_max; i++) { enemy1_img[i] = load_surface(IDB_PNG8 + i, enemy1_width, enemy1_height, 69.0, 89.0); }
	for (int i = 0; i < enemy2_img_max; i++) { enemy2_img[i] = load_surface(IDB_PNG15 + i, enemy2_width, enemy2_height, 165.0, 256.0); }
}

void Game::load_fonts()
{
	font_title = TTF_OpenFontRW(get_resource(hinstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, title_font_size);
	font_info = TTF_OpenFontRW(get_resource(hinstance, MAKEINTRESOURCE(IDR_FONT1), RT_FONT), 1, info_font_size);
	black = color_black;
	red = color_red;
}

void Game::add_timer()
{
	hero_change = SDL_AddTimer(hero_change_interval, create_hero_change, NULL);
	enemy2_change = SDL_AddTimer(enemy2_change_interval, create_enemy2_change, NULL);
	hero_fire = SDL_AddTimer(hero_fire_interval, create_hero_fire, NULL);
	enemy1_fire = SDL_AddTimer(enemy1_fire_interval, create_enemy1_fire, NULL);
	enemy2_fire = SDL_AddTimer(enemy2_fire_interval, create_enemy2_fire, NULL);
	aircraft_down = SDL_AddTimer(aircraft_down_interval, create_aircraft_down, NULL);
	alive = SDL_AddTimer(alive_interval, create_alive, NULL);
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
	for (int i = 0; i < hero_img_max; i++) { SDL_FreeSurface(hero_img[i]); }
	for (int i = 0; i < enemy0_img_max; i++) { SDL_FreeSurface(enemy0_img[i]); }
	for (int i = 0; i < enemy1_img_max; i++) { SDL_FreeSurface(enemy1_img[i]); }
	for (int i = 0; i < enemy2_img_max; i++) { SDL_FreeSurface(enemy2_img[i]); }
	SDL_Quit();
	exit(0);
}

void Game::update()
{
	if (status == playing)
	{
		add_enemy(enemy0, p_enemy0, enemy0_width, enemy0_height, enemy0_append_score);
		add_enemy(enemy1, p_enemy1, enemy1_width, enemy1_height, enemy1_append_score);
		add_enemy(enemy2, p_enemy2, enemy2_width, enemy2_height, enemy2_append_score);

		hero.crash(enemy0, enemy0_width, enemy0_height, enemy0_score);
		hero.crash(enemy1, enemy1_width, enemy1_height, enemy1_score);
		hero.crash(enemy2, enemy2_width, enemy2_height, enemy2_score);

		for (int i = 0; i < enemy0.size(); i++)
		{
			enemy0[i].move(enemy0_speed);
			enemy0[i].miss(enemy0_status_max);
			if (enemy0[i].status == enemy0_status_max) { enemy0.erase(enemy0.begin() + i--); }
		}
		for (int i = 0; i < enemy1.size(); i++)
		{
			enemy1[i].move(enemy1_speed);
			enemy1[i].miss(enemy1_status_max);
			if (enemy1[i].status == enemy1_status_max) { enemy1.erase(enemy1.begin() + i--); }
		}
		for (int i = 0; i < enemy2.size(); i++)
		{
			enemy2[i].move(enemy2_speed);
			enemy2[i].miss(enemy2_status_max);
			if (enemy2[i].status == enemy2_status_max) { enemy2.erase(enemy2.begin() + i--); }
		}
		for (int i = 0; i < hero_bullet.size(); i++)
		{
			hero_bullet[i].move();
			hero_bullet[i].miss();
			hero_bullet[i].hit(enemy0, enemy0_width, enemy0_height, enemy0_score);
			hero_bullet[i].hit(enemy1, enemy1_width, enemy1_height, enemy1_score);
			hero_bullet[i].hit(enemy2, enemy2_width, enemy2_height, enemy2_score);
			if (hero_bullet[i].status == down_status) { hero_bullet.erase(hero_bullet.begin() + i--); }
		}
		for (int i = 0; i < enemy1_bullet.size(); i++)
		{
			enemy1_bullet[i].move(enemy1_bullet_speed);
			enemy1_bullet[i].miss();
			enemy1_bullet[i].hit(enemy1_bullet_damage, enemy1_bullet_width, enemy1_bullet_height);
			if (enemy1_bullet[i].status == down_status) { enemy1_bullet.erase(enemy1_bullet.begin() + i--); }
		}
		for (int i = 0; i < enemy2_bullet.size(); i++)
		{
			enemy2_bullet[i].move(enemy2_bullet_speed);
			enemy2_bullet[i].miss();
			enemy2_bullet[i].hit(enemy2_bullet_damage, enemy2_bullet_width, enemy2_bullet_height);
			if (enemy2_bullet[i].status == down_status) { enemy2_bullet.erase(enemy2_bullet.begin() + i--); }
		}
	}
	if (status == end) { if (score > score_best) { score_best = score; } }
}

void Game::events()
{
	if (status == playing && hero.status == alive_status) { hero.move(); }
	if (hero.status == hero_status_max) { status = end; }
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) { exit_game(); }
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (status == start || status == pause) { status = playing; }
			else if (status == end)
			{
				init();
				status = playing;
			}
		}
		if (event.key.keysym.sym == SDLK_p && status == playing) { status = pause; }
		if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_b && status == playing && hero.bomb_count > 0)
		{
			hero.release_bomb(enemy0, enemy0_score);
			hero.release_bomb(enemy1, enemy1_score);
			hero.release_bomb(enemy2, enemy2_score);
			hero.bomb_count -= 1;
		}
	}
}

void Game::display()
{
	char info[30];
	display_background();
	if (status == start)
	{
		display_text("Welcome to PlaneWar", font_title, screen_width / 2 - 125, (int)(screen_height * title_position), black);
		display_text("Click anywhere to start...", font_info, screen_width / 2 - 110, (int)(screen_height * info_position), black);
	}
	else if (status == playing)
	{
		hero.display();
		for (int i = 0; i < enemy0.size(); i++) { enemy0[i].display(); }
		for (int i = 0; i < enemy1.size(); i++) { enemy1[i].display(); }
		for (int i = 0; i < enemy2.size(); i++) { enemy2[i].display(); }
		for (int i = 0; i < hero_bullet.size(); i++) { hero_bullet[i].display(); }
		for (int i = 0; i < enemy1_bullet.size(); i++) { enemy1_bullet[i].display(enemy1_bullet_img); }
		for (int i = 0; i < enemy2_bullet.size(); i++) { enemy2_bullet[i].display(enemy2_bullet_img); }

		sprintf_s(info, "score: %d", score);
		display_text(info, font_info, border_text, screen_height - (border_text + info_font_size), black);
		sprintf_s(info, "HP: %d%%", hero.hp);
		display_text(info, font_info, screen_width - (80 + border_text), screen_height - (border_text + info_font_size), ((hero.hp > 30) ? black : red));
		sprintf_s(info, "BOMB: %d", hero.bomb_count);
		display_text(info, font_info, border_text, border_text, black);
	}
	else if (status == pause)
	{
		display_text("Pause", font_title, screen_width / 2 - 40, (int)(screen_height * title_position), black);
		display_text("Click anywhere to resume...", font_info, screen_width / 2 - 110, (int)(screen_height * info_position), black);
	}
	else if (status == end)
	{
		sprintf_s(info, "Your score: %d", score);
		display_text(info, font_info, screen_width / 2 - 70, (int)(screen_height * score_position), black);
		sprintf_s(info, "Best score: %d", score_best);
		display_text(info, font_info, screen_width / 2 - 70, (int)(screen_height * best_score_position), black);
		display_text("Gameover!", font_title, screen_width / 2 - 60, (int)(screen_height * title_position), black);
		display_text("Click anywhere to restart...", font_info, screen_width / 2 - 110, (int)(screen_height * info_position), black);
	}
	SDL_UpdateWindowSurface(window);
}

void Aircraft::move(int speed) { rect.y += speed; }

void Aircraft::miss(int count)
{
	if (rect.y > screen_height)
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

void Aircraft::change_img()
{
	if (change == appearance1) { change = appearance2; }
	else { change = appearance1; }
}

void Bullet::move(int speed) { rect.y += speed; }
void Bullet::miss() { if (rect.y > screen_height) { status = down_status; } }
void Bullet::display(SDL_Surface* image) { SDL_BlitSurface(image, NULL, surface, &rect); }

void Bullet::hit(int damage, int width, int height)
{
	double distance_x = fabs((rect.x + width / 2) - (hero.rect.x + hero_width / 2));
	double distance_y = fabs((rect.y + height / 2) - (hero.rect.y + hero_height / 2));
	if (distance_x <= hero_width / 2 - enemy_hit_dev && distance_y <= hero_height / 2 - enemy_hit_dev && hero.hp > 0)
	{
		hero.hp -= damage;
		status = down_status;
	}
}

Hero::Hero() { init(); }
void Hero::init()
{
	rect = { screen_width / 2 - hero_width / 2,screen_height - hero_height - 40,hero_width,hero_height };
	hp = hero_hp;
	status = alive_status;
	change = appearance1;
	bomb_count = hero_bomb_init_count;
}

void Hero::move()
{
	if (game.keystatus[SDL_SCANCODE_W] && rect.y >= border_y) { rect.y -= hero_speed; }
	if (game.keystatus[SDL_SCANCODE_S] && rect.y <= screen_height - hero_height - border_y) { rect.y += hero_speed; }
	if (game.keystatus[SDL_SCANCODE_A] && rect.x >= border_x) { rect.x -= hero_speed; }
	if (game.keystatus[SDL_SCANCODE_D] && rect.x <= screen_width - hero_width - border_x) { rect.x += hero_speed; }
}

void Hero::fire()
{
	if (status == alive_status)
	{
		int bullet_x = rect.x + hero_width / 2 - hero_bullet_width / 2 + 1;
		int bullet_y = rect.y - hero_bullet_height;
		hero_bullet.push_back(Hero_bullet(bullet_x, bullet_y));
	}
}

template <class Enemy>
void Hero::release_bomb(vector <Enemy> &enemy, int num)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		enemy[i].hp = 0;
		enemy[i].status = down_status;
		game.score += num;
	}
}

template <class Enemy>
void Hero::crash(vector <Enemy> &enemy, int width, int height, int num)
{
	if (status == alive_status)
	{
		for (int i = 0; i < enemy.size(); i++)
		{
			int distance_x = fabs((rect.x + hero_width / 2) - (enemy[i].rect.x + width / 2));
			int distance_y = fabs((rect.y + hero_height / 2) - (enemy[i].rect.y + height / 2));
			if (distance_x <= (hero_width + width) / 2 - crash_dev && distance_y <= (hero_height + height) / 2 - crash_dev && enemy[i].status == alive_status)
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
	if (status == alive_status)
	{
		if (change == appearance1) { image = hero_img[0]; }
		else if (change = appearance2) { image = hero_img[1]; }
	}
	else { image = hero_img[status + 1]; }
	SDL_BlitSurface(image, NULL, surface, &rect);
}

Enemy0::Enemy0(int enemy_x, int enemy_y)
{
	rect = { enemy_x,enemy_y,enemy0_width,enemy0_height };
	hp = enemy0_hp;
	status = alive_status;
	change = appearance1;
}

void Enemy0::display()
{
	if (rect.y < 0) { overflow_blit(enemy0_img[status], rect); }
	else { SDL_BlitSurface(enemy0_img[status], NULL, surface, &rect); }
}

Enemy1::Enemy1(int enemy_x, int enemy_y)
{
	rect = { enemy_x,enemy_y,enemy1_width,enemy1_height };
	hp = enemy1_hp;
	status = alive_status;
	change = appearance1;
}

void Enemy1::fire()
{
	if (status == alive_status)
	{
		int bullet_x = rect.x + enemy1_width / 2 - enemy1_bullet_width / 2 + 1;
		int bullet_y = rect.y + enemy1_height;
		enemy1_bullet.push_back(Enemy1_bullet(bullet_x, bullet_y));
	}
}

void Enemy1::display()
{
	if (status == alive_status && hp > enemy1_hp / 2) { image = enemy1_img[0]; }
	else if (status == alive_status && hp <= enemy1_hp / 2) { image = enemy1_img[1]; }
	else { image = enemy1_img[status + 1]; }

	if (rect.y < 0) { overflow_blit(image, rect); }
	else { SDL_BlitSurface(image, NULL, surface, &rect); }
}

Enemy2::Enemy2(int enemy_x, int enemy_y)
{
	rect = { enemy_x,enemy_y,enemy2_width,enemy2_height };
	hp = enemy2_hp;
	change = appearance1;
	status = alive_status;
}

void Enemy2::fire()
{
	if (status == alive_status && hp > enemy2_hp / 2)
	{
		int bullet_x = rect.x + enemy2_width / 2 - enemy2_bullet_width / 2 + 1;
		int bullet_y = rect.y + enemy2_height;
		enemy2_bullet.push_back(Enemy2_bullet(bullet_x, bullet_y));
	}
}

void Enemy2::display()
{
	if (status == alive_status && hp > enemy2_hp / 2)
	{
		if (change == appearance1) { image = enemy2_img[0]; }
		else if (change == appearance2) { image = enemy2_img[1]; }
	}
	else if (status == 0 && hp <= enemy2_hp / 2) { image = enemy2_img[2]; }
	else { image = enemy2_img[status + 2]; }

	if (rect.y < 0) { overflow_blit(image, rect); }
	else { SDL_BlitSurface(image, NULL, surface, &rect); }
}

Hero_bullet::Hero_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x,bullet_y,hero_bullet_width,hero_bullet_height };
	status = alive_status;
}

void Hero_bullet::move() { rect.y -= hero_bullet_speed; }
void Hero_bullet::miss() { if (rect.y <= -hero_bullet_height) { status = down_status; } }

void Hero_bullet::display()
{
	if (rect.y < 0) { overflow_blit(hero_bullet_img, rect); }
	else { SDL_BlitSurface(hero_bullet_img, NULL, surface, &rect); }
}

template <class Enemy>
void Hero_bullet::hit(vector <Enemy> &enemy, int width, int height, int num)
{
	for (int i = 0; i < enemy.size(); i++)
	{
		int distance_x = fabs((rect.x + hero_bullet_width / 2) - (enemy[i].rect.x + width / 2));
		int distance_y = fabs((rect.y + hero_bullet_height / 2) - (enemy[i].rect.y + height / 2));
		if (distance_x <= width / 2 - hero_hit_dev && distance_y <= height / 2 - hero_hit_dev && enemy[i].hp > 0)
		{
			enemy[i].hp -= hero_bullet_damage;
			status = down_status;
			if (enemy[i].hp <= 0) { game.score += num; }
		}
	}
}

Enemy1_bullet::Enemy1_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x,bullet_y,enemy1_bullet_width,enemy1_bullet_height };
	status = alive_status;
}

Enemy2_bullet::Enemy2_bullet(int bullet_x, int bullet_y)
{
	rect = { bullet_x,bullet_y,enemy2_bullet_width,enemy2_bullet_height };
	status = alive_status;
}