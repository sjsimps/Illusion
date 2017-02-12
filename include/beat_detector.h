class BeatDetector
{
public:
    BeatDetector(int buff_size, float prob_target);
    ~BeatDetector();
    int contains_beat();

    float* m_data;
    int m_data_size;
    float m_ampl;
    float m_threshold;
    int m_lowpass;
    float m_prob_beat;
    float m_prob_beat_target;
    int m_n_detections;
    int m_n_calls;
};
