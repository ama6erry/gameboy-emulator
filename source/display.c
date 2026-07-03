#include <SDL.h>
#include <SDL_ttf.h>
#include "CPU.h"
#include "utils.h"
#include <stdio.h>

SDL_Window* debugWindow;
SDL_Surface* debugSurface;
SDL_Renderer* debugRenderer;
SDL_Texture* debugTexture;
TTF_Font* font;
SDL_Color fontColor = {0, 0, 0, 255};

void init_debug_display(){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    font = TTF_OpenFont("font.ttf", 15);
    debugWindow = SDL_CreateWindow("GBEMU - Debug Info", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 300, 500, SDL_WINDOW_SHOWN);
    debugRenderer = SDL_CreateRenderer(debugWindow, -1, SDL_RENDERER_ACCELERATED);
}

void update_debug_window(){
    unsigned short AF = register_AF.full;
    unsigned short BC = register_BC.full;
    unsigned short DE = register_DE.full;
    unsigned short HL = register_HL.full;
    unsigned short SP = sp.full;
    unsigned short PC = pc;
    
    static const char *template = 
    "registers\n"
    "AF = $%04X\n"
    "BC = $%04X\n"
    "DE = $%04X\n"
    "HL = $%04X\n"
    "SP = $%04X\n"
    "PC = $%04X\n";

    

    char info[256];

    snprintf(info, 256, template, AF, BC, DE, HL, SP, PC);

    debugSurface = TTF_RenderText_Blended_Wrapped(font, info, fontColor, 0);
    if (!debugSurface) {
        LOG_E("TTF_RenderText_Blended_Wrapped failed: %s\n", TTF_GetError());
        return;
    }
    debugTexture = SDL_CreateTextureFromSurface(debugRenderer, debugSurface);
    if (!debugTexture) {
        LOG_E("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        SDL_FreeSurface(debugSurface);
        return;
    }

    SDL_Rect textRect = { 10, 10, debugSurface->w, debugSurface->h };

    SDL_SetRenderDrawColor(debugRenderer, 255, 255, 255, 255);
    SDL_RenderClear(debugRenderer);

    SDL_RenderCopy(debugRenderer, debugTexture, NULL, &textRect);

    SDL_RenderPresent(debugRenderer);
}

void destroy_debug_display(){
    SDL_DestroyTexture(debugTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(debugRenderer);
    SDL_DestroyWindow(debugWindow);
    TTF_Quit();
    SDL_Quit();
}