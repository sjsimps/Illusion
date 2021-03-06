
#include <math.h>
#include <cmath>
#include <stdio.h>
#include <iostream>

#include "../include/beat_detector.h"

BeatDetector::BeatDetector(int buff_size, float prob_target)
{
    m_data = new float[buff_size];
    m_data_size = buff_size;
    m_lowpass = 1;
    m_n_detections = 0;
    m_n_calls = 0;
    m_threshold = 100;
    m_prob_beat = 0;
    m_prob_beat_target = prob_target;
}

BeatDetector::~BeatDetector()
{
    delete m_data;
}

double BeatDetector::get_power()
{
    double mean = 0.0;
    double power = 0.0;
    for (int x = 0; x < m_data_size; x++)
    {
        mean += m_data[x];
    }
    mean /= m_data_size;

    for (int x = 0; x < m_data_size; x++)
    {
        power += std::pow(m_data[x] - mean, 2);
    }
    power /= m_data_size;
    power /= 1000000.0;
    return power;
}

int BeatDetector::contains_beat()
{
    float delta = 0;
    float prev_ampl;
    int retval = 0;
    for (int x = 0; x < m_data_size; x++)
    {
        prev_ampl = m_ampl;
        m_ampl = (m_data[x] + (m_lowpass-1)*m_ampl)/m_lowpass;
        delta = std::abs(m_ampl - prev_ampl);
        //std::cout << m_ampl << " // " << delta << "\n";
        if (delta > m_threshold)
        {
            retval += delta/m_threshold;
        }
    }

    m_n_calls++;
    if (retval > 0)
    {
        m_n_detections++;
    }
    if (m_n_calls >= 100)
    {
        m_n_detections = m_n_detections >> 1;
        m_n_calls = m_n_calls >> 1;
    }
    m_prob_beat = (float)m_n_detections / (float)m_n_calls;
    m_threshold = m_threshold - 10*(m_prob_beat_target - m_prob_beat);
    if (m_threshold < 0) m_threshold = 0.0;
    return retval;
}

float m_ampl;
float m_threshold;
