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
};

struct FreqContent
{
    int frq;
    double pwr;
}
