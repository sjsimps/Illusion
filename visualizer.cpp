#include <stdio.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "visualizer.h"

#define WIDTH 800
#define HEIGHT 650
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
    // put the location where we want the texture to be drawn into a rectangle
    // I'm also scaling the texture 2x simply by setting the width and height
    m_texr.x = 0;
    m_texr.y = 0;
    m_texr.w = width;
    m_texr.h = height;

    // create the window and renderer
    // note that the renderer is accelerated
    m_win = SDL_CreateWindow("Image Loading", 100, 100, width, height, 0);
    m_renderer = SDL_CreateRenderer(m_win, -1, SDL_RENDERER_ACCELERATED);

    // load our image
    SDL_Surface* img_tmp = IMG_Load(IMG_PATH);
    //img_tmp = SDL_ConvertSurfaceFormat(img_tmp, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING);

    SDL_Surface* s = SDL_CreateRGBSurface(0x00, width, height,
                                          32 /*bits*/, 0xff /*r*/, 0xff00/*g*/, 0xff0000 /*b*/, 0xff000000);
                                          //32 /*bits*/, 0xff /*r*/, 0xff00/*g*/, 0xff0000 /*b*/, 0xff000000);

    SDL_BlitSurface(img_tmp, &m_texr, s, NULL);

    //m_img = SDL_CreateTextureFromSurface(m_renderer,s);

    m_img = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height );

    set_pixels((uint32_t*)s->pixels, width, height);

    SDL_QueryTexture(m_img, NULL, NULL, &m_width, &m_height); // get the width and height of the texture
    SDL_FreeSurface(img_tmp);
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

void Visualizer::get_pixels(uint32_t* pixel_buffer, int width, int height)
{
    uint32_t* pixels = NULL;
    int pitch = 0;

    // Now let's make our "pixels" pointer point to the texture data.
    if (SDL_LockTexture(m_img, NULL, (void**)&pixels, &pitch))
    {
        std::cout << "Get Pixels : SDL Lock Failed : " << SDL_GetError() << "\n";
    }
    else
    {
        size_t buffer_size = width*height;
        memcpy(pixel_buffer, pixels, buffer_size * 4);
    }
    SDL_UnlockTexture(m_img);
}

void Visualizer::set_pixels(uint32_t* pixel_buffer, int width, int height)
{
    uint32_t* pixels = NULL;
    int pitch = 0;

    // Now let's make our "pixels" pointer point to the texture data.
    if (SDL_LockTexture(m_img, NULL, (void**)&pixels, &pitch))
    {
        //LOCK FAILURE
        std::cout << "Set Pixels : SDL Lock Failed : " << SDL_GetError() << "\n";
    }
    else
    {
        size_t buffer_size = width*height;
        memcpy(pixels, pixel_buffer, buffer_size * 4);
    }
    SDL_UnlockTexture(m_img);
}


int main (int argc, char *argv[])
{
    Visualizer visualizer;
    visualizer.initialize(WIDTH, HEIGHT);

    uint32_t* pixels = new uint32_t[WIDTH*HEIGHT];
    visualizer.get_pixels(pixels, WIDTH, HEIGHT);

    int count = 0;
    while (visualizer.render())
    {
        for (int x = 0; x <  WIDTH * HEIGHT; x++)
        {
            pixels[count] += (x%7) - (x%3) + x;
            count = (count + 1) % (WIDTH * HEIGHT);
        }
        std::cout << count << "\n";
        visualizer.set_pixels(pixels, WIDTH, HEIGHT);
    }
    return 0;
}
