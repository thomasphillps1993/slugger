// Slug Game in C using SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TRAIL_LENGTH 20
#define MAX_POOS 100 // Maximum number of poos on screen

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
const int BLOCK_SIZE = 20;
const int SLUG_SPEED = 150; // Slower movement
const int POO_DROP_CHANCE = 33; // 20% chance to drop poo after eating

typedef struct {
    int x, y;
} Point;

Point slug[100];
int food_eaten = 0; // Simplified food counter
Point food;
Point poos[MAX_POOS];
int poo_count = 0;

Point trail[MAX_TRAIL_LENGTH];
int trail_count = 0;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

int directionX = 0;
int directionY = 0;
bool can_poo = false;

void initSlug() {
    slug[0].x = SCREEN_WIDTH / 2;
    slug[0].y = SCREEN_HEIGHT / 2;
    food_eaten = 0;
    poo_count = 0;
    directionX = BLOCK_SIZE;
    directionY = 0;
    trail_count = 0;
    can_poo = false;
    for (int i = 0; i < MAX_TRAIL_LENGTH; i++) {
        trail[i].x = -BLOCK_SIZE;
        trail[i].y = -BLOCK_SIZE;
    }
}

void placeFood() {
    food.x = (rand() % (SCREEN_WIDTH / BLOCK_SIZE)) * BLOCK_SIZE;
    food.y = (rand() % (SCREEN_HEIGHT / BLOCK_SIZE)) * BLOCK_SIZE;
}

void renderText(const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void startScreen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255};
    renderText("Slug it out", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 50, white);
    renderText("Press Enter to Start", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 20, white);

    SDL_RenderPresent(renderer);

    SDL_Event event;
    bool waiting = true;
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) exit(0);
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) waiting = false;
        }
    }
}

void dropPoo() {
    if (can_poo && poo_count < MAX_POOS) {
        poos[poo_count++] = trail[trail_count - 1];
        can_poo = false;
    }
}

void moveSlug() {
    for (int i = trail_count - 1; i > 0; i--) trail[i] = trail[i - 1];
    if (trail_count < MAX_TRAIL_LENGTH) trail_count++;
    trail[0] = slug[0];

    for (int i = food_eaten; i > 0; i--) slug[i] = slug[i - 1];
    slug[0].x += directionX;
    slug[0].y += directionY;

    dropPoo();

    if (slug[0].x < 0) slug[0].x = SCREEN_WIDTH - BLOCK_SIZE;
    if (slug[0].x >= SCREEN_WIDTH) slug[0].x = 0;
    if (slug[0].y < 0) slug[0].y = SCREEN_HEIGHT - BLOCK_SIZE;
    if (slug[0].y >= SCREEN_HEIGHT) slug[0].y = 0;

    if (slug[0].x == food.x && slug[0].y == food.y) {
        food_eaten++;
        placeFood();
        can_poo = true;
    }

    for (int i = 1; i < trail_count; i++) {
        if (slug[0].x == trail[i].x && slug[0].y == trail[i].y) {
            printf("Game Over! You hit your own trail.\n");
            exit(0);
        }
    }

    for (int i = 0; i < poo_count; i++) {
        if (slug[0].x == poos[i].x && slug[0].y == poos[i].y) {
            printf("Game Over! You slipped on poo!\n");
            exit(0);
        }
    }
}

void gameLoop() {
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:    directionX = 0; directionY = -BLOCK_SIZE; break;
                    case SDLK_DOWN:  directionX = 0; directionY = BLOCK_SIZE; break;
                    case SDLK_LEFT:  directionX = -BLOCK_SIZE; directionY = 0; break;
                    case SDLK_RIGHT: directionX = BLOCK_SIZE; directionY = 0; break;
                    case SDLK_ESCAPE: running = false; break;
                }
            }
        }

        moveSlug();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (int i = 0; i < trail_count; i++) {
            SDL_Rect trailRect = {trail[i].x, trail[i].y, BLOCK_SIZE, BLOCK_SIZE};
            SDL_RenderFillRect(renderer, &trailRect);
        }

        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        for (int i = 0; i <= food_eaten; i++) {
            SDL_Rect slugRect = {slug[i].x, slug[i].y, BLOCK_SIZE, BLOCK_SIZE};
            SDL_RenderFillRect(renderer, &slugRect);
        }

        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
        for (int i = 0; i < poo_count; i++) {
            SDL_Rect pooRect = {poos[i].x, poos[i].y, BLOCK_SIZE, BLOCK_SIZE};
            SDL_RenderFillRect(renderer, &pooRect);
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect foodRect = {food.x, food.y, BLOCK_SIZE, BLOCK_SIZE};
        SDL_RenderFillRect(renderer, &foodRect);

        char counterText[20];
        sprintf(counterText, "Food Eaten: %d", food_eaten);
        SDL_Color white = {255, 255, 255};
        renderText(counterText, 10, 10, white);

        SDL_RenderPresent(renderer);
        SDL_Delay(SLUG_SPEED);
    }
}

int main() {
    srand(time(0));
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
    window = SDL_CreateWindow("A slow game of Snake!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initSlug();
    placeFood();

    startScreen();
    gameLoop();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
