#include "Includes.h"
#include "Tetromino.h"
#include <ctime>
#include "Globals.h"
#pragma comment(lib,"x64\\SDL2.lib")
#pragma comment(lib,"x64\\SDL2_ttf.lib")

// the pieces stored in ints
// 0 = blank space
// 1 = cyan
// 2 = blue
// 3 = orange
// 4 = yellow
// 5 = green
// 6 = purple
// 7 = red

int tetrominos[7][8] = { {
	{1},{1},{1},{1}, // 4 Wide
	{0},{0},{0},{0}
	},
	{
	{2},{0},{0},{0}, // L-Piece
	{2},{2},{2},{0}
	},
	{
	{0},{0},{3},{0}, // Reverse L-Piece
	{3},{3},{3},{0}
	},
	{
	{4},{4},{0},{0}, // Square
	{4},{4},{0},{0}
	},
	{
	{0},{5},{5},{0}, // Reverse S
	{5},{5},{0},{0}
	},
	{
	{0},{6},{0},{0}, // T-Piece
	{6},{6},{6},{0}
	},
	{
	{7},{7},{0},{0}, // S
	{0},{7},{7},{0}
	}
};



// [level][0] = speed for that level

const float cellFrames[16][1] = { // gotten from https://gamedev.stackexchange.com/questions/159835/understanding-tetris-speed-curve
	{0.01667},
	{0.021017},
	{0.026977},
	{0.035256},
	{0.04693},
	{0.06361},
	{0.0879},
	{0.1236},
	{0.1775},
	{0.2598},
	{0.388},
	{0.59},
	{0.92},
	{1.46},
	{2.36}
};

float deltaTime = 0;

std::vector<TetrominoPiece> groundedPieces[21];

TTF_Font* font;

int _score;
float _clears;

float scoreW, scoreH;
SDL_Texture* score;
float levelW, levelH;
SDL_Texture* levelt;

float startLevelW, startLevelH;
SDL_Texture* startLevel;

float lastIndex = -1;

SDL_FRect scrollingRect;
SDL_Rect scrollingSrc;

SDL_Texture* startTexture;
SDL_Texture* beforeTexture;

float mTime;
float midW, midH;
SDL_Texture* middleText;

Tetromino* faillingTetr;
Tetromino* ghostTetr;

bool isOffseting = false;

int selectedPieceIndex = 0;

std::vector<int> offsetX;
std::vector<int> offsetY;

float boardX = ((864 / 2) - 80);
float bottomBoard = 512;

Tetromino* held = NULL;
bool usedHold = false;

void holdPiece()
{
	if (!usedHold)
	{
		usedHold = true;
		if (held == NULL)
		{
			held = faillingTetr;
			faillingTetr = nullptr;
		}
		else
		{
			faillingTetr = held;
			held = NULL;
		}
	}
}

void createAndSetText(std::string str, SDL_Texture** texToWrite, float* w, float* h)
{
	if (*texToWrite)
		SDL_DestroyTexture(*texToWrite);
	SDL_Surface* surf = TTF_RenderText_Blended(font, str.c_str(), {255,255,255,255});
	*texToWrite = SDL_CreateTextureFromSurface(Globals::renderer,surf);
	*w = surf->w;
	*h = surf->h;
	SDL_FreeSurface(surf);
}

void setScore(std::string sc)
{
	createAndSetText("Score " + sc, &score, &scoreW, &scoreH);
}


void setLevel(std::string lvl)
{
	createAndSetText("Level " + lvl, &levelt, &levelW, &levelH);
}

void setMid(std::string mid)
{
	mTime = 500;
	SDL_Surface* surf = TTF_RenderText_Blended(font, mid.c_str(), {255,255,255,255});
	if (middleText)
		SDL_DestroyTexture(middleText);
	middleText = SDL_CreateTextureFromSurface(Globals::renderer,surf);
	midW = surf->w;
	midH = surf->h;
	SDL_FreeSurface(surf);
}

bool random = false;

