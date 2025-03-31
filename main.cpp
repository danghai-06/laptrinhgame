#include    <SDL.h>
#include<SDL
#include <iostream>
using namespace std;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TITLE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH/ TITLE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TITLE_SIZE;
class Game {
public:
    SDL_Window* windows;
    SDL_Renderer* renderer;
    bool runing;

    Game() {
    bool running = true ;
 if(SDL_Init(SDL_INIT_VIDEO) < 0){
cerr<<"SDL  COULD NOT INITIALIZE ! SDL ERROR : "<<SDL_GetError()<<endl;
running = false ;

     }

    sdl_window window = SDL_Create_Window("Battle City ",  SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH , SCREEN_HEIGHT , SDL_WINDOW_SHOWN);
     if(!window){
        cerr<<"Window could not be created ! SDL_Error: "<< SDL_GetError()<<endl;
        running = false ;
     }
     renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
     if(!render){
        cerr<<"Renderer could not be created ! SDL_Error:   "<<SDL_GetError() <<endl;
        running = false ;
     }
     }
    void render() {
    SDL__SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for(int i = 1 ; i < MAP_HEIGHT - 1 ;++i){
         for(int j = 1 ; j < MAP_WIDTH - 1 ; ++j){
            SDL_Rect title = { j * TITLE_SIZE, i * TITLE_SIZE, TITLE_SIZE, TITLE_SIZE};
            SDL_RenderFillRect(renderer, &tile);
         }
    }
    SDL_RenderPresent(renderer);
    }
    void run() {
    while (running){
        render() ;
        SDL_Delay(16);
    }
    }
    ~Game() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    }

};



int main(int argc , char* argv[] ){
 Game game ;
 if(game.running){
    game.run();
 }
 return 0;

}

}




