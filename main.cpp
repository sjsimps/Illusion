
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
    const int N_FRQS = 1;
    char* IMG_PATH = "test1.bmp";

    Visualizer visualizer;
    visualizer.initialize(WIDTH, HEIGHT, IMG_PATH);

    uint32_t* pixels = new uint32_t[WIDTH*HEIGHT];
    visualizer.get_pixels(pixels, WIDTH, HEIGHT);
    visualizer.render();

    std::cout << "VISUALIZER\n";

    while(1)
    {
        if (content.size() > N_FRQS)
        {
            float frqs[N_FRQS];
            float amps[N_FRQS];
            for (int z = 0; z < N_FRQS; z++)
            {
                frqs[z] = content[z].frq;
                amps[z] = content[z].pwr;
            }

            for (int content_idx = 0; content_idx < N_FRQS; content_idx++)
            {
                // TODO :: NEEDS MUTEX
                //std::cout << "FRQ :: " << frq_cmp << "\n";
                int frq_idx_n = 0;
                for (int frq_idx = 500; frq_idx < 5000; frq_idx *= 3)
                {
                    if (frqs[content_idx] < frq_idx)
                    {
                        for (int x = content_idx; x < WIDTH * HEIGHT; x+=3)
                        {
                            pixels[x] += ((int)(amps[content_idx]/400 ) << (frq_idx_n*8)) & (0xFF << (frq_idx_n*8));
                        }
                        visualizer.set_pixels(pixels, WIDTH, HEIGHT);
                        visualizer.render();
                        frq_idx = 15000;
                    }
                    else
                    {
                        frq_idx_n++;
                    }
                }
            }
        }
        usleep(50000);
    }
    delete pixels;
    return NULL;
}

int main(int argc, char*argv[])
{
    const int REC_BUF_SIZE = 4096<<2;
    const int FFT_BUF_SIZE = 65536;
    PulseAudioRecorder recorder(REC_BUF_SIZE);
    SmallFFT fft(FFT_BUF_SIZE, 1.0/FFT_BUF_SIZE);

    pthread_t vis_thread;
    pthread_create(&vis_thread, NULL, run_visualizer, (void *)1);

    float* data = new float[FFT_BUF_SIZE*2];

    while (1)
    {
        if (recorder.read_to_buf() >= 0)
        {
            //recorder.print_buf();

            // FORMATTING DATA : APPENDING CHUNK
            int buf_idx = 0;
            memcpy(data, &data[REC_BUF_SIZE*2], (FFT_BUF_SIZE - REC_BUF_SIZE)*2*sizeof(float));
            for (int x = (FFT_BUF_SIZE - REC_BUF_SIZE)*2; x < FFT_BUF_SIZE*2; x+=2)
            {
                data[x] = (float)(recorder.m_buf[buf_idx])/1024.0;
                buf_idx++;
            }

            // EXECUTING FFT
            clock_t t = clock();
            memcpy(fft.m_data, data, FFT_BUF_SIZE*2*sizeof(float));
            content = fft.get_significant_frq(500.0);
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

    delete data;
}
