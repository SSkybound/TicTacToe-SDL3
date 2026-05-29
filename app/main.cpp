#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "TicTacToe.h"

int main(int argc, char* argv[])
{
	Game app{ "TicTacToe" };
	app.Init();

	return 0;
}