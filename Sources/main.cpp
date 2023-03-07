#include "planewar.h"

int main(int argc, char* argv[])
{
	MainGame game;

	Uint32 startTick = 0;
	Uint32 endTIck = 0;

	while (game.isRunning())
	{
		startTick = SDL_GetTicks();
		game.update();
		game.events();
		game.display();
		endTIck = SDL_GetTicks();
		game.delay(startTick, endTIck);
	}
	return EXIT_SUCCESS;
}