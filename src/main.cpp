
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
#include <getopt.h>
#include <math.h>

#include "../include/small_fft.h"
#include "../include/pulseaudio_recorder.h"
#include "../include/visualizer.h"
#include "../include/beat_detector.h"
#include "../include/image_manipulator.h"

const int WIDTH = 800;
const int HEIGHT = 600;
const int N_FRQS = 3;
const float IMG_CHANGE_TIME = 5.0;
const float FRQ_FACTOR = 25.0;
const float AMPL_THRESHOLD = 250.0;
const float AMPL_FACTOR = 50.0;
const float NORMALIZATION_COEF = 1000.0;

static std::vector<std::string> image_files;
static std::vector<struct FreqContent> content;
static int on_beat = 0;

static pthread_mutex_t content_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t beat_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool option_fullscreen = false;

static double total_pwr;

static void set_options(int argc, char* argv[])
{
    int option_index;
    static struct option options[] =
    {
        {"help",       no_argument,       0, 'h'},
        {"fullscreen", no_argument,       0, 'f'},
        {0, 0, 0, 0}
    };

    while ((option_index = getopt_long(argc, argv, "hf", options, NULL)) != -1)
    {
        switch (option_index){
            case 'f':
                option_fullscreen = true;
                break;
            case 'h':
                std::cout <<"\nUsage: ./illusion [options]"
                     <<"\n\t[-f | --fullscreen] : Show output in fullscreen mode"
                     <<"\n\t[-h | --help ]      : Display help\n\n";
                exit(EXIT_SUCCESS);
                break;
            default:
                std::cout<< "\nInvalid option. Program exiting.\n";
                exit(EXIT_FAILURE);
                break;
        }
    }
}


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

int get_adjusted_frq(float frq)
{
    return (int)(frq / FRQ_FACTOR);
}

int get_adjusted_amp(float amp_f)
{
    pthread_mutex_lock(&beat_mutex);
    int amp = ((int)pow((1.0 + amp_f/AMPL_FACTOR) *
                        (1.0 + total_pwr) *
                        (1.0 + on_beat), 0.5) & 0xff);
    if (on_beat > 0)
    {
        amp *= on_beat;
        on_beat--;
    }
    pthread_mutex_unlock(&beat_mutex);
    return amp;
}

void* run_visualizer(void* thread_id)
{

    Visualizer visualizer;
    int image_num = std::rand() % image_files.size();
    visualizer.initialize(WIDTH, HEIGHT, image_files[image_num], option_fullscreen);

    uint32_t* pixels = new uint32_t[WIDTH*HEIGHT];
    visualizer.get_image_pixels(WIDTH, HEIGHT,image_files[image_num], pixels);
    visualizer.render();

    std::cout << "VISUALIZER\n";

    ImageManipulator manipulator(&visualizer, pixels,
                                 WIDTH, HEIGHT);

    bool changing_image = false;
    bool fading_overlay = false;
    clock_t last_image_change = clock();

    float frqs[N_FRQS];
    float amps[N_FRQS];

    while(1)
    {
        // Get the current frequency content
        pthread_mutex_lock(&content_mutex);
        int content_size = content.size();
        for (int z = 0; (z < N_FRQS  && z < content_size); z++)
        {
            frqs[z] = content[z].frq;
            amps[z] = content[z].pwr;
        }
        pthread_mutex_unlock(&content_mutex);

        // If the music is loud enough : Transform the image
        if (content_size > 0 && total_pwr > 0.001)
        {
            if (changing_image)
            {
                manipulator.set_image(pixels, WIDTH, HEIGHT);
                changing_image = false;
            }
            if (fading_overlay)
            {
                manipulator.clear_overlay();
                fading_overlay = false;
            }

            for (int content_idx = 0; (content_idx < N_FRQS  && content_idx < content_size); content_idx++)
            {
                float frq = get_adjusted_frq(frqs[content_idx]);
                float amp = get_adjusted_amp(amps[content_idx]);
                manipulator.transform_overlay(frq, amp, content_idx, N_FRQS);
            }
            manipulator.update_image();
        }
        // Else the music is too quiet : Fade the overlay
        else
        {
            manipulator.fade_overlay();
            manipulator.update_image();
            fading_overlay = true;
        }

        // Change the image on a timer
        if ((float)(clock()-last_image_change)/CLOCKS_PER_SEC >= IMG_CHANGE_TIME)
        {
            image_num = std::rand() % image_files.size();
            last_image_change = clock();
            visualizer.get_image_pixels(WIDTH, HEIGHT,image_files[image_num], pixels);
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
    srand (time(NULL));
    GetImages(&image_files);
    set_options(argc, argv);

    const int REC_BUF_SIZE = 4096 >> 2;
    const int FFT_BUF_SIZE = 32768 >> 2;
    const int SAMPLE_RATE = 44100; //Samples per sec
    PulseAudioRecorder recorder(REC_BUF_SIZE);
    SmallFFT fft(FFT_BUF_SIZE, 1.0/SAMPLE_RATE);

    pthread_t vis_thread;
    pthread_create(&vis_thread, NULL, run_visualizer, (void *)1);

    BeatDetector beat_det(REC_BUF_SIZE, 0.05);

    float* data = new float[FFT_BUF_SIZE*2];

    clock_t read_time = clock();
    while (1)
    {
        bool run_fft = true;
        // READING AUDIO BUFFER
        if (recorder.read_to_buf() >= 0)
        {
            // MONITOR PROCESSING TIME
            clock_t processing_time = clock();

            // GET NORMALIZATION
            float normalization_factor = 1.0 / (recorder.normalize_buffer() * NORMALIZATION_COEF + 1.0);

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

            // GETTING SIGNAL POWER OF NEWEST AUDIO CHUNK
            int beat_amp = beat_det.contains_beat();
            total_pwr = beat_det.get_power();
            if (beat_amp > 0)
            {
                pthread_mutex_lock(&beat_mutex);
                on_beat++;
                pthread_mutex_unlock(&beat_mutex);
            }

            // EXECUTING FFT
            memcpy(fft.m_data, data, FFT_BUF_SIZE*2*sizeof(float));
            if (run_fft)
            {
                static std::vector<struct FreqContent> content_tmp;
                content_tmp = fft.get_significant_frq(AMPL_THRESHOLD, 1.0, 25);

                pthread_mutex_lock(&content_mutex);
                content = content_tmp;
                pthread_mutex_unlock(&content_mutex);
            }
            run_fft = !run_fft;

            processing_time = clock() - processing_time;
            read_time = clock() - read_time;

            std::cout << "HAS BEAT  : " << on_beat << " " << beat_det.m_threshold << "\n";
            std::cout << "NORMALIZE : " << normalization_factor << "\n";
            std::cout << "TOTAL PWR : " << total_pwr << "\n";
            std::cout << "EXEC TIME : " << ((float)processing_time)/CLOCKS_PER_SEC << "\n";
            std::cout << "READ TIME : " << ((float)read_time)/CLOCKS_PER_SEC << "\n";
            for (unsigned int x = 0; x < content.size() && x < 5; x++)
            {
                std::cout << "FRQ : " << content[x].frq <<
                          " // AMPL: " << content[x].pwr << "\n";
            }
            fft.reset();
            std::cout << " ------------- \n";

            read_time = clock();
        }
    }

    delete data;
}
