class BeatDetector
{
public:
    BeatDetector(float threshold, int buff_size, float prob_target);
    ~BeatDetector();
    bool contains_beat();

    float* m_data;
    int m_data_size;
    float m_ampl;
    float m_threshold;
    int m_lowpass;
    float m_prob_beat;
    float m_prob_beat_target;
    float m_n_detections;
    float m_n_calls;
};
