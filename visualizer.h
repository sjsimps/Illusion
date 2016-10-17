class Visualizer
{
public:
    Visualizer();
    ~Visualizer();

    void initialize(int width, int height);
    void set_texture(char* img_path);
    bool render();

    SDL_Window* m_win;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_img;
    SDL_Rect m_texr;
    int m_height, m_width;
};
