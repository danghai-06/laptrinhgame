#include <SDL.h>
#include<SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <SDL_mixer.h>

using namespace std;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

enum GameState {
    STATE_START_SCREEN,
    STATE_GAME_RUNNING,
    STATE_GAME_OVER
};

class Wall;
class Bullet;
class PlayerTank;
class Player1Tank;
class Player2Tank;
class EnemyTank;
class Button;

class Button {
public:
    SDL_Rect rect;
    string text;
    bool isHovered;

    Button(int x, int y, int w, int h, string buttonText) {
        rect = {x, y, w, h};
        text = buttonText;
        isHovered = false;
    }

    bool isMouseOver(int mouseX, int mouseY) {
        if (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
            mouseY >= rect.y && mouseY <= rect.y + rect.h) {
            isHovered = true;
            return true;
        }
        isHovered = false;
        return false;
    }

    void render(SDL_Renderer* renderer, TTF_Font* font) {
        if (isHovered) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
        }
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);

        if (font) {
            SDL_Color textColor = {0, 0, 0, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = {
                        rect.x + (rect.w - textSurface->w) / 2,
                        rect.y + (rect.h - textSurface->h) / 2,
                        textSurface->w,
                        textSurface->h
                    };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
    }
};

class Bullet {
public:
   int x, y;
   int dx, dy;
   SDL_Rect rect;
   bool active;
   int ownerID;


   Bullet(int startX, int startY, int dirX, int dirY, int owner) {
      x = startX;
      y = startY;
      dx = dirX * 10;
      dy = dirY * 10;
      active = true;
      rect = {x, y, 10, 10};
      ownerID = owner;
   }

   void move() {
      x += dx;
      y += dy;
      rect.x = x;
      rect.y = y;
      if(x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE || y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE) {
            active = false;
      }
   }

   void render(SDL_Renderer* renderer) {
       if(active) {

      if(ownerID == 0) {
         SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
      }
      else if(ownerID == 1) {
         SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
      }
      else {
         SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      }


      SDL_RenderFillRect(renderer, &rect);
   }
   }
};

class Wall {
public:
   int x, y;
   SDL_Rect rect;
   bool active;

   Wall(int startX, int startY) {
      x = startX;
      y = startY;
      active = true;
      rect = {x, y, TILE_SIZE, TILE_SIZE};
   }

   void render(SDL_Renderer* renderer) {
      if(active) {
            SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
      }
   }
};

class PlayerTank {
public:
   int x, y;
   int dirX, dirY;
   SDL_Rect rect;
   vector<Bullet> bullets;
   bool active;
   int playerID;
   SDL_Color color;

   SDL_Texture* tankTexture;
   SDL_Rect srcRect;
   int angle;
   PlayerTank() : x(0), y(0), dirX(0), dirY(-1), active(true), playerID(0) {
      rect = {x, y, TILE_SIZE, TILE_SIZE};
      color = {255, 255, 0, 255};
   }

   PlayerTank(int startX, int startY, int id) {
      x = startX;
      y = startY;
      rect = {x, y, TILE_SIZE, TILE_SIZE};
      dirX = 0;
      dirY = -1;
      active = true;
      playerID = id;


      if(playerID == 0) {
            color = {255, 255, 0, 255};
      } else {
            color = {0, 255, 255, 255};
      }


      if(playerID == 1) {
            dirY = 1;
      }
   }

   void move(int dx, int dy, const vector<Wall>& walls) {
      int newX = x + dx;
      int newY = y + dy;


      if (dx != 0 && dy == 0) {
            dirX = (dx > 0) ? 1 : -1;
            dirY = 0;
      }
      else if (dy != 0 && dx == 0) {
            dirY = (dy > 0) ? 1 : -1;
            dirX = 0;
      }


      SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
      for(int i = 0; i < walls.size(); i++) {
            if(walls[i].active && SDL_HasIntersection(&newRect, &walls[i].rect)) {
               return;
            }
      }

      if(newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
         newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
      }
   }

