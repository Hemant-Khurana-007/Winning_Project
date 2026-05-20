# Optimized C++ WAV Library

A high-performance C++ library for reading and writing WAV files, optimized for speed using memory-mapped I/O and low-level system calls.

## Features
- **Zero-copy Reading:** Uses `mmap` to map WAV files directly into memory.
- **Fast Writing:** Uses memory mapping or optimized system calls for high throughput.
- **Robust Parsing:** Properly handles WAV chunks (e.g., skips metadata/INFO chunks to find sample data).
- **Modern C++:** Clean, header-only design (optional).

## Usage

```cpp
#include "wav_lib.hpp"

// Reading
wav_lib::WavReader reader;
if (reader.open("input.wav")) {
    const int16_t* samples = reader.samples<int16_t>();
    size_t count = reader.num_samples();
    // Process samples...
}

// Writing
wav_lib::WavWriter::write("output.wav", data_ptr, data_size, channels, sample_rate, bits_per_sample);
```

## Performance
Designed for low-latency and high-throughput audio applications.
- **Read:** Mapping is O(1) regardless of file size.
- **Write:** Optimized for sequential disk I/O.
