#ifndef __GLOBAL_H__
#define __GLOBAL_H__

using namespace std;

HINSTANCE hinstance;
SDL_Window* window;
SDL_Surface* surface;
SDL_Rect surface_rect;
SDL_Rect background_rect_self;
SDL_Rect background_rect_dst;
SDL_Rect overflow_rect_self;
SDL_Rect overflow_rect_dst;
SDL_Event event;
SDL_PixelFormat* format;

SDL_Surface* image;
SDL_Surface* background;
SDL_Surface* hero_bullet_img;
SDL_Surface* enemy1_bullet_img;
SDL_Surface* enemy2_bullet_img;
SDL_Surface* hero_img[7];
SDL_Surface* enemy0_img[6];
SDL_Surface* enemy1_img[7];
SDL_Surface* enemy2_img[10];

TTF_Font* font_title;
TTF_Font* font_info;
SDL_Surface* text_surface;
SDL_Rect text_rect;
SDL_Color black;
SDL_Color red;

SDL_TimerID hero_change;
SDL_TimerID hero_fire;
SDL_TimerID enemy1_fire;
SDL_TimerID enemy2_change;
SDL_TimerID enemy2_fire;
SDL_TimerID aircraft_down;
SDL_TimerID alive;

default_random_engine random((unsigned)time(NULL));
uniform_real_distribution <double> randdouble(0.0, 1.0);

Game game;
Hero hero;
vector <Enemy0> enemy0;
vector <Enemy1> enemy1;
vector <Enemy2> enemy2;
vector <Hero_bullet> hero_bullet;
vector <Enemy1_bullet> enemy1_bullet;
vector <Enemy2_bullet> enemy2_bullet;
#endif