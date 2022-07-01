#include "planewar.h"

using namespace std;

Window window;
Game game;
Hero hero;

vector <Enemy0> enemy0;
vector <Enemy1> enemy1;
vector <Enemy2> enemy2;

vector <HeroBullet> heroBullet;
vector <Enemy1Bullet> enemy1Bullet;
vector <Enemy2Bullet> enemy2Bullet;

int main(int arg, char* argv[])
{
	Uint32 startTick;
	Uint32 endTick;
	INT32 delayTick;

	window.init();
	window.initColor();
	window.loadImage();
	window.loadFonts();

	game.init();
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
	window.close();
	return 0;
}