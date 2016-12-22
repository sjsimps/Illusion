
#include <vector>

struct FreqContent
{
    int frq;
    double pwr;
};

class SmallFFT
{
public:

    //Each even data element corresponds to real component
    //Each odd element corresponds to imaginary component
    float* m_data;

    int m_sample_width;
    double m_sample_period;

    SmallFFT(int sample_width, double sample_period);
    ~SmallFFT();

    void update_cfg();

    void comp_FFT();

    void reset();

    std::vector<struct FreqContent> get_significant_frq(double threshold,
                                                        int lower_frq_bound,
                                                        int lpf_size);
};