void createTetr(int constructId) {
	Tetromino* tetr = new Tetromino();

	if (random)
	{
		for(int i = 0; i < 7; i++)
		{
			auto& ar = tetrominos[i];
			std::cout << ar << std::endl;
			for (int ii = 0; ii < 6; ii++)
			{
				ar[ii] = rand() % 7;
			}
		}
	}

	tetr->constructIndex = constructId;
	tetr->rect.w = 16;
	tetr->rect.h = 16;

	int colIndex = 0;
	int lanes = 1;

	for (int ii = 0; ii < 4; ii++)
	{
		int piece = tetrominos[constructId][ii];
		int bottomPiece = tetrominos[constructId][ii + 4];
		if (piece != 0)
			tetr->addPiece(0, colIndex, piece);
		if (bottomPiece != 0)
		{
			tetr->addPiece(1, colIndex, bottomPiece);
			lanes = 2;
		}

		if (piece != 0 || bottomPiece != 0)
			colIndex++;

	}
	tetr->rect.w = colIndex * 16;
	tetr->rect.h = lanes * 16;
	tetr->rect.x = boardX + (16 * 3);
	tetr->rect.y = 192;
	tetr->storeY = 192;
	faillingTetr = tetr;
}


void toClipboard(const std::string& s) {
	OpenClipboard(0);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size());
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size());
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

bool playing = false;

float currentTime = 0;
float simulatedTime = 0;

float rotateTime = 0;


int level;


float align(float value, float size)
{
	return value - std::abs(std::fmod(value, size));
}


void checkBoardCol() {
	for (TetrominoPiece piece : faillingTetr->pieces)
	{
		if (faillingTetr->rect.x + piece.rect.x < boardX)
			faillingTetr->rect.x += 16;
		if (faillingTetr->rect.x + piece.rect.x >= boardX + 160)
			faillingTetr->rect.x -= 16;
	}

}

int findLaneOfPiece(int pieceY) {
	int lane = pieceY - 192;
	for (int i = 0; i < 22; i++)
	{
		int laneMay = i * 16;
		if (lane == laneMay)
			return i;
	}
	return -1;
}

SDL_Texture* tetrisText;
SDL_Texture* highscore;
SDL_Texture* pressToPlay;

Tetromino* lowestPos() {
	Tetromino* copy = new Tetromino();
	copy->pieces = faillingTetr->pieces;
	copy->rect = faillingTetr->rect;
	bool landed = false;
	TetrominoPiece lowestPiece = copy->pieces[0];
	for (TetrominoPiece p : copy->pieces)
	{
		if (p.rect.y > lowestPiece.rect.y)
		{
			lowestPiece = p;
		}
	}
	while (!landed)
	{
		copy->rect.y += 16;

		for (TetrominoPiece p : copy->pieces)
		{	
			int lane = findLaneOfPiece(copy->rect.y + p.rect.y);
			if (lane == -1)
				continue;
			for (TetrominoPiece pp : groundedPieces[lane])
			{
				if (copy->checkCol(pp))
				{
					landed = true;
					copy->rect.y -= 16;
					break;
				}
			}
			if (landed)
				break;
		}

		
		if (copy->rect.y + lowestPiece.rect.y >= bottomBoard && !landed)
		{
			landed = true;
			copy->rect.y -= 16;
		}
	}
	return copy;
}

std::vector<Globals::lineClearAnim> lineClearAnims;

void lineClearAnim(int y)
{
	Globals::lineClearAnim anim;
	anim.dur = 1000;
	anim.rect.x = boardX;
	anim.rect.y = y;
	anim.rect.w = 160;
	anim.rect.h = 16;

	lineClearAnims.push_back(anim);
}

void resortLanes()
{
	std::vector<TetrominoPiece> pieces;
	for(int lane = 0; lane < 21; lane++)
	{
		for(int i = 0; i < groundedPieces[lane].size(); i++)
		{
			TetrominoPiece piece = groundedPieces[lane][i]; // copy
			pieces.push_back(piece);
		}
		groundedPieces[lane].clear();
	}
	for(int i = 0; i < pieces.size(); i++)
	{
		TetrominoPiece p = pieces[i]; // copy
		int lane = findLaneOfPiece(p.rect.y);
		if (lane == -1)
			continue;
		groundedPieces[lane].push_back(p);
	}
}

