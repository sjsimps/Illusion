
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "small_fft.h"
#include "pulseaudio_recorder.h"
#include "visualizer.h"
#include "beat_detector.h"

std::vector<struct FreqContent> content;
const int WIDTH = 800;
const int HEIGHT = 600;
const int N_FRQS = 3;
const float FRQ_THRESHOLD = 150.0;
char* IMG_PATH = "test1.bmp";

void transform_pixmap(uint32_t* pixels, float frq, float amp,
                      int content_idx, int n_frqs)
{
    // TODO :: NEEDS MUTEX
    //std::cout << "FRQ :: " << frq_cmp << "\n";
    int frq_idx_n = 0;
    for (int frq_idx = 3*FRQ_THRESHOLD; frq_idx < 5000; frq_idx *= 3)
    {
        if (frq < frq_idx)
        {
            for (int x = content_idx; x < WIDTH * HEIGHT; x+=n_frqs)
            {
                pixels[x] += ((int)(amp/400) << (frq_idx_n*8)) & (0xFF << (frq_idx_n*8));
            }
            frq_idx = 15000;
        }
        else
        {
            frq_idx_n++;
        }
    }
}

void* run_visualizer(void* thread_id)
{

    Visualizer visualizer;
    visualizer.initialize(WIDTH, HEIGHT, IMG_PATH);

    uint32_t* pixels = new uint32_t[WIDTH*HEIGHT];
    visualizer.get_pixels(pixels, WIDTH, HEIGHT);
    visualizer.render();

    std::cout << "VISUALIZER\n";

    while(1)
    {
        int content_size = content.size();
        if (content_size > 0)
        {
            float frqs[N_FRQS];
            float amps[N_FRQS];
            for (int z = 0; (z < N_FRQS  && z < content_size); z++)
            {
                frqs[z] = content[z].frq;
                amps[z] = content[z].pwr;
            }

            for (int content_idx = 0; (content_idx < N_FRQS  && content_idx < content_size); content_idx++)
            {
                //TODO
                transform_pixmap(pixels, frqs[content_idx], amps[content_idx],
                                 content_idx, N_FRQS);
            }
            visualizer.set_pixels(pixels, WIDTH, HEIGHT);
            visualizer.render();
        }
        usleep(50000);
    }
    delete pixels;
    return NULL;
}

void sigint_handle(int p)
{
    exit(1);
}

int main(int argc, char*argv[])
{
    signal(SIGINT, sigint_handle);

    const int REC_BUF_SIZE = 4096<<1;
    const int FFT_BUF_SIZE = 32768 >> 2;
    const int SAMPLE_RATE = 44100; //Samples per sec
    PulseAudioRecorder recorder(REC_BUF_SIZE);
    SmallFFT fft(FFT_BUF_SIZE, 1.0/SAMPLE_RATE);

    pthread_t vis_thread;
    pthread_create(&vis_thread, NULL, run_visualizer, (void *)1);

    BeatDetector beat_det(1.0, REC_BUF_SIZE, 0.25);

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
                float datapoint = (float)(recorder.m_buf[buf_idx]) / 1024;
                data[x] = datapoint; //>> 10); // /1024
                beat_det.m_data[x>>1] = datapoint;
                buf_idx++;
            }
            //std::cout << "HAS BEAT : " << beat_det.contains_beat() << " " << beat_det.m_threshold << "\n";

            // EXECUTING FFT
            clock_t t = clock();
            memcpy(fft.m_data, data, FFT_BUF_SIZE*2*sizeof(float));
            content = fft.get_significant_frq(500.0, FRQ_THRESHOLD);
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
