#pragma once
#include "Includes.h"
#include "Globals.h"

struct TetrominoPiece {
	SDL_FRect rect;
	Globals::color color;
	SDL_FRect og;
};

class Tetromino {
public:
	std::vector<TetrominoPiece> pieces;
	SDL_FRect rect;
	int rotateIndex;
	int constructIndex; // basically the type of the tetromino in the "tetrominos" int[] in ConsoleApplication1.cpp
	SDL_FRect centerPoint;

	float storeY = 0;
	float storeX = 0;

	float change = 0;

	void addPiece(int lane, int col, int color)
	{
		TetrominoPiece piece;
		piece.rect.x = 0 + (16 * col);
		// temp until I add colors
		Globals::color c;
		c.r = 255;
		c.g = 255;
		c.b = 255;
		c.a = 255;
		std::cout << "added color " << color << std::endl;
		switch (color)
		{
			case 1:
				c.r = 128;
				c.g = 255;
				c.b = 240;
				break;
			case 2:
				c.r = 0;
				c.g = 128;
				c.b = 255;
				break;
			case 3:
				c.r = 255;
				c.g = 162;
				c.b = 24;
				break;
			case 4:
				c.r = 255;
				c.g = 240;
				c.b = 24;
				break;
			case 5:
				c.r = 128;
				c.g = 255;
				c.b = 128;
				break;
			case 6:
				c.r = 174;
				c.g = 64;
				c.b = 255;
				break;
			case 7:
				c.r = 255;
				c.g = 64;
				c.b = 64;
				break;
		}
		piece.color = c;
		piece.rect.y = 16 * lane;
		piece.rect.w = 16;
		piece.rect.h = 16;

		SDL_FRect copyRect = piece.rect;

		piece.og = copyRect;

		pieces.push_back(piece);

	}

	void movePiece(TetrominoPiece& piece, int directionX, int directionY)
	{
		SDL_FRect copyOfOriginal = piece.og;
		piece.rect.x = copyOfOriginal.x + (directionX * 16);
		piece.rect.y = copyOfOriginal.y + (directionY * 16);
	}

	void rotate()
	{
		rotateIndex = (rotateIndex + 1) % 4;
		switch (constructIndex)
		{
		case 0:
			switch (rotateIndex)
			{
			case 0:
				movePiece(pieces[0], 0, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 1:
				movePiece(pieces[0], 1, -3);
				movePiece(pieces[1], 0, -2);
				movePiece(pieces[2], -1, -1);
				movePiece(pieces[3], -2, 0);
				break;
			case 2:
				movePiece(pieces[0], 0, -1);
				movePiece(pieces[1], 0, -1);
				movePiece(pieces[2], 0, -1);
				movePiece(pieces[3], 0, -1);
				break;
			case 3:
				movePiece(pieces[0], 2, -3);
				movePiece(pieces[1], 1, -2);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -1, -1);
				break;
			}
			break;
		case 1:
			switch (rotateIndex)
			{
			case 0:
				movePiece(pieces[0], 0, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 1:
				movePiece(pieces[0], 2, 0);
				movePiece(pieces[1], 1, -1);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -1, 1);
				break;
			case 2:
				movePiece(pieces[0], 2, 2);
				movePiece(pieces[1], 2, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -2, 0);
				break;
			case 3:
				movePiece(pieces[0], 0, 2);
				movePiece(pieces[1], 1, 1);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -1, -1);
				break;
			}
			break;
		case 2:
			switch (rotateIndex)
			{
			case 0:
				movePiece(pieces[0], 0, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 1:
				movePiece(pieces[0], 1, -1);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], -1, 2);
				movePiece(pieces[3], 0, 1);
				break;
			case 2:
				movePiece(pieces[0], 2, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], -2, 2);
				movePiece(pieces[3], -2, 0);
				break;
			case 3:
				movePiece(pieces[0], 1, 1);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], -2, 0);
				movePiece(pieces[3], -1, -1);
				break;
			}
			break;
		case 4:
			switch (rotateIndex)
			{
			case 0:
				movePiece(pieces[0], 0, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 1:
				movePiece(pieces[0], 2, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 2);
				break;
			case 2:
				movePiece(pieces[0], 1, 1);
				movePiece(pieces[1], 1, 1);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -2, 2);
				break;
			case 3:
				movePiece(pieces[0], 1, 1);
				movePiece(pieces[1], -1, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -2, 1);
				break;
			}
			break;
		case 5:
			switch (rotateIndex)
			{
			case 0:
				movePiece(pieces[0], 0, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 1:
				movePiece(pieces[0], 1, 1);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 2:
				movePiece(pieces[0], 1, 1);
				movePiece(pieces[1], -1, 1);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 3:
				movePiece(pieces[0], 1, 1);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -2, 0);
				break;
			}
			break;
		case 6:
			switch (rotateIndex)
			{
			case 0:
				movePiece(pieces[0], 0, 0);
				movePiece(pieces[1], 0, 0);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 1:
				movePiece(pieces[0], 2, 0);
				movePiece(pieces[1], 0, 2);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 0);
				break;
			case 2:
				movePiece(pieces[0], 1, 2);
				movePiece(pieces[1], -1, 1);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], 0, 1);
				break;
			case 3:
				movePiece(pieces[0], 1, 0);
				movePiece(pieces[1], -1, 1);
				movePiece(pieces[2], 0, 0);
				movePiece(pieces[3], -2, 1);
				break;
			}
			break;
		}

	}

	bool checkCol(TetrominoPiece& piece)
	{

		for (TetrominoPiece oPiece : pieces)
		{
			if (piece.rect.x == rect.x + oPiece.rect.x && piece.rect.y == rect.y + oPiece.rect.y)
				return true;
		}
		
		return false;
	}

	void draw()
	{
		if (pieces.size() == 0)
			return;


		for (int i = 0; i < pieces.size(); i++)
		{
			std::vector<TetrominoPiece>& lane = pieces;
			SDL_SetRenderDrawColor(Globals::renderer, lane[i].color.r, lane[i].color.g, lane[i].color.b, lane[i].color.a);
			SDL_FRect newRect;
			newRect.x = rect.x + lane[i].rect.x;
			newRect.y = rect.y + lane[i].rect.y;
			newRect.w = lane[i].rect.w;
			newRect.h = lane[i].rect.h;
			SDL_RenderFillRectF(Globals::renderer, &newRect);
			SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, 255);
			SDL_RenderDrawRectF(Globals::renderer, &newRect);
		}
	}
};