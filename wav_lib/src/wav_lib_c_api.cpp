#include "wav_lib.hpp"
#include <cmath>

extern "C" {
    void* wav_reader_create() {
        return new wav_lib::WavReader();
    }

    bool wav_reader_open(void* reader, const char* filename) {
        return static_cast<wav_lib::WavReader*>(reader)->open(filename);
    }

    size_t wav_reader_get_data_size(void* reader) {
        return static_cast<wav_lib::WavReader*>(reader)->data_size();
    }

    const uint8_t* wav_reader_get_data(void* reader) {
        return static_cast<wav_lib::WavReader*>(reader)->data();
    }

    uint16_t wav_reader_get_num_channels(void* reader) {
        return static_cast<wav_lib::WavReader*>(reader)->num_channels();
    }

    uint32_t wav_reader_get_sample_rate(void* reader) {
        return static_cast<wav_lib::WavReader*>(reader)->sample_rate();
    }

    uint16_t wav_reader_get_bits_per_sample(void* reader) {
        return static_cast<wav_lib::WavReader*>(reader)->bits_per_sample();
    }

    void wav_reader_generate_delayed_script(void* reader, const char* out_filename, const char* wav_filename) {
        static_cast<wav_lib::WavReader*>(reader)->generate_delayed_script(out_filename, wav_filename);
    }

    void wav_reader_destroy(void* reader) {
        delete static_cast<wav_lib::WavReader*>(reader);
    }

    void wav_apply_soft_clipping(int16_t* samples, size_t num_samples, float drive) {
        for (size_t i = 0; i < num_samples; ++i) {
            float x = samples[i] / 32768.0f;
            samples[i] = static_cast<int16_t>(tanhf(drive * x) * 32767.0f);
        }
    }

    bool wav_writer_write(const char* filename, const uint8_t* data, uint32_t data_size,
                         uint16_t num_channels, uint32_t sample_rate, uint16_t bits_per_sample) {
        return wav_lib::WavWriter::write(filename, data, data_size, num_channels, sample_rate, bits_per_sample);
    }
}
