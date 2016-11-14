
#include <iostream>
#include <math.h>
#include <cmath>
#include <cstring>
#include <algorithm>

#include "small_fft.h"

using namespace std;

SmallFFT::SmallFFT(int sample_width, double sample_period)
{
    m_sample_width = sample_width;
    m_sample_period = sample_period;
    m_data = new float[m_sample_width*2];
}

SmallFFT::~SmallFFT()
{
    if (m_data != NULL)
    {
        delete m_data;
    }
}

//Source : http://www.drdobbs.com/cpp/a-simple-and-efficient-fft-implementatio/199500857?pgno=1
void SmallFFT::comp_FFT()
{
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    // reverse-binary reindexing
    n = m_sample_width<<1;
    j=1;
    for (i=1; i<n; i+=2) {
        if (j>i) {
            swap(m_data[j-1], m_data[i-1]);
            swap(m_data[j], m_data[i]);
        }
        m = m_sample_width;
        while (m>=2 && j>m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    };

    // here begins the Danielson-Lanczos section
    mmax=2;
    while (n>mmax) {
        istep = mmax<<1;
        theta = -(2*M_PI/mmax);
        wtemp = sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m=1; m < mmax; m += 2) {
            for (i=m; i <= n; i += istep) {
                j=i+mmax;
                tempr = wr*m_data[j-1] - wi*m_data[j];
                tempi = wr * m_data[j] + wi*m_data[j-1];

                m_data[j-1] = m_data[i-1] - tempr;
                m_data[j] = m_data[i] - tempi;
                m_data[i-1] += tempr;
                m_data[i] += tempi;
            }
            wtemp=wr;
            wr += wr*wpr - wi*wpi;
            wi += wi*wpr + wtemp*wpi;
        }
        mmax=istep;
    }
}

bool compareByAmpl(const struct FreqContent a, const struct FreqContent b)
{
    return std::abs(a.pwr) > std::abs(b.pwr);
}

std::vector<struct FreqContent> SmallFFT::get_significant_frq(double threshold, int lower_frq_bound)
{
    std::vector<struct FreqContent> retval;
    comp_FFT();
    for (int x = lower_frq_bound; x < m_sample_width/3; x+=2)
    {
        double ampl = m_data[x];
        if (ampl > threshold || ampl < -threshold)
        {

            struct FreqContent content;
            content.frq = (double)x/3;
            content.pwr = ampl;
            retval.push_back(content);
        }
    }
    std::sort(retval.begin(), retval.end(), compareByAmpl);
    return retval;
}

void SmallFFT::reset()
{
    memset(m_data, 0, (m_sample_width*2)*sizeof(float));
}

/*
int main(int argc, char* argv[])
{
    SmallFFT fft(65536, 1.0/65536.0);

    for (int x = 0; x < 65536*2; x+=2)
    {
        //Signal:
        fft.m_data[x] = cos(2.0*M_PI*(double)x*(100.0/65536.0));
        //fft.m_data[x] = ((x % (6000) ) < (3000) ) ? 1.0 : 0.0;
    }

    fft.get_significant_frq(100.0);
}
*/
