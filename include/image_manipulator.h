
#ifndef IMAGE_MANIPULATOR_H

#include "visualizer.h"

class ImageManipulator
{
public:
    ImageManipulator(Visualizer* visualizer, uint32_t* pixels,
                     int width, int height);
    ~ImageManipulator();

    void set_image(uint32_t* pixels, int width, int height);

    void transform_overlay(int frq, int amp_f,
                           int content_idx=0, int n_frqs=1);

    void fade_overlay(float rate = 0.25);
    void clear_overlay();

    void update_image();

    Visualizer* m_visualizer;
    int m_width, m_height;
    uint32_t* m_pixels;
    uint32_t* m_overlay;
    uint32_t* m_image;
};

#endif // IMAGE_MANIPULATOR_H
#define IMAGE_MANIPULATOR_H