   void shoot() {
      bullets.push_back(Bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, playerID));
   }

   void updateBullets() {
      for(auto &bullet : bullets) {
            bullet.move();
      }
      bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet &b) {return !b.active;}), bullets.end());
   }

   void render(SDL_Renderer* renderer) {
   if(active) {
      if(tankTexture) {
         SDL_Rect destRect = {x, y, TILE_SIZE, TILE_SIZE};
         if(dirX == 1) angle = 90;
         else if(dirX == -1) angle = 270;
         else if(dirY == 1) angle = 180;
         else if(dirY == -1) angle = 0;
         SDL_RenderCopyEx(renderer, tankTexture, NULL, &destRect, angle, NULL, SDL_FLIP_NONE);
      } else {
         SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
         SDL_RenderFillRect(renderer, &rect);
      }
      for(auto &bullet : bullets) {
         bullet.render(renderer);
      }
   }
}

   bool checkCollision(const PlayerTank& other) {
      return SDL_HasIntersection(&rect, &other.rect);
   }
};
class Player1Tank : public PlayerTank {
public:
   Player1Tank(int startX, int startY) : PlayerTank(startX, startY, 0) {
      dirY = -1;
      dirX = 0;
      angle = 0;
   }
};

class Player2Tank : public PlayerTank {
public:
   Player2Tank(int startX, int startY) : PlayerTank(startX, startY, 1) {
      dirY = 1;
      dirX = 0;
      angle = 180;
   }
};

class EnemyTank {
public:
   int x, y;
   int dirX, dirY;
   int moveDelay, shootDelay;
   SDL_Rect rect;
   bool active;
   vector<Bullet> bullets;

   EnemyTank(int startX, int startY) {
      moveDelay = 15;
      shootDelay = 5;
      x = startX;
      y = startY;
      rect = {x, y, TILE_SIZE, TILE_SIZE};
      dirX = 0;
      dirY = 1;
      active = true;
   }

   void move(const vector<Wall>& walls) {
      if(--moveDelay > 0) return;
      moveDelay = 15;
      int r = rand() % 4;
      if(r == 0) {
            dirX = 0;
            dirY = -1;
      }
      else if(r == 1) {
            dirX = 0;
            dirY = 1;
      }
      else if(r == 2) {
            dirY = 0;
            dirX = -1;
      }
      else if(r == 3) {
            dirY = 0;
            dirX = 1;
      }

      int newX = x + dirX * 5;
      int newY = y + dirY * 5;

      SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
      for(const auto& wall : walls) {
            if(wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
               return;
            }
      }

      if(newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
         newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
      }
   }

   void shoot() {
      if(--shootDelay > 0) return;
      shootDelay = 5;
      bullets.push_back(Bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, 2)); // 2 represents enemy bullets
   }

   void updateBullets() {
      for(auto &bullet : bullets) {
            bullet.move();
      }
      bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet &b) {return !b.active;}), bullets.end());
   }

   void render(SDL_Renderer* renderer) {
      if (active) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
            for(auto &bullet : bullets) {
               bullet.render(renderer);
            }
      }
   }
};

class Game {
public:
   SDL_Window* window;
   SDL_Renderer* renderer;
   bool running;
   vector<Wall> walls;
   Player1Tank* player1;
   Player2Tank* player2;
   int enemyNumber = 3;
   vector<EnemyTank> enemies;
   bool gameOver;
   string winMessage;
   Mix_Music* backgroundMusic;

   SDL_Texture* player1TankTexture;
   SDL_Texture* player2TankTexture;
   SDL_Texture* enemyTankTexture;
   const Uint8* keyboardState;
   GameState currentState;
   TTF_Font* font;
   TTF_Font* titleFont;
    SDL_Texture* backgroundTexture;
   Button* startButton;
   Button* quitButton;
   Button* restartButton;
   Button* menuButton;