bool gameover = false;
std::fstream high;
float lastHighscore;


void placePiece()
{
	usedHold = false;
	for (TetrominoPiece piece : faillingTetr->pieces)
	{
		piece.rect.x = faillingTetr->rect.x + piece.rect.x;
		piece.rect.y = faillingTetr->rect.y + piece.rect.y;

		int lane = findLaneOfPiece(piece.rect.y);

		if (lane == -1) // game over
		{
			std::cout << "GAME OVER!" << std::endl;
			gameover = true;
			if (_score > lastHighscore)
			{
				high.close();
				DeleteFileA("highscore.dat");
				std::ofstream outfile ("highscore.dat");
				outfile << _score << std::endl;
				outfile.close();
				high.open("highscore.dat");
			}
			setMid("Game over!");
			for(int i = 0; i < 21; i++)
			{
				for(int p = 0; p < groundedPieces[i].size(); p++)
				{
					TetrominoPiece& pi = groundedPieces[i][p];
					pi.color = {60,60,60,255};
				}
			}
			return;
		}

		groundedPieces[lane].push_back(piece);
	}
	resortLanes();
	// check for line clears
	int clears = 0;
	for(int lane = 0; lane < 21; lane++)
	{
		std::vector<TetrominoPiece>& pieces = groundedPieces[lane];
		if (pieces.size() == 10)
		{
			std::cout << "Cleared " << pieces.size() << std::endl;
			pieces.clear();
			lineClearAnim(pieces[0].rect.y);
			for(int i = 0; i < lane + 1; i++)
			{
				for(int pi = 0; pi < groundedPieces[i].size(); pi++)
				{
					groundedPieces[i][pi].rect.y += 16;
				}
			}
			clears++;
			_clears++;
			resortLanes();
		}
	}

	if (clears == 4)
		_clears += 1.5;

	if (level + 1 < 9 && _clears >= 15)
	{
		level++;
		setLevel(std::to_string(level + 1));
		_clears -= 15;
	}

	_score += clears * 100;
	setScore(std::to_string(_score));
	if (clears == 4)
		setMid("Tetris!");
	else if (clears > 0)
		setMid("Clear!");
}

bool place = false;

std::vector<Tetromino*> menuMinos;

void createMinos()
{
	menuMinos.clear();
	for(int y = 0; y < 10; y++)
	{
		for(int i = 0; i < 10; i++)
		{
			Tetromino* tetr = new Tetromino();
			tetr->constructIndex = rand() % 6;
			tetr->rect.w = 16;
			tetr->rect.h = 16;

			int colIndex = 0;
			int lanes = 1;

			for (int ii = 0; ii < 4; ii++)
			{
				int piece = tetrominos[tetr->constructIndex][ii];
				int bottomPiece = tetrominos[tetr->constructIndex][ii + 4];
				if (piece != 0)
					tetr->addPiece(0, colIndex, piece);
				if (bottomPiece != 0)
				{
					tetr->addPiece(1, colIndex, bottomPiece);
					lanes = 2;
				}

				if (piece != 0 || bottomPiece != 0)
					colIndex++;

			}
			tetr->rect.w = colIndex * 16;
			tetr->rect.h = lanes * 16;
			tetr->rect.x = ((95 * i) + 35);
			tetr->rect.y = ((95 * y) + 35);
			tetr->storeY = y;
			tetr->storeX = i;
			menuMinos.push_back(tetr);
		}	
	}
}

