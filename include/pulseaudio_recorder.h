#include <pulse/simple.h>
#include <pulse/error.h>

class PulseAudioRecorder
{
public:
    PulseAudioRecorder(int buf_size);
    ~PulseAudioRecorder();

    int read_to_buf();
    void print_buf();
    float normalize_buffer();

    pa_simple* m_simple; //Noted as "s"
    pa_sample_spec m_spec;
    int16_t* m_buf;
    int m_buf_size;
    uint32_t m_sample_rate;
    float m_max_signal;
};
