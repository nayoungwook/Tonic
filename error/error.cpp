#include "engine/error.h"

void error(const std::string &msg) {
	std::cerr << "Error! : " << msg << std::endl;
	SDL_Quit();
	exit(-1);
}