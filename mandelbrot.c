//Using SDL and standard IO
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const double SCREEN_RATIO = (double)SCREEN_HEIGHT / SCREEN_WIDTH;
const int FPS = 30;
const int CLOCK = 1000 / FPS;
const int INITIAL_RESOLUTION = 1 << 4;

Uint32 pixels[SCREEN_WIDTH*SCREEN_HEIGHT*4];
SDL_Window* gWindow = NULL;
SDL_Surface* gScreen = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;
int gResolution = INITIAL_RESOLUTION;
double gScale = 2;
double gOffsetX = 0;
double gOffsetY = 0;
double gLastScale = 0;

double scaleX(int x) {
  return (((double)x / SCREEN_WIDTH) - 0.5) * gScale + gOffsetX;
}

double scaleY(int y) {
  return -(((double)y / SCREEN_HEIGHT * SCREEN_RATIO) - SCREEN_RATIO / 2) * gScale + gOffsetY;
}


void mandelbrot3(int resolution) {
  int offset = resolution == 1 ? 0 : resolution >> 1;

  for (int screenX = 0; screenX < SCREEN_WIDTH; screenX += resolution) {
    for (int screenY = 0; screenY < SCREEN_HEIGHT; screenY += resolution) {
      double x0 = scaleX(screenX - offset);
      double y0 = scaleY(screenY - offset);
      double x = 0;
      double y = 0;
      int iteration = 0;
      while ((x*x + y*y <= 4) && iteration <= 0xff) {
        double xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iteration++;
      }
      if (resolution == 1) {
        pixels[screenY * SCREEN_WIDTH + screenX] = iteration << 8;
      }
      for(int rx = 0; rx < resolution; rx++) {
        for(int ry = 0; ry < resolution; ry++) {
          pixels[(screenY + ry) * SCREEN_WIDTH + screenX + rx] = iteration << 8;
        }
      }
    }
  }
}

void mandelbrot() {
  for (int screenX = 0; screenX < SCREEN_WIDTH; screenX++) {
    for (int screenY = 0; screenY < SCREEN_HEIGHT; screenY++) {
      double x0 = scaleX(screenX);
      double y0 = scaleY(screenY);
      double x = 0;
      double y = 0;
      int iteration = 0;
      while ((x*x + y*y <= 4) && iteration <= 0xff) {
        double xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iteration++;
      }
      pixels[screenY * SCREEN_WIDTH + screenX] = iteration << 8;
    }
  }
}

void mandelbrot2() {
  for (int screenX = 0; screenX < SCREEN_WIDTH; screenX++) {
    for (int screenY = 0; screenY < SCREEN_HEIGHT; screenY++) {
      double x0 = scaleX(screenX);
      double y0 = scaleY(screenY);
      double x = 0;
      double y = 0;
      int iteration = 0;
      int maxIteration = 0xff;
      while ((x*x + y*y <= (1 << 16)) && iteration < maxIteration) {
        double xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iteration++;
      }
      if (iteration < maxIteration) {
        double logZn = log(x*x + y*y) / 2;
        double nu = log(logZn/log(2))/log(2);
        iteration = iteration + 1 - nu;
      }
      //pixels[screenY * SCREEN_WIDTH + screenX] = (int)floor(iteration) << 16;
      pixels[screenY * SCREEN_WIDTH + screenX] = ((int)iteration & 0x7) | ((int)iteration & 0x38) << 8 | ((int)iteration & 0x60) << 16;
    }
  }
}

int render() {
  if(gLastScale != gScale || gResolution != 0) {
    mandelbrot3(gResolution);
    SDL_UpdateTexture(gTexture, NULL, pixels, 640 * sizeof(Uint32));
    SDL_RenderClear(gRenderer);
    SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
    SDL_RenderPresent(gRenderer);
    gLastScale = gScale;
    gResolution >>= 1;
  }
}

void event(SDL_Event* e) {
  switch(e->type) {
    case SDL_MOUSEBUTTONUP:
      gOffsetX = scaleX(e->motion.x);
      gOffsetY = scaleY(e->motion.y);
      if (e->button.button == SDL_BUTTON_LEFT) {
        gScale *= 0.5;
        gResolution = INITIAL_RESOLUTION;
      } else if (e->button.button == SDL_BUTTON_RIGHT) {
        gScale *= 1.5;
        gResolution = INITIAL_RESOLUTION;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      break;
    case SDL_MOUSEMOTION:
      char title[80];
      sprintf(title, "offset x: %f, offset y: %f", scaleX(e->motion.x), scaleY(e->motion.y));
      SDL_SetWindowTitle(gWindow, title);
      break;
  }
}

int loop() {
  bool quit = false;
  SDL_Event e;
  unsigned int timer;
  int delay;

  while(!quit) {
    timer = SDL_GetTicks();

    while(SDL_PollEvent(&e) != 0) {
      if(e.type == SDL_QUIT) {
        quit = true;
      } else {
        event(&e);
      }
    }

    render();
    delay = CLOCK - (SDL_GetTicks() - timer);
    SDL_Delay(delay > 0 ? delay : 0);
  }
}

void boot() {
  memset(pixels, 0, 640 * 480 * sizeof(Uint32));
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  } else {
    gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 640, 480);

    if(gWindow == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    } else {
      gScreen = SDL_GetWindowSurface(gWindow);
      loop();
    }
  }
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
}

int main( int argc, char* args[] ) {
  boot();
  return 0;
}
