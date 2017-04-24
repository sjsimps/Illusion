
#include "../include/image_manipulator.h"

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
        m_overlay = new uint32_t[num_pixels];
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
    for (int x = 0; x < (m_width * m_height); x++)
    {
        if (std::rand() < compare)
        {
            m_overlay[x] = 0;
        }
    }
}

void ImageManipulator::clear_overlay()
{
    for (int x = 0; x < (m_width * m_height); x++)
    {
        m_overlay[x] = 0;
    }
}

void ImageManipulator::transform_overlay(int frq, int amp,
                                         int content_idx, int n_frqs)
{
    for (int x = content_idx; x < (m_width * m_height); x+=n_frqs)
    {
        int newpix = m_overlay[x];

        switch (frq)
        {
            case 0:
            case 1:
                newpix = ((newpix+(amp<<8)) & 0xff00) | (newpix & 0xffff00ff);
                break;
            case 2:
            case 3:
                newpix = ((newpix+(amp>>1)) & 0xff) | (newpix & 0xffffff00);
                newpix = ((newpix+(amp<<7)) & 0xff00) | (newpix & 0xffff00ff);
                break;
            case 4:
            case 5:
                newpix = ((newpix+amp) & 0xff) | (newpix & 0xffffff00);
                break;
            case 6:
            case 7:
                newpix = ((newpix+(amp<<7)) & 0xff00) | (newpix & 0xffff00ff);
                newpix = ((newpix+(amp<<15)) & 0xff0000) | (newpix & 0xff00ffff);
                break;
            case 8:
            case 9:
            case 10:
                newpix = ((newpix+(amp<<16)) & 0xff0000) | (newpix & 0xff00ffff);
                break;
            default:
                newpix = ((newpix+(amp<<15)) & 0xff0000) | (newpix & 0xff00ffff);
                newpix = ((newpix+(amp>>1)) & 0xff) | (newpix & 0xffffff00);
                break;
        }
        m_overlay[x] = newpix;
    }
}

void ImageManipulator::update_image()
{
    for (int x = 0; x < (m_width * m_height); x++)
    {
        m_image[x] = m_overlay[x] + m_pixels[x];
    }
    m_visualizer->set_pixels(m_image, m_width, m_height);
    m_visualizer->render();
}

