#include "planewar.h"

using namespace std;

Window window;
Game game;
Hero hero;

vector <Enemy0> enemy0;
vector <Enemy1> enemy1;
vector <Enemy2> enemy2;

vector <Hero_bullet> hero_bullet;
vector <Enemy1_bullet> enemy1_bullet;
vector <Enemy2_bullet> enemy2_bullet;

int main(int arg, char* argv[])
{
	Uint32 start_tick;
	Uint32 end_tick;
	Uint32 delta;

	window.init();
	window.init_color();
	window.load_image();
	window.load_fonts();

	game.init();
	game.add_timer();

	while (game.status != EXIT)
	{
		start_tick = SDL_GetTicks();

		game.update();
		game.events();
		game.display();

		end_tick = SDL_GetTicks();
		delta = (1000 / GAME_FPS) - (end_tick - start_tick);
		if (delta >= 0 && delta <= 1000 / GAME_FPS) { SDL_Delay(delta); }
	}
	window.close();
	return 0;
}