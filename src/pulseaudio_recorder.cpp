/***
  This file is part of PulseAudio.
  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.
  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#include "../include/pulseaudio_recorder.h"

#define NORMALIZE_STEP 10

PulseAudioRecorder::PulseAudioRecorder(int buf_size)
{
    /* The sample type to use */
    m_sample_rate = 44100;
    m_spec = {
        .format = PA_SAMPLE_S16LE,
        .rate = m_sample_rate,
        .channels = 1
    };
    m_simple = NULL;
    m_buf_size = buf_size;

    int error;
    /* Create the recording stream */
    if (!(m_simple = pa_simple_new(NULL, NULL, PA_STREAM_RECORD, NULL, "record", &m_spec, NULL, NULL, &error)))
    {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
    }
    m_buf = new int16_t[m_buf_size];
    for (int x = 0; x < m_buf_size; x++)
    {
        m_buf[x] = 0;
    }

    m_max_signal = 1;
}

PulseAudioRecorder::~PulseAudioRecorder()
{
    if (m_simple)
    {
        pa_simple_free(m_simple);
    }
    if (m_buf)
    {
        delete m_buf;
    }
}

void PulseAudioRecorder::print_buf()
{
    int x;
    for (x = 0; x < m_buf_size; x++)
    {
        // It seems the buffer will not fill if the consumption if
        // faster than production of sound date. Therefore EOT is when
        // the first zero is encountered.
        //if (m_buf[x] == 0) break;//return;
        std::cout << m_buf[x] << "\n" ;
    }
}

float PulseAudioRecorder::normalize_buffer()
{
    float new_max = 0;
    for (int x = 0; x < m_buf_size; x+=NORMALIZE_STEP)
    {
        if (std::abs(m_buf[x]) > new_max)
        {
            new_max = std::abs(m_buf[x]);
        }
    }
    if (new_max > m_max_signal)
    {
        m_max_signal = new_max;
    }
    else
    {
        int lowpass = 50;
        m_max_signal = (new_max + m_max_signal*(lowpass-1) ) / lowpass;
    }
    std::cout << (float)(m_max_signal) / float(INT16_MAX) << "\n";
    return (float)(m_max_signal) / float(INT16_MAX);
}

int PulseAudioRecorder::read_to_buf()
{
    int error;
    /* Record some data ... */
    if (pa_simple_read(m_simple, m_buf, m_buf_size*sizeof(int16_t), &error) < 0)
    {
        fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
    }
    return error;
}