   Game() {
      running = true;
      gameOver = false;
      backgroundMusic = nullptr;
      keyboardState = SDL_GetKeyboardState(NULL);
      restartButton = new Button(SCREEN_WIDTH - 150, TILE_SIZE/2, 120, 40, "Restart");
      menuButton = new Button(20, TILE_SIZE/2, 120, 40, "Menu");

      currentState = STATE_START_SCREEN;

      if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
         cerr << "SDL COULD NOT INITIALIZE! SDL_ERROR: " << SDL_GetError() << endl;
         running = false;
      }

       int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
         if(!(IMG_Init(imgFlags) & imgFlags)) {
        cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
        running = false;
    }

      if(TTF_Init() < 0) {
         cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
         running = false;
      }

      if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
         cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
         running = false;
      }

      window = SDL_CreateWindow("Battle City - Two Players", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
      if(!window) {
         cerr << "WINDOW COULD NOT BE CREATED! SDL_ERROR: " << SDL_GetError() << endl;
         running = false;
      }
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
      if(!renderer) {
         cerr << "RENDERER COULD NOT BE CREATED! SDL_Error: " << SDL_GetError() << endl;
         running = false;
      }

       SDL_Surface* tempSurface = IMG_Load("image/tank.jpg");
    if(tempSurface) {
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
        SDL_FreeSurface(tempSurface);
        if(!backgroundTexture) {
            cerr << "Failed to create texture from background image! SDL Error: " << SDL_GetError() << endl;
        }
    } else {
        cerr << "Failed to load background image! SDL_image Error: " << IMG_GetError() << endl;
    }

      SDL_Surface* tank1Surface = IMG_Load("image/tank1.png");
   if(tank1Surface) {
       player1TankTexture = SDL_CreateTextureFromSurface(renderer, tank1Surface);
       SDL_FreeSurface(tank1Surface);
       if(!player1TankTexture) {
           cerr << "Failed to create texture for player 1 tank! SDL Error: " << SDL_GetError() << endl;
       }
   } else {
       cerr << "Failed to load player 1 tank image! SDL_image Error: " << IMG_GetError() << endl;
   }

   SDL_Surface* tank2Surface = IMG_Load("image/tank2.png");
   if(tank2Surface) {
       player2TankTexture = SDL_CreateTextureFromSurface(renderer, tank2Surface);
       SDL_FreeSurface(tank2Surface);
       if(!player2TankTexture) {
           cerr << "Failed to create texture for player 2 tank! SDL Error: " << SDL_GetError() << endl;
       }
   } else {
       cerr << "Failed to load player 2 tank image! SDL_image Error: " << IMG_GetError() << endl;
   }

      font = TTF_OpenFont("fonts/arial.ttf", 24);
      titleFont = TTF_OpenFont(" fonts/arial.ttf", 48);
      if(!font || !titleFont) {
         cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
      }
      startButton = new Button(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, 200, 60, "Start Game");
      quitButton = new Button(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 100, 200, 60, "Quit Game");

      backgroundMusic = Mix_LoadMUS("music/Ladyfingers.mp3");
      if(!backgroundMusic) {
         cerr << "Failed to load background music! SDL_mixer Error: " << Mix_GetError() << endl;
      }

      if(backgroundMusic) {
         Mix_PlayMusic(backgroundMusic, -1);  // -1 nghĩa là lặp vô hạn
      }

   }

   void initGame() {
      generateWalls();
      player1 = new Player1Tank(((MAP_WIDTH-1) / 2 - 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE);
      player2 = new Player2Tank(((MAP_WIDTH-1) / 2 + 2) * TILE_SIZE, 2 * TILE_SIZE);
      if(player1) player1->tankTexture = player1TankTexture;
      if(player2) player2->tankTexture = player2TankTexture;
      spawnEnemies();
      currentState = STATE_GAME_RUNNING;
   }

   void renderStartScreen() {
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);

      if(backgroundTexture) {
        SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect);
    }

      if(titleFont) {
         SDL_Color titleColor = {255, 255, 0, 255}; // Màu vàng
         string titleText = "BATTLE CITY";
         SDL_Surface* titleSurface = TTF_RenderText_Solid(titleFont, titleText.c_str(), titleColor);
         if(titleSurface) {
            SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
            if(titleTexture) {
               SDL_Rect titleRect = {
                  SCREEN_WIDTH/2 - titleSurface->w/2,
                  SCREEN_HEIGHT/4 - titleSurface->h/2,
                  titleSurface->w,
                  titleSurface->h
               };
               SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
               SDL_DestroyTexture(titleTexture);
            }
            SDL_FreeSurface(titleSurface);
         }

         SDL_Color infoColor = {255, 255, 255, 255}; // Màu trắng
         string controlsText = "Player 1: WASD to move, SPACE to shoot";
         SDL_Surface* controlsSurface = TTF_RenderText_Solid(font, controlsText.c_str(), infoColor);
         if(controlsSurface) {
            SDL_Texture* controlsTexture = SDL_CreateTextureFromSurface(renderer, controlsSurface);
            if(controlsTexture) {
               SDL_Rect controlsRect = {
                  SCREEN_WIDTH/2 - controlsSurface->w/2,
                  SCREEN_HEIGHT/2 - 80,
                  controlsSurface->w,
                  controlsSurface->h
               };
               SDL_RenderCopy(renderer, controlsTexture, NULL, &controlsRect);
               SDL_DestroyTexture(controlsTexture);
            }
            SDL_FreeSurface(controlsSurface);
         }

         string controls2Text = "Player 2: Arrow keys to move, + key to shoot";
         SDL_Surface* controls2Surface = TTF_RenderText_Solid(font, controls2Text.c_str(), infoColor);
         if(controls2Surface) {
            SDL_Texture* controls2Texture = SDL_CreateTextureFromSurface(renderer, controls2Surface);
            if(controls2Texture) {
               SDL_Rect controls2Rect = {
                  SCREEN_WIDTH/2 - controls2Surface->w/2,
                  SCREEN_HEIGHT/2 - 40,
                  controls2Surface->w,
                  controls2Surface->h
               };
               SDL_RenderCopy(renderer, controls2Texture, NULL, &controls2Rect);
               SDL_DestroyTexture(controls2Texture);
            }
            SDL_FreeSurface(controls2Surface);
         }
      }

      startButton->render(renderer, font);
      quitButton->render(renderer, font);

      SDL_RenderPresent(renderer);
   }

   void render() {
      if(currentState == STATE_START_SCREEN) {
         renderStartScreen();
         return;
      }
      SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
      SDL_RenderClear(renderer);

      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      for(int i = 1; i < MAP_HEIGHT - 1; ++i) {
            for(int j = 1; j < MAP_WIDTH - 1; ++j) {
               SDL_Rect tile = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE};
               SDL_RenderFillRect(renderer, &tile);
            }
      }

      for(int i = 0; i < walls.size(); i++) {
            walls[i].render(renderer);
      }

      if (player1 && player1->active) {
            player1->render(renderer);
      }

      if (player2 && player2->active) {
            player2->render(renderer);
      }

      for(auto &enemy : enemies) {
            enemy.render(renderer);
      }
         if(!gameOver) {
        restartButton->render(renderer, font);
        menuButton->render(renderer, font);
      }
      if(gameOver) {
         if(font) {
            SDL_Color messageColor = {255, 255, 255, 255};
            SDL_Surface* messageSurface = TTF_RenderText_Solid(font, winMessage.c_str(), messageColor);
            if(messageSurface) {
               SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
               if(messageTexture) {
                  SDL_Rect messageRect = {
                     SCREEN_WIDTH/2 - messageSurface->w/2,
                     SCREEN_HEIGHT/2 - messageSurface->h/2,
                     messageSurface->w,
                     messageSurface->h
                  };
                  SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
                  SDL_DestroyTexture(messageTexture);
               }
               SDL_FreeSurface(messageSurface);
            }
         } else {
            cout << winMessage << endl;
         }
      }

      SDL_RenderPresent(renderer);
   }

   void generateWalls() {
      walls.clear();
      for(int i = 3; i < MAP_HEIGHT - 3; i += 2) {
            for(int j = 3; j < MAP_WIDTH - 3; j += 2) {
               Wall w = Wall(j * TILE_SIZE, i * TILE_SIZE);
               walls.push_back(w);
            }
      }
   }

   void handleStartScreenEvents() {
      SDL_Event event;
      int mouseX, mouseY;

      while(SDL_PollEvent(&event)) {
         if(event.type == SDL_QUIT) {
            running = false;
         } else if(event.type == SDL_MOUSEMOTION) {
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            startButton->isMouseOver(mouseX, mouseY);
            quitButton->isMouseOver(mouseX, mouseY);

         } else if(event.type == SDL_MOUSEBUTTONDOWN) {
            if(event.button.button == SDL_BUTTON_LEFT) {
               mouseX = event.button.x;
               mouseY = event.button.y;

               if(startButton->isMouseOver(mouseX, mouseY)) {
                  initGame();
               } else if(quitButton->isMouseOver(mouseX, mouseY)) {
                  running = false;
               }
            }
         }
      }
   }

   void handleEvents() {
      if(currentState == STATE_START_SCREEN) {
         handleStartScreenEvents();
         return;
      }

      SDL_Event event;
       int mouseX, mouseY;
         while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            running = false;
        } else if(event.type == SDL_MOUSEMOTION) {
            mouseX = event.motion.x;
            mouseY = event.motion.y;

            restartButton->isMouseOver(mouseX, mouseY);
            menuButton->isMouseOver(mouseX, mouseY);

        } else if(event.type == SDL_MOUSEBUTTONDOWN) {
            if(event.button.button == SDL_BUTTON_LEFT) {
                mouseX = event.button.x;
                mouseY = event.button.y;

                if(restartButton->isMouseOver(mouseX, mouseY)) {
                    resetGame();
                    initGame();
                } else if(menuButton->isMouseOver(mouseX, mouseY)) {
                    resetGame();
                }
            }
            } else if(event.type == SDL_KEYDOWN) {
               switch(event.key.keysym.sym) {
                  case SDLK_SPACE:
                        if(player1->active)
                           player1->shoot();
                        break;
                  case SDLK_KP_PLUS:
                        if(player2->active)
                           player2->shoot();
                        break;
                  case SDLK_ESCAPE:
                        if(currentState == STATE_GAME_RUNNING || currentState == STATE_GAME_OVER) {
                           resetGame();
                        }
                        break;
               }
            }
      }

      keyboardState = SDL_GetKeyboardState(NULL);

      if(player1->active) {
         int dx = 0, dy = 0;

         if(keyboardState[SDL_SCANCODE_W]) dy -= 5;
         if(keyboardState[SDL_SCANCODE_S]) dy += 5;
         if(keyboardState[SDL_SCANCODE_A]) dx -= 5;
         if(keyboardState[SDL_SCANCODE_D]) dx += 5;
         if(dx != 0 || dy != 0) {
            player1->move(dx, dy, walls);
         }
      }

      if(player2->active) {
         int dx = 0, dy = 0;

         if(keyboardState[SDL_SCANCODE_UP]) dy -= 5;
         if(keyboardState[SDL_SCANCODE_DOWN]) dy += 5;
         if(keyboardState[SDL_SCANCODE_LEFT]) dx -= 5;
         if(keyboardState[SDL_SCANCODE_RIGHT]) dx += 5;
         if(dx != 0 || dy != 0) {
            player2->move(dx, dy, walls);
         }
      }
      if(player1->active && player2->active) {
            if(player1->checkCollision(*player2)) {
               if(player1->dirX != 0) {
                  player1->x -= player1->dirX * 5;
                  player1->rect.x = player1->x;
               }
               else if(player1->dirY != 0) {
                  player1->y -= player1->dirY * 5;
                  player1->rect.y = player1->y;
               }

               if(player2->dirX != 0) {
                  player2->x -= player2->dirX * 5;
                  player2->rect.x = player2->x;
               }
               else if(player2->dirY != 0) {
                  player2->y -= player2->dirY * 5;
                  player2->rect.y = player2->y;
               }
            }
      }
   }

   void resetGame() {
      if(player1) {
         delete player1;
         player1 = nullptr;
      }
      if(player2) {
         delete player2;
         player2 = nullptr;
      }

      walls.clear();
      enemies.clear();
      gameOver = false;
      currentState = STATE_START_SCREEN;
   }

   void spawnEnemies() {
      enemies.clear();
      for(int i = 0; i < enemyNumber; ++i) {
            int ex, ey;
            bool validPosition = false;
            while (!validPosition) {
               ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
               ey = (rand() % ((MAP_HEIGHT - 2) / 2) + 1) * TILE_SIZE;
               validPosition = true;
               for (const auto& wall : walls) {
                  if(wall.active && wall.x == ex && wall.y == ey) {
                        validPosition = false;
                        break;
                  }
               }
               for (const auto& enemy : enemies) {
                  if(enemy.active && abs(enemy.x - ex) < TILE_SIZE && abs(enemy.y - ey) < TILE_SIZE) {
                        validPosition = false;
                        break;
                  }
               }
               if(player2 && abs(player2->x - ex) < TILE_SIZE && abs(player2->y - ey) < TILE_SIZE) {
                  validPosition = false;
               }
            }
            enemies.push_back(EnemyTank(ex, ey));
      }
   }

   void update() {
      if(currentState == STATE_START_SCREEN) {
         return;
      }
      if(gameOver) return;
      if(player1->active) {
            player1->updateBullets();
      }

      if(player2->active) {
            player2->updateBullets();
      }

      for(auto& enemy : enemies) {
            if(enemy.active) {
               enemy.move(walls);
               enemy.updateBullets();
               if(rand() % 100 < 2) {
                  enemy.shoot();
               }
            }
      }

      for(auto& enemy : enemies) {
            for(auto& bullet : enemy.bullets) {
               for(auto& wall : walls) {
                  if(wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                        wall.active = false;
                        bullet.active = false;
                        break;
                  }
               }
            }
      }

      for(auto& bullet : player1->bullets) {
            for(auto& wall : walls) {
               if(wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                  wall.active = false;
                  bullet.active = false;
                  break;
               }
            }
      }

      for(auto& bullet : player2->bullets) {
            for(auto& wall : walls) {
               if(wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                  wall.active = false;
                  bullet.active = false;
                  break;
               }
            }
      }

      for(auto& bullet : player1->bullets) {
            for(auto& enemy : enemies) {
               if(enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                  enemy.active = false;
                  bullet.active = false;
               }
            }
      }

      for(auto& bullet : player2->bullets) {
            for(auto& enemy : enemies) {
               if(enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                  enemy.active = false;
                  bullet.active = false;
               }
            }
      }

      for(auto& bullet : player1->bullets) {
            if(player2->active && SDL_HasIntersection(&bullet.rect, &player2->rect)) {
               player2->active = false;
               bullet.active = false;
               checkGameState();
            }
      }

      for(auto& bullet : player2->bullets) {
            if(player1->active && SDL_HasIntersection(&bullet.rect, &player1->rect)) {
               player1->active = false;
               bullet.active = false;
               checkGameState();
            }
      }

      enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const EnemyTank &e) { return !e.active; }), enemies.end());

      if(enemies.empty()) {
            if(player1->active && player2->active) {
               gameOver = true;
               winMessage = "Both players win against the enemies!";
               currentState = STATE_GAME_OVER;
            }
            else {
               checkGameState();
            }
      }

      for(auto& enemy : enemies) {
            for(auto& bullet : enemy.bullets) {
               if(player1->active && SDL_HasIntersection(&bullet.rect, &player1->rect)) {
                  player1->active = false;
                  bullet.active = false;
                  checkGameState();
               }

               if(player2->active && SDL_HasIntersection(&bullet.rect, &player2->rect)) {
                  player2->active = false;
                  bullet.active = false;
                  checkGameState();
               }
            }
      }
   }

   void checkGameState() {
      bool player1Active = player1->active;
      bool player2Active = player2->active;
      if(!player1Active && !player2Active) {
            gameOver = true;
            winMessage = "Both players lost! Enemies win!";
            currentState = STATE_GAME_OVER;
      }
      else if(player1Active && !player2Active && enemies.empty()) {
            gameOver = true;
            winMessage = "Player 1 wins!";
            currentState = STATE_GAME_OVER;
      }
      else if(!player1Active && player2Active && enemies.empty()) {
            gameOver = true;
            winMessage = "Player 2 wins!";
            currentState = STATE_GAME_OVER;
      }
   }

   void renderGameOver() {
      SDL_Color textColor = {255, 255, 255, 255};
       restartButton->render(renderer, font);
       menuButton->render(renderer, font);
      if(font) {
         SDL_Surface* messageSurface = TTF_RenderText_Solid(font, winMessage.c_str(), textColor);
         if(messageSurface) {
            SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
            if(messageTexture) {
               SDL_Rect messageRect = {
                  SCREEN_WIDTH/2 - messageSurface->w/2,
                  SCREEN_HEIGHT/2 - messageSurface->h/2,
                  messageSurface->w,
                  messageSurface->h
               };
               SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
               SDL_DestroyTexture(messageTexture);
            }
            SDL_FreeSurface(messageSurface);
         }
      }

      if(font) {
         string returnText = "Press ESC to return to main menu";
         SDL_Surface* returnSurface = TTF_RenderText_Solid(font, returnText.c_str(), textColor);
         if(returnSurface) {
            SDL_Texture* returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
            if(returnTexture) {
               SDL_Rect returnRect = {
                  SCREEN_WIDTH/2 - returnSurface->w/2,
                  SCREEN_HEIGHT/2 + 50,
                  returnSurface->w,
                  returnSurface->h
               };
               SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
               SDL_DestroyTexture(returnTexture);
            }
            SDL_FreeSurface(returnSurface);
         }
      }
   }

   void run() {
      while(running) {
            handleEvents();
            update();
            if(currentState == STATE_START_SCREEN) {
               renderStartScreen();
            } else if(currentState == STATE_GAME_RUNNING) {
               render();
            } else if(currentState == STATE_GAME_OVER) {
               render();
               renderGameOver();

               SDL_RenderPresent(renderer);
            }

            SDL_Delay(16);
      }
   }

   ~Game() {
      if(backgroundMusic) {
         Mix_FreeMusic(backgroundMusic);
         backgroundMusic = nullptr;
      }


      delete startButton;
      delete quitButton;
      delete restartButton;
      delete menuButton;

      if(font) TTF_CloseFont(font);
      if(titleFont) TTF_CloseFont(titleFont);

      if(player1) delete player1;
      if(player2) delete player2;
      if(player1TankTexture) SDL_DestroyTexture(player1TankTexture);
      if(player2TankTexture) SDL_DestroyTexture(player2TankTexture);

      TTF_Quit();
      Mix_Quit();
      IMG_Quit();
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
   }
};

int main(int argc, char* argv[]) {
   srand(time(NULL));
   Game game;
   if(game.running) {
      game.run();
   }
   return 0;
}
