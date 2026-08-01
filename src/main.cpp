#include <iostream>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;

constexpr int MAP_WIDTH = 20;
constexpr int MAP_HEIGHT = 20;

constexpr int FRAMETIME = 200;

bool drawGrid = false;
bool lightMode = false;
bool quit = false;

enum Direction
{
	None,
	Up,
	Down,
	Left,
	Right,
};

struct iVec2
{
	int x{};
	int y{};
};

struct GameState
{
	iVec2 snakeTailPos[MAP_WIDTH * MAP_HEIGHT]{};
	int snakeTailLen{};
	iVec2 snakePos{};
	Direction direction{};
	iVec2 fruitPos{};
	int score{};
	bool gameover{};
};

void setup(GameState* gameState);
void getInput(GameState* gameState);
void logic(GameState* gameState);
void draw(GameState* gameState, SDL_Renderer* renderer);

int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--draw-grid") == 0) drawGrid = true;
		if (strcmp(argv[i], "--light-mode") == 0) lightMode = true;
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	if (!SDL_CreateWindowAndRenderer("Snake", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer))
	{
		SDL_Log("Could not create window or renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	while (!quit)
	{
		std::time_t startTime = std::time(nullptr);

		GameState gameState{};
		setup(&gameState);
		while (!gameState.gameover && !quit)
		{
			getInput(&gameState);
			logic(&gameState);
			draw(&gameState, renderer);
			SDL_Delay(FRAMETIME);
		}

		std::time_t endTime = std::time(nullptr) - startTime;
		std::string message = "Game Over!\nYour final score was " + std::to_string(gameState.score) + "!\n";
		message.append("You survived for " + std::to_string(endTime) + " seconds!");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over!", message.c_str(), window);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void setup(GameState* gameState)
{
	srand(time(nullptr));
	gameState->gameover = false;
	gameState->score = 0;
	gameState->snakeTailLen = 0;

	gameState->snakePos.x = MAP_WIDTH / 2;
	gameState->snakePos.y = MAP_HEIGHT / 2;

	gameState->direction = None;

	gameState->fruitPos.x = rand() % (MAP_WIDTH - 2) + 1;
	gameState->fruitPos.y = rand() % (MAP_HEIGHT - 2) + 1;
}


void getInput(GameState* gameState)
{
	SDL_Event event{};
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			quit = true;
		}
		if (event.type == SDL_EVENT_KEY_DOWN)
		{
			if (event.key.key == SDLK_ESCAPE)
			{
				quit = true;
			}
			if ((event.key.key == SDLK_UP || event.key.key == SDLK_W) && gameState->direction != Down)
			{
				gameState->direction = Up;
			}
			if ((event.key.key == SDLK_DOWN || event.key.key == SDLK_S) && gameState->direction != Up)
			{
				gameState->direction = Down;
			}
			if ((event.key.key == SDLK_LEFT || event.key.key == SDLK_A) && gameState->direction != Right)
			{
				gameState->direction = Left;
			}
			if ((event.key.key == SDLK_RIGHT || event.key.key == SDLK_D) && gameState->direction != Left)
			{
				gameState->direction = Right;
			}
		}
	}
}

void logic(GameState* gameState)
{
	// Eating Fruit
	if (gameState->snakePos.x == gameState->fruitPos.x && gameState->snakePos.y == gameState->fruitPos.y)
	{
		gameState->score += 1;
		gameState->snakeTailLen++;
		gameState->fruitPos.x = rand() % (MAP_WIDTH - 2) + 1;
		gameState->fruitPos.y = rand() % (MAP_HEIGHT - 2) + 1;
	}

	// Logic for tail following
	int prevX = gameState->snakeTailPos[0].x;
	int prevY = gameState->snakeTailPos[0].y;
	int prev2X{}, prev2Y{};
	gameState->snakeTailPos[0].x = gameState->snakePos.x;
	gameState->snakeTailPos[0].y = gameState->snakePos.y;

	for (int i = 1; i < gameState->snakeTailLen; i++)
	{
		prev2X = gameState->snakeTailPos[i].x;
		prev2Y = gameState->snakeTailPos[i].y;
		gameState->snakeTailPos[i].x = prevX;
		gameState->snakeTailPos[i].y = prevY;
		prevX = prev2X;
		prevY = prev2Y;
	}

	// Movement
	switch (gameState->direction)
	{
	case None:
		break;
	case Up:
		gameState->snakePos.y--;
		break;
	case Down:
		gameState->snakePos.y++;
		break;
	case Left:
		gameState->snakePos.x--;
		break;
	case Right:
		gameState->snakePos.x++;
		break;
	}

	// Wall Collision
	if (gameState->snakePos.x < 0 || gameState->snakePos.x >= MAP_WIDTH || gameState->snakePos.y < 0 || gameState->snakePos.y >= MAP_HEIGHT)
	{
		gameState->gameover = true;
	}

	// Tail Collision
	for (int i = 0; i < gameState->snakeTailLen; i++)
	{
		if (gameState->snakeTailPos[i].x == gameState->snakePos.x && gameState->snakeTailPos[i].y == gameState->snakePos.y)
		{
			gameState->gameover = true;
		}
	}
}

void draw(GameState* gameState, SDL_Renderer* renderer)
{
	/*
	 * Draw fruit above tail to avoid it being covered it it spawns on the same tile.
	 * Draw head above fruit to avoid looking weird while on the same tile.
	 */
	if (lightMode)
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	else
		SDL_SetRenderDrawColor(renderer, 51, 51, 51, 255);
	SDL_RenderClear(renderer);

	if (drawGrid)
	{
		if (lightMode)
			SDL_SetRenderDrawColor(renderer, 51, 51, 51, 255);
		else
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		const int verticalLineSpacing = WINDOW_WIDTH / MAP_WIDTH;
		for (int i = 1; i < MAP_WIDTH; i++)
		{
			SDL_RenderLine(renderer, static_cast<float>(i * verticalLineSpacing), 0, static_cast<float>(i * verticalLineSpacing), WINDOW_HEIGHT);
		}
		const int horizontalLineSpacing = WINDOW_HEIGHT / MAP_HEIGHT;
		for (int i = 1; i < MAP_HEIGHT; i++)
		{
			SDL_RenderLine(renderer, 0, static_cast<float>(i * horizontalLineSpacing), WINDOW_WIDTH, static_cast<float>(i * horizontalLineSpacing));
		}
	}

	const int cellWidth = WINDOW_WIDTH / MAP_WIDTH;
	const int cellHeight = WINDOW_HEIGHT / MAP_HEIGHT;
	// Render tail
	std::vector<SDL_FRect> tailRects{};
	for (int i = 0; i < gameState->snakeTailLen; i++)
	{
		SDL_FRect tail{};
		tail.x = static_cast<float>(gameState->snakeTailPos[i].x * cellWidth);
		tail.y = static_cast<float>(gameState->snakeTailPos[i].y * cellHeight);
		tail.w = cellWidth;
		tail.h = cellHeight;
		tailRects.push_back(tail);
	}
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderFillRects(renderer, tailRects.data(), static_cast<int>(tailRects.size()));

	// Render fruit
	SDL_FRect fruit{};
	fruit.x = static_cast<float>(gameState->fruitPos.x * cellWidth);
	fruit.y = static_cast<float>(gameState->fruitPos.y * cellHeight);
	fruit.w = cellWidth;
	fruit.h = cellHeight;
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(renderer, &fruit);

	// Render head
	SDL_FRect head{};
	head.x = static_cast<float>(gameState->snakePos.x * cellWidth);
	head.y = static_cast<float>(gameState->snakePos.y * cellHeight);
	head.w = cellWidth;
	head.h = cellHeight;
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	// Blend head and fruit color when eating fruit
	if (gameState->snakePos.x == gameState->fruitPos.x && gameState->snakePos.y == gameState->fruitPos.y)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	}
	SDL_RenderFillRect(renderer, &head);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

	SDL_RenderPresent(renderer);
}