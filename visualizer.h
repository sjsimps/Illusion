
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
class Visualizer
{
public:
    Visualizer();
    ~Visualizer();

    void initialize(int width, int height, char* img_path, bool fullscreen);
    void set_texture(char* img_path);
    bool render();
    void get_pixels(uint32_t* pixel_buffer, int width, int height);
    void set_pixels(uint32_t* pixel_buffer, int width, int height);

    SDL_Window* m_win;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_img;
    SDL_Rect m_texr;
    int m_height, m_width;
};
