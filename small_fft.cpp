
#include <iostream>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <vector>

#define PI 3.14159265358979323846

static const int TABLE_LEN = 2001;

using namespace std;

class SmallFFT
{
public:

    double* m_data;
    int m_sample_width;
    double m_sample_period;
    vector<double> m_frequencies;

    double m_sin_table[TABLE_LEN];
    double m_cos_table[TABLE_LEN];

    struct Config
    {
        int sample_width; //number_of_samples
        double sample_period; //milliseconds
        vector<double> sampled_frequencies; //which frequencies to track
    };
    struct Config m_cfg;

    SmallFFT();
    ~SmallFFT();

    void sin_cos(double a1, double* sin, double* cos);

    void update_cfg();

    double comp_amplitude(double frequency);

};

SmallFFT::SmallFFT()
{
    m_data = NULL;

    for (int index = 0; index < TABLE_LEN; index++)
    {
        m_sin_table[index] = sin(2 * PI * ((double)index / (double)TABLE_LEN));
        m_cos_table[index] = cos(2 * PI * ((double)index / (double)TABLE_LEN));
    }
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

double SmallFFT::comp_amplitude(double frequency /*Hz*/)
{
    frequency = frequency * PI * 2;

    double magnitude = 0;
    double sin, cos;
    double step = 0;
    double step_d = 1.0/m_sample_width * frequency;

    for(int x = 0; x < m_sample_width; x++)
    {
        sin_cos(step, &sin, &cos);
        magnitude += m_data[x] * (sin + cos);
        step += step_d;
    }

    magnitude = magnitude / m_sample_width;
    return magnitude;
}

void SmallFFT::sin_cos(double angle, double* sin_, double* cos_)
{
    // Fast sin + cos implementation
    double div2pi = angle / (2*PI);
    int index = (int)(div2pi * TABLE_LEN) % TABLE_LEN;

    *sin_ = m_sin_table[index];
    *cos_ = m_cos_table[index];
}

int main(int argc, char* argv[])
{
    SmallFFT fft;

    //44.1 kHz
    fft.m_cfg.sample_period = 1.0/44100.0;
    fft.m_cfg.sample_width = 44100;
    fft.update_cfg();

    for (int x = 0; x < 44100; x+=1)
    {
        fft.m_data[x] = ((x % 100) < 50) ? 1.0 : 0.0;
    }

    double ampl = 0;
    for (int x = 0; x < 21000; x+=1)
    {
        ampl = fft.comp_amplitude(x);
        if (ampl > 0.01 || ampl < -0.01)
        {
            cout << "FRQ: " << x << " / AMPL: " << ampl << "\n";
        }
    }
}
