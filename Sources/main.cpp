#include "planewar.h"

using namespace std;

MainGame game;

int main(int arg, char* argv[])
{
	Uint32 startTick;
	Uint32 endTick;
	INT32 delayTick;

	game.initWindow();
	game.initGame();
	game.initColor();
	game.loadImage();
	game.loadFonts();
	game.addTimer();

	while (game.status != EXIT)
	{
		startTick = SDL_GetTicks();

		game.update();
		game.control();
		game.display();

		endTick = SDL_GetTicks();
		delayTick = (1000 / GAME_FPS) - (endTick - startTick);

		SDL_Delay((delayTick > 0) ? delayTick : 0);
	}
	game.close();
	return 0;
}