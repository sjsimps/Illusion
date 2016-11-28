
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <cstdlib>
#include <dirent.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

#include "small_fft.h"
#include "pulseaudio_recorder.h"
#include "visualizer.h"
#include "beat_detector.h"

const int WIDTH = 800;
const int HEIGHT = 600;
const int N_FRQS = 3;
const float IMG_CHANGE_TIME = 5.0;
const float FRQ_THRESHOLD = 150.0;
const float AMPL_THRESHOLD = 250.0;
const bool USE_FULLSCREEN = false;
//const bool USE_FULLSCREEN = true;

static std::vector<std::string> image_files;
static std::vector<struct FreqContent> content;
static bool on_beat = false;

/* Returns a list of files in a directory (except the ones that begin with a dot) */

void GetImages(std::vector<std::string>* out)
{
    DIR *dir;
    class dirent *ent;
    class stat st;

    dir = opendir("./Images");
    while ((ent = readdir(dir)) != NULL) {
        const std::string file_name = ent->d_name;
        const std::string full_file_name = "./Images/" + file_name;

        if (file_name[0] == '.')
            continue;

        if (stat(full_file_name.c_str(), &st) == -1)
            continue;

        const bool is_directory = (st.st_mode & S_IFDIR) != 0;

        if (is_directory)
            continue;

        out->push_back(full_file_name);
    }
    closedir(dir);
}

void transform_pixmap(uint32_t* pixels, float frq, float amp_f,
                      int content_idx, int n_frqs, uint32_t* original_pixels)
{
    int frq_idx = (int)(frq / FRQ_THRESHOLD);
    int amp = ((int)(amp_f/ AMPL_THRESHOLD) & 0xff) * (1 + 2*on_beat);

    for (int x = content_idx; x < WIDTH * HEIGHT; x+=n_frqs)
    {
        int newpix = pixels[x];
        if (original_pixels && std::rand() < (RAND_MAX>>2))
        {
            newpix = original_pixels[x];
        }
        switch (frq_idx)
        {
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
        pixels[x] = newpix;
    }
}

void* run_visualizer(void* thread_id)
{

    Visualizer visualizer;
    visualizer.initialize(WIDTH, HEIGHT, image_files[0], USE_FULLSCREEN);

    uint32_t* original_pixels = new uint32_t[WIDTH*HEIGHT];
    uint32_t* pixels = new uint32_t[WIDTH*HEIGHT];
    visualizer.get_pixels(pixels, WIDTH, HEIGHT);
    visualizer.get_image_pixels(WIDTH, HEIGHT,image_files[0], original_pixels);
    visualizer.render();

    std::cout << "VISUALIZER\n";

    bool changing_image = false;
    clock_t last_image_change = clock();

    while(1)
    {
        int content_size = content.size();
        if (content_size > 0)
        {
            if (changing_image)
            {
                memcpy(pixels,original_pixels,sizeof(uint32_t)*WIDTH*HEIGHT);
                changing_image = false;
            }
            float frqs[N_FRQS];
            float amps[N_FRQS];
            for (int z = 0; (z < N_FRQS  && z < content_size); z++)
            {
                frqs[z] = content[z].frq;
                amps[z] = content[z].pwr;
            }

            for (int content_idx = 0; (content_idx < N_FRQS  && content_idx < content_size); content_idx++)
            {
                transform_pixmap(pixels, frqs[content_idx], amps[content_idx],
                                 content_idx, N_FRQS, NULL);
            }
            visualizer.set_pixels(pixels, WIDTH, HEIGHT);
            visualizer.render();
        }
        else
        {
            if ((float)(clock()-last_image_change)/CLOCKS_PER_SEC >= IMG_CHANGE_TIME)
            {
                int image_num = std::rand() % image_files.size();
                last_image_change = clock();
                visualizer.get_image_pixels(WIDTH, HEIGHT,image_files[image_num], original_pixels);
            }
            transform_pixmap(pixels, 0, 0,
                             0, 1, original_pixels);
            visualizer.set_pixels(pixels, WIDTH, HEIGHT);
            visualizer.render();
            changing_image = true;
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

    GetImages(&image_files);

    const int REC_BUF_SIZE = 4096 >> 3;//<<1;
    const int FFT_BUF_SIZE = 32768 >> 2;
    const int SAMPLE_RATE = 44100; //Samples per sec
    PulseAudioRecorder recorder(REC_BUF_SIZE);
    SmallFFT fft(FFT_BUF_SIZE, 1.0/SAMPLE_RATE);

    pthread_t vis_thread;
    pthread_create(&vis_thread, NULL, run_visualizer, (void *)1);

    BeatDetector beat_det(500.0, REC_BUF_SIZE, 0.05);

    float* data = new float[FFT_BUF_SIZE*2];

    while (1)
    {
        // READING AUDIO BUFFER
        if (recorder.read_to_buf() >= 0)
        {
            // GET NORMALIZATION
            float normalization_factor = 1.0 / (recorder.normalize_buffer() * 2000.0 + 1.0);
            std::cout << "NORMALIZE: " << normalization_factor << "\n";

            // FORMATTING DATA AND APPENDING CHUNK TO FFT BUFFER
            int buf_idx = 0;
            memcpy(data, &data[REC_BUF_SIZE*2-1], (FFT_BUF_SIZE - REC_BUF_SIZE)*2*sizeof(float));
            for (int x = (FFT_BUF_SIZE - REC_BUF_SIZE)*2; x < FFT_BUF_SIZE*2; x+=2)
            {
                float datapoint = (float)(recorder.m_buf[buf_idx]);
                data[x] = datapoint * normalization_factor;
                beat_det.m_data[buf_idx] = datapoint;
                buf_idx++;
            }
            on_beat = beat_det.contains_beat();
            std::cout << "HAS BEAT : " << on_beat << " " << beat_det.m_threshold << "\n";

            // EXECUTING FFT
            clock_t t = clock();
            memcpy(fft.m_data, data, FFT_BUF_SIZE*2*sizeof(float));
            content = fft.get_significant_frq(AMPL_THRESHOLD, FRQ_THRESHOLD, 2);
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
