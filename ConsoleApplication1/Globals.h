#pragma once
#include "Includes.h"
class Globals {
public:
	static SDL_Renderer* renderer;

	struct color {
		int r;
		int g;
		int b;
		int a;
	};

	
	struct lineClearAnim {
		float time;
		float dur;
		SDL_FRect rect;
	};
};