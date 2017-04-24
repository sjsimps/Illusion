
#ifndef VISUALIZER_H

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
class Visualizer
{
public:
    Visualizer();
    ~Visualizer();

    void initialize(int width, int height, std::string img_path, bool fullscreen);
    void set_texture(std::string img_path);
    bool render();
    void get_pixels(uint32_t* pixel_buffer, int width, int height);
    void set_pixels(uint32_t* pixel_buffer, int width, int height);
    void get_image_pixels(int width, int height, std::string image_path, uint32_t* buffer);

    SDL_Window* m_win;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_img;
    SDL_Rect m_texr;
    int m_height, m_width;
};

#endif //VISUALIZER_H
#define VISUALIZER_H
