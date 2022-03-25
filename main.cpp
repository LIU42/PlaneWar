#include "planewar.h"
#include "config.h"

using namespace std;

int main(int arg, char* argv[])
{
	Uint32 start_tick;
	Uint32 end_tick;
	Uint32 delta;

	game.load_image();
	game.load_fonts();
	game.add_timer();
	game.set_window();

	while (true)
	{
		start_tick = SDL_GetTicks();

		game.update();
		game.events();
		game.display();

		end_tick = SDL_GetTicks();
		delta = (1000 / game_fps) - (end_tick - start_tick);
		if (delta > 0 && delta < 1000 / game_fps) { SDL_Delay(delta); }
	}
	return 0;
}