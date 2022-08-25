#include "planewar.h"

MainGame game;

int main(int argc, char* argv[])
{
	int startTick;
	int endTick;
	int delayTick;

	srand((unsigned)time(NULL));

	game.initWindow();
	game.initData();
	game.initGame();
	game.initColor();
	game.loadImage();
	game.loadFonts();
	game.addTimer();

	while (game.status != EXIT)
	{
		startTick = SDL_GetTicks();

		game.update();
		game.events();
		game.display();

		endTick = SDL_GetTicks();
		delayTick = (1000 / GAME_FPS) - (endTick - startTick);

		SDL_Delay((delayTick > 0) ? delayTick : 0);
	}
	game.close();
	return 0;
}