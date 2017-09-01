
#include "../include/image_manipulator.h"
#include <iostream>

ImageManipulator::ImageManipulator(Visualizer* visualizer,
                                   uint32_t* pixels,
                                   int width, int height)
{
    m_visualizer = visualizer;
    set_image(pixels, width, height);
}

ImageManipulator::~ImageManipulator()
{
    if (m_pixels != NULL) delete m_pixels;
    if (m_overlay != NULL) delete m_overlay;
    if (m_image != NULL) delete m_image;
}

void ImageManipulator::set_image(uint32_t* pixels,
                            int width, int height)
{
    int num_pixels = width * height;
    if (m_height != height ||
        m_width != width)
    {
        m_height = height;
        m_width = width;

        if (m_pixels != NULL) delete m_pixels;
        if (m_overlay != NULL) delete m_overlay;
        if (m_image != NULL) delete m_image;
        m_pixels = new uint32_t[num_pixels];
        m_overlay = new uint8_t[num_pixels*3];
        m_image = new uint32_t[num_pixels];
    }

    if (pixels != NULL)
    {
        memcpy(m_pixels, pixels, sizeof(uint32_t)*num_pixels);
        memcpy(m_image, pixels, sizeof(uint32_t)*num_pixels);
    }
}

void ImageManipulator::fade_overlay(float rate)
{
    int compare = (RAND_MAX*rate);
    for (int x = 0; x < (m_width * m_height * 3);)
    {
        if (std::rand() < compare)
        {
            m_overlay[x++] = 0;
            m_overlay[x++] = 0;
            m_overlay[x++] = 0;
        }
        else
        {
            x += 3;
        }
    }
}

void ImageManipulator::clear_overlay()
{
    for (int x = 0; x < (m_width * m_height * 3);)
    {
        m_overlay[x++] = 0;
        m_overlay[x++] = 0;
        m_overlay[x++] = 0;
    }
}

static int DECAY = 0;
void ImageManipulator::transform_overlay(int frq, int amp,
                                         int content_idx, int n_frqs)
{
    int over = content_idx*3;
    for (int x = content_idx; x < (m_width * m_height); x+=n_frqs)
    {
        uint8_t r = m_overlay[over] >> DECAY;
        uint8_t g = m_overlay[over+1] >> DECAY;
        uint8_t b = m_overlay[over+2] >> DECAY;
        amp &= 0xff;

        switch (frq)
        {
            case 0:
            case 1:
                g += amp;
                //g |= -(g < amp);
                m_overlay[over+1] = g;
                break;
            case 2:
            case 3:
                g += amp;
                //g |= -(g < amp);
                m_overlay[over+1] = g;
                b = b+amp;
                //b |= -(b < amp);
                m_overlay[over+2] = b;
                break;
            case 4:
            case 5:
                b = b+amp;
                //b |= -(b < amp);
                m_overlay[over+2] = b;
                break;
            case 6:
            case 7:
                b = b+amp;
                //b |= -(b < amp);
                m_overlay[over+2] = b;
                r = r+amp;
                //r |= -(r < amp);
                m_overlay[over] = r;
                break;
            case 8:
            case 9:
            case 10:
                r = r+amp;
                //r |= -(r < amp);
                m_overlay[over] = r;
                break;
            default:
                r = r+amp;
                //r |= -(r < amp);
                m_overlay[over] = r;
                g = g+amp;
                //g |= -(g < amp);
                m_overlay[over+1] = g;
                break;
        }
        m_image[x] = ((r << 16) | (g << 8) | b) + m_pixels[x];
        over += n_frqs*3;
    }
}

void ImageManipulator::update_image()
{
    m_visualizer->set_pixels(m_image, m_width, m_height);
    m_visualizer->render();
}

