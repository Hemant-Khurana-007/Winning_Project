#include "wav_lib.hpp"
#include <iostream>
#include <chrono>
#include <vector>

int main(int argc, char** argv) {
    const std::string input_file = (argc > 1) ? argv[1] : "test.wav";
    const std::string output_file = "output_optimized.wav";

    auto start_read = std::chrono::high_resolution_clock::now();

    wav_lib::WavReader reader;
    if (!reader.open(input_file)) {
        std::cerr << "Failed to open input file: " << input_file << std::endl;
        return 1;
    }

    auto end_read = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_read = end_read - start_read;

    std::cout << "--- WAV Read Stats ---" << std::endl;
    std::cout << "File: " << input_file << std::endl;
    std::cout << "Data size: " << reader.data_size() << " bytes" << std::endl;
    std::cout << "Sample Rate: " << reader.sample_rate() << " Hz" << std::endl;
    std::cout << "Channels: " << reader.num_channels() << std::endl;
    std::cout << "Bits per Sample: " << reader.bits_per_sample() << std::endl;
    std::cout << "Read Time (mapping): " << elapsed_read.count() << " seconds" << std::endl;
    // Simulate some simple processing (e.g., gain adjustment) to touch the memory
    // and measure actual throughput if desired.
    /*
    auto start_process = std::chrono::high_resolution_clock::now();
    if (reader.bits_per_sample() == 16) {
        const int16_t* samples = reader.samples<int16_t>();
        size_t n = reader.num_samples();
        volatile int16_t sum = 0;
        for (size_t i = 0; i < n; ++i) sum += samples[i];
    }
    auto end_process = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_process = end_process - start_process;
    std::cout << "Process Time (data access): " << elapsed_process.count() << " seconds" << std::endl;
    */

    auto start_write = std::chrono::high_resolution_clock::now();

    if (!wav_lib::WavWriter::write(output_file, reader.data(), reader.data_size(),
                                  reader.num_channels(),
                                  reader.sample_rate(),
                                  reader.bits_per_sample())) {
        std::cerr << "Failed to write output file: " << output_file << std::endl;
        return 1;
    }

    auto end_write = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_write = end_write - start_write;

    std::cout << "\n--- WAV Write Stats ---" << std::endl;
    std::cout << "Output: " << output_file << std::endl;
    std::cout << "Write Time (mmap): " << elapsed_write.count() << " seconds" << std::endl;
    std::cout << "Throughput: " << (reader.data_size() / (1024.0 * 1024.0)) / elapsed_write.count() << " MB/s" << std::endl;

    return 0;
}
