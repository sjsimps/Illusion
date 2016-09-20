
#include <iostream>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <vector>

using namespace std;

class SmallFFT
{
public:
    SmallFFT();
    ~SmallFFT();

    double* m_data;
    int m_sample_width;
    double m_sample_period;
    vector<double> m_frequencies;

    struct Config
    {
        int sample_width; //number_of_samples
        double sample_period; //milliseconds
        vector<double> sampled_frequencies; //which frequencies to track
    };
    struct Config m_cfg;

    void sin_cos(double a1, double* sin, double* cos);

    void update_cfg();

    double comp_amplitude(double frequency);

};

SmallFFT::SmallFFT()
{
    m_data = NULL;
}

SmallFFT::~SmallFFT()
{
    if (m_data != NULL)
    {
        delete m_data;
    }
}

void SmallFFT::update_cfg()
{
    m_frequencies = m_cfg.sampled_frequencies;
    m_sample_width = m_cfg.sample_width;
    m_sample_period = m_cfg.sample_period;

    if (m_data != NULL)
    {
        delete m_data;
    }
    m_data = new double[m_sample_width];
}

double SmallFFT::comp_amplitude(double frequency)
{
    double magnitude = 0;
    double step = 0;
    double span = m_sample_width * m_sample_period;
    double sin, cos;
    for(int x = 0; x < m_sample_width; x++)
    {
        sin_cos(frequency * step, &sin, &cos);

        magnitude += m_data[x] * ( sin + cos );
        step += m_sample_period;
    }

    magnitude = magnitude * 2 / span;
    return magnitude;
}

void SmallFFT::sin_cos(double a1, double* sin, double* cos)
{
    // Fast sin + cos implementation

    double a2 = a1 * a1;
    double a4 = a2 * a2;
    double a6 = a4 * a2;
    //sin = a1 - a3/3! + a5/5! - a7/7! ...
    *sin = a1 * (1 - a2/6 + a4/120);
    //cos = 1 - a2/2! + a4/4! - a6/6! ...
    *cos = 1 - a2/2 + a4/24 - a6/720;
}

int main(int argc, char* argv[])
{
    SmallFFT fft;
    clock_t begin_time, end_time;
    double sin_, cos_;


    unsigned int n_loops = 100000000;

    begin_time = clock();
    for (unsigned int x = 0; x < n_loops; x++)
    {
        fft.sin_cos(x,&sin_,&cos_);
    }
    end_time = clock();
    cout << begin_time - end_time << "\n";

    begin_time = clock();
    for (unsigned int x = 0; x < n_loops; x++)
    {
        sin_ = sin(x);
        cos_ = cos(x);
    }
    end_time = clock();
    cout << begin_time - end_time << "\n";
}
