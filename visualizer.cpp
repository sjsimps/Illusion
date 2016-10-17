#include <stdio.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "visualizer.h"

#define WIDTH 800
#define HEIGHT 600
#define IMG_PATH "out.bmp"

Visualizer::Visualizer()
{
    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL Init Failed\n";
    }

    // variable declarations
    m_win = NULL;
    m_renderer = NULL;
    m_img = NULL;
}
 
Visualizer::~Visualizer()
{
    SDL_DestroyTexture(m_img);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_win);
}

void Visualizer::initialize(int width, int height)
{
    // create the window and renderer
    // note that the renderer is accelerated
    m_win = SDL_CreateWindow("Image Loading", 100, 100, WIDTH, HEIGHT, 0);
    m_renderer = SDL_CreateRenderer(m_win, -1, SDL_RENDERER_ACCELERATED);
    
    // load our image
    m_img = IMG_LoadTexture(m_renderer, IMG_PATH);
    SDL_QueryTexture(m_img, NULL, NULL, &m_width, &m_height); // get the width and height of the texture
    // put the location where we want the texture to be drawn into a rectangle
    // I'm also scaling the texture 2x simply by setting the width and height
    m_texr.x = width/2;
    m_texr.y = height/2;
    m_texr.w = m_width*2;
    m_texr.h = m_height*2; 
}

bool Visualizer::render()
{
    // event handling
    SDL_Event e;
    if ( SDL_PollEvent(&e) )
    {
        if ( (e.type == SDL_QUIT) ||
             (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE) )
        {
            return false;
        }
    } 
    // clear the screen
    SDL_RenderClear(m_renderer);
    // copy the texture to the rendering context
    SDL_RenderCopy(m_renderer, m_img, NULL, &m_texr);
    // flip the backbuffer
    // this means that everything that we prepared behind the screens is actually shown
    SDL_RenderPresent(m_renderer);

    return true;
}

int main (int argc, char *argv[])
{
    Visualizer visualizer;
    visualizer.initialize(WIDTH, HEIGHT);

    while (visualizer.render()){}
    return 0;
}
