
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "small_fft.h"
#include "pulseaudio_recorder.h"
#include "visualizer.h"

std::vector<struct FreqContent> content;

void* run_visualizer(void* thread_id)
{
    const int WIDTH = 800;
    const int HEIGHT = 600;
    char* IMG_PATH = "test1.bmp";

    Visualizer visualizer;
    visualizer.initialize(WIDTH, HEIGHT, IMG_PATH);

    uint32_t* pixels = new uint32_t[WIDTH*HEIGHT];
    visualizer.get_pixels(pixels, WIDTH, HEIGHT);
    visualizer.render();

    std::cout << "VISUALIZER\n";

    while(1)
    {
        if (content.size() > 0)
        {
            float frq_cmp = content[0].frq;
            float ampl = content[0].pwr;
            //std::cout << "FRQ :: " << frq_cmp << "\n";
            int frq_idx_n = 1;
            for (int frq_idx = 100; frq_idx < 15000; frq_idx *= 2)
            {
                if (frq_cmp < frq_idx)
                {
                    int pix_count = 0;
                    for (int x = 0; x <  WIDTH * HEIGHT; x++)
                    {
                        pixels[pix_count] += (x%(3*frq_idx_n)) - (x%(frq_idx_n)) + x;
                        pix_count = (pix_count + 1) % (WIDTH * HEIGHT);
                    }
                    visualizer.set_pixels(pixels, WIDTH, HEIGHT);
                    visualizer.render();
                    usleep(10000);
                    frq_idx = 15000;
                }
                else
                {
                    frq_idx_n += 1;
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char*argv[])
{
    const int REC_BUF_SIZE = 1024;
    const int FFT_BUF_SIZE = 65536;
    PulseAudioRecorder recorder(REC_BUF_SIZE);
    SmallFFT fft(FFT_BUF_SIZE, 1.0/FFT_BUF_SIZE);

    pthread_t vis_thread;
    pthread_create(&vis_thread, NULL, run_visualizer, (void *)1);

    while (1)
    {
        int index = 0;
        while (recorder.read_to_buf() >= 0
               && (index+REC_BUF_SIZE) < FFT_BUF_SIZE)
        {
            //recorder.print_buf();
            int buf_idx = 0;
            for (int x = index; x < (index + REC_BUF_SIZE); x++)
            {
                fft.m_data[x<<1] = (float)(recorder.m_buf[buf_idx])/1024.0;
                buf_idx++;
            }
            index += REC_BUF_SIZE;
        }

        clock_t t = clock(); 
        content = fft.get_significant_frq(400.0);
        t = clock() - t;
        std::cout << "EXEC_TIME : " << ((float)t)/CLOCKS_PER_SEC << "\n";
        for (unsigned int x = 0; x < content.size(); x++)
        {
            std::cout << "FRQ : " << content[x].frq <<
                      " // AMPL: " << content[x].pwr << "\n";
        }
        fft.reset();
        std::cout << " ------------- \n";


    }

}