int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR lpCmdLine,
	INT nCmdShow)
{
	srand((unsigned)time(0));
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	scrollingRect.x = 0;
	scrollingRect.y = 0;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");


	SDL_Window* window = SDL_CreateWindow("Da window", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 864, 672, SDL_WINDOW_SHOWN);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	Globals::renderer = renderer;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	bool run = true;

	font = TTF_OpenFont("PublicPixel-0W6DP.TTF",18);

	setScore("0");
	setLevel("1");

	float hscore = 0;

	float logoW, logoH;
	float pressW, pressH;
	float highscoreW, highscoreH;

	high.open("highscore.dat");

	if (!high.is_open())
	{
		std::ofstream outfile ("highscore.dat");
		std::cout << "creating new file" << std::endl;
		outfile << "0" << std::endl;
		outfile.close();
		high.open("highscore.dat");
	}

	createAndSetText("Tetris!", &tetrisText, &logoW, &logoH);

	createAndSetText("Press space to play", &pressToPlay, &pressW, &pressH);

	createAndSetText("Start Level " + std::to_string(level + 1) + "  Press left or right", &startLevel, &startLevelW, &startLevelH);

	std::string line;
	while(std::getline(high, line))
	{
		hscore = std::stoi(line); // if it isn't a number then bruh moment
		lastHighscore = hscore;
		break;
	}

	createAndSetText("Highscore " + std::to_string((int)hscore), &highscore, &highscoreW, &highscoreH);

	createTetr(rand() % 6);

	bool skip = false;
	

	createMinos();
	

	while (run)
	{
		const Uint32 startTime = SDL_GetTicks();
		SDL_RenderClear(renderer);
		SDL_Event event;

		while (SDL_PollEvent(&event) > 0)
		{
			switch (event.type) {
				case SDL_QUIT:
					run = false;
					break;
				case SDL_KEYDOWN:
					if (playing)
					{
						switch (event.key.keysym.sym)
						{
						case SDLK_UP:
							if (faillingTetr)
							{
								rotateTime = 500;
								faillingTetr->rotate();
								checkBoardCol();
							}
							break;
						case SDLK_LEFT:
							if (faillingTetr)
							{
								faillingTetr->rect.x -= 16;
								checkBoardCol();
							}
							break;
						case SDLK_RIGHT:
							if (faillingTetr)
							{
								faillingTetr->rect.x += 16;
								checkBoardCol();
							}
							break;
						case SDLK_r:
							random = !random;
							setMid("Random toggled!");
							break;
						case SDLK_ESCAPE:
							for(int i = 0; i < 21; i++)
								groundedPieces[i].clear();
							playing = false;
							line = "";
							while(std::getline(high, line))
							{
								hscore = std::stoi(line); // if it isn't a number then bruh moment
								lastHighscore = hscore;
								break;
							}
							level = 0;
							createAndSetText("Start Level " + std::to_string(level + 1) + "  Press left or right", &startLevel, &startLevelW, &startLevelH);
							createAndSetText("Highscore " + std::to_string((int)hscore), &highscore, &highscoreW, &highscoreH);

							break;
						case SDLK_DOWN:
							if (!gameover)
							{
								rotateTime = 500;
								skip = true;
							}
							break;
						case SDLK_c:
							if (!gameover)
								holdPiece();
							break;
						case SDLK_SPACE:
							if (!gameover)
							{
								Tetromino* lowestPiece = lowestPos();

								faillingTetr->rect.y = lowestPiece->rect.y;

								delete lowestPiece;

								if (faillingTetr)
								{
									std::cout << "place" << std::endl;
									placePiece();

									delete faillingTetr;
									faillingTetr = nullptr;
								}
							}
							break;
						case SDLK_RETURN:
							if (gameover)
							{
								gameover = false;
								for(int i = 0; i < 21; i++)
									groundedPieces[i].clear();
								setMid("Start!");
								_score = 0;
								level = 0;
								setScore("0");
								setLevel("1");
							}
						break;
						}
					}
					else
					{
						switch (event.key.keysym.sym)
						{
							case SDLK_SPACE:
							playing = true;
							setMid("Start!");
							setLevel(std::to_string(level + 1));
							if (faillingTetr)
							{
								delete faillingTetr;
								faillingTetr = nullptr;
							}
							break;
							case SDLK_LEFT:
								level--;
								if (level < 0)
									level = 0;

								createAndSetText("Start Level " + std::to_string(level + 1) + "  Press left or right", &startLevel, &startLevelW, &startLevelH);
							break;
							case SDLK_RIGHT:
								level++;
								if (level > 9)
									level = 9;
								createAndSetText("Start Level " + std::to_string(level + 1) + "  Press left or right", &startLevel, &startLevelW, &startLevelH);

							break;
						}
					}
					break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
					case SDLK_DOWN:
						skip = false;
						break;
					}
					break;
			}
		

		}
		if (playing)
		{
			SDL_FRect scoreDst;
			scoreDst.x = (boardX - scoreW) -24;
			scoreDst.y = 192;
			scoreDst.w = scoreW;
			scoreDst.h = scoreH;

			SDL_FRect lvlDst;
			lvlDst.x = (boardX - levelW) -24;
			lvlDst.y = (192 + scoreH) + 24;
			lvlDst.w = levelW;
			lvlDst.h = levelH;

			SDL_RenderCopyF(Globals::renderer,score,NULL,&scoreDst);
			SDL_RenderCopyF(Globals::renderer,levelt,NULL,&lvlDst);

			if (score == NULL)
			std::cout << "score is null" << std::endl;

			float prog = 0;
			if (mTime > 0)
				prog = (mTime / 500);
			
			if (middleText && !gameover)
			{
				if (mTime > 0)
					mTime -= deltaTime * 0.6;
				SDL_SetTextureAlphaMod(middleText, prog * 255);
			}

			currentTime += deltaTime;

			if (rotateTime > 0)
			{
				rotateTime -= deltaTime;
			}

			if (faillingTetr && !gameover)
			{
				ghostTetr = lowestPos();
				
				for(int i = 0; i < ghostTetr->pieces.size(); i++)
				{
					TetrominoPiece& piece = ghostTetr->pieces[i];
					piece.color.r = 255;
					piece.color.g = 255;
					piece.color.b = 255;
					piece.color.a = 128;
				}

				// collision

				bool die = false;
				bool willGoUnder = false;
				for (TetrominoPiece piece : faillingTetr->pieces)
				{
					if (faillingTetr->rect.y + piece.rect.y >= bottomBoard - 16)
						willGoUnder = true;
				}

				for (int i = 0; i < 21; i++)
				{
					for (TetrominoPiece& piece : groundedPieces[i])
					{
						if (faillingTetr->checkCol(piece))
						{
							willGoUnder = true;
						}
					}
				}
				if (!willGoUnder)
				{
					if (!skip)
						while (currentTime > simulatedTime)
						{
							faillingTetr->storeY++;
							faillingTetr->rect.y = align(faillingTetr->storeY, 16);
							simulatedTime += 1 / cellFrames[level][0];
							std::cout << "time division " << cellFrames[level][0] << " " << level << std::endl;
							rotateTime = 500;
						}

					if (skip && SDL_GetTicks() % 10 == 0)
					{
						rotateTime = 500;
						faillingTetr->storeY++;
						faillingTetr->rect.y = align(faillingTetr->storeY, 16);
					}
				}

				for (int i = 0; i < 21; i++)
				{
					for (TetrominoPiece& piece : groundedPieces[i])
					{
						if (faillingTetr->checkCol(piece))
						{
							faillingTetr->rect.y -= 16;
							placePiece();
							die = true;
							break;
						}
					}
				}


				for (TetrominoPiece piece : faillingTetr->pieces)
				{
					if (faillingTetr->rect.y + piece.rect.y >= bottomBoard - 16 && rotateTime <= 0)
					{
						placePiece();
						faillingTetr->rect.y = bottomBoard - 16;
						die = true;
						break;
					}
				}

				if (die)
				{
					delete faillingTetr;
					faillingTetr = nullptr;
				}

				if (faillingTetr)
				{
					faillingTetr->draw();
					ghostTetr->draw();
				}
				

			}
			else
			{
				if (!gameover)
				{
					createTetr(rand() % 6);
					if (faillingTetr->constructIndex == lastIndex)
					{
						delete faillingTetr;
						faillingTetr = nullptr;
						createTetr(rand() % 6);
					}
					lastIndex = faillingTetr->constructIndex;
				}
			}

			for (int i = 0; i < 21; i++)
			{
				for (TetrominoPiece& piece : groundedPieces[i])
				{
					SDL_SetRenderDrawColor(Globals::renderer, piece.color.r, piece.color.g, piece.color.b, piece.color.a);
					SDL_FRect newRect;
					newRect.x = piece.rect.x;
					newRect.y = piece.rect.y;
					newRect.w = piece.rect.w;
					newRect.h = piece.rect.h;
					SDL_RenderFillRectF(Globals::renderer, &newRect);
					SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, 255);
					SDL_RenderDrawRectF(Globals::renderer, &newRect);
				}
			}

			if (middleText)
			{
				SDL_FRect midDst;
				midDst.x = 432 - (midW / 2);
				midDst.y = 336 - (midH / 2);
				midDst.w = midW;
				midDst.h = midH;

				SDL_RenderCopyF(Globals::renderer,middleText,NULL,&midDst);
			}


			// render box

			bool remove = false;
			int index = 0;

			for(Globals::lineClearAnim& anim : lineClearAnims)
			{
				anim.time += deltaTime * 5;
				float f = anim.time / anim.dur;
				float alphaValue = 255 - (255 * f);
				SDL_SetRenderDrawColor(Globals::renderer,255,255,255,alphaValue);
				SDL_RenderFillRectF(Globals::renderer,&anim.rect);
				if (f >= 1)
				{
					remove = true;
				}
				index++;
			}
			if (remove)
			{
				lineClearAnims.erase(lineClearAnims.begin() + index - 1);
			}

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			SDL_FPoint p[4];

			p[0].x = boardX;
			p[0].y = 192;
			p[1].x = boardX;
			p[1].y = bottomBoard;
			p[2].x = boardX + 160;
			p[2].y = bottomBoard;
			p[3].x = boardX + 160;
			p[3].y = 192;

			SDL_RenderDrawLinesF(renderer, p, 4);

		}
		else
		{
			// bg minos

			if (!startTexture)
				startTexture = SDL_CreateTexture(Globals::renderer, SDL_PIXELFORMAT_ARGB32,SDL_TEXTUREACCESS_TARGET,1280,720);

			SDL_SetRenderTarget(Globals::renderer,startTexture);
			for(Tetromino* mino : menuMinos)
				mino->draw();
			SDL_SetRenderTarget(Globals::renderer,NULL);
		
			scrollingRect.x = 0;
			scrollingRect.y = 0;
			scrollingRect.w = 1280;
			scrollingRect.h = 760;

			SDL_RenderCopyF(Globals::renderer,startTexture,NULL,&scrollingRect);

			SDL_FRect background;
			background.x = 0;
			background.y = 0;
			background.w = 1280;
			background.h = 720;

			SDL_SetRenderDrawColor(Globals::renderer,0,0,0,64);
			SDL_RenderFillRectF(Globals::renderer, &background);
			SDL_SetRenderDrawColor(Globals::renderer,255,255,255,255);
			
			SDL_FRect tetrisDst;
			tetrisDst.x = 440 - ((logoW * 2) / 2);
			tetrisDst.y = 235 - ((logoH * 2) / 2);
			tetrisDst.w = logoW * 2;
			tetrisDst.h = logoH * 2;

			SDL_RenderCopyF(renderer,tetrisText,NULL,&tetrisDst);

			SDL_FRect hscoreDst;
			hscoreDst.x = 432 - (highscoreW / 2);
			hscoreDst.y = 350 - (highscoreH / 2);
			hscoreDst.w = highscoreW;
			hscoreDst.h = highscoreH;

			SDL_RenderCopyF(renderer,highscore,NULL,&hscoreDst);

			SDL_FRect pressToPlayDst;
			pressToPlayDst.x = 432 - (pressW / 2);
			pressToPlayDst.y = 470 - (pressH / 2);
			pressToPlayDst.w = pressW;
			pressToPlayDst.h = pressH;

			SDL_RenderCopyF(renderer,pressToPlay,NULL,&pressToPlayDst);

			SDL_FRect startLevelWW;
			startLevelWW.x = 432 - (startLevelW / 2);
			startLevelWW.y = 500 - (startLevelH / 2);
			startLevelWW.w = startLevelW;
			startLevelWW.h = startLevelH;

			SDL_RenderCopyF(renderer,startLevel,NULL,&startLevelWW);
			
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderPresent(renderer);

		deltaTime = SDL_GetTicks() - startTime;

	}

	high.close();

	return 0;

}
