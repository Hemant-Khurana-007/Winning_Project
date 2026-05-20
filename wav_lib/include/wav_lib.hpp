#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace wav_lib {

#pragma pack(push, 1)
struct RiffHeader {
    char chunk_id[4];     // "RIFF"
    uint32_t chunk_size;  // File size - 8
    char format[4];       // "WAVE"
};

struct ChunkHeader {
    char chunk_id[4];
    uint32_t chunk_size;
};

struct FmtChunk {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};
#pragma pack(pop)

class WavReader {
public:
    WavReader() : mmap_data_(nullptr), mmap_size_(0), fd_(-1), data_ptr_(nullptr), data_size_(0) {}
    ~WavReader() { close(); }

    bool open(const std::string& filename) {
        fd_ = ::open(filename.c_str(), O_RDONLY);
        if (fd_ == -1) return false;

        struct stat st;
        if (fstat(fd_, &st) == -1) {
            ::close(fd_);
            return false;
        }
        mmap_size_ = st.st_size;

        mmap_data_ = static_cast<uint8_t*>(mmap(nullptr, mmap_size_, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_, 0));
        if (mmap_data_ == MAP_FAILED) {
            ::close(fd_);
            return false;
        }

        if (mmap_size_ < sizeof(RiffHeader)) {
            close();
            return false;
        }

        const RiffHeader* riff = reinterpret_cast<const RiffHeader*>(mmap_data_);
        if (std::memcmp(riff->chunk_id, "RIFF", 4) != 0 ||
            std::memcmp(riff->format, "WAVE", 4) != 0) {
            close();
            return false;
        }

        size_t offset = sizeof(RiffHeader);
        bool found_fmt = false;
        bool found_data = false;

        while (offset + sizeof(ChunkHeader) <= mmap_size_) {
            const ChunkHeader* chunk = reinterpret_cast<const ChunkHeader*>(mmap_data_ + offset);
            offset += sizeof(ChunkHeader);

            if (std::memcmp(chunk->chunk_id, "fmt ", 4) == 0) {
                if (chunk->chunk_size >= sizeof(FmtChunk)) {
                    const FmtChunk* fmt = reinterpret_cast<const FmtChunk*>(mmap_data_ + offset);
                    num_channels_ = fmt->num_channels;
                    sample_rate_ = fmt->sample_rate;
                    bits_per_sample_ = fmt->bits_per_sample;
                    found_fmt = true;
                }
            } else if (std::memcmp(chunk->chunk_id, "data", 4) == 0) {
                data_ptr_ = mmap_data_ + offset;
                data_size_ = chunk->chunk_size;
                found_data = true;
                // We could break here if we only care about the first data chunk
            }

            offset += (chunk->chunk_size + 1) & ~1;
        }

        return found_fmt && found_data;
    }

    void close() {
        if (mmap_data_ && mmap_data_ != MAP_FAILED) {
            munmap(mmap_data_, mmap_size_);
            mmap_data_ = nullptr;
        }
        if (fd_ != -1) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    uint16_t num_channels() const { return num_channels_; }
    uint32_t sample_rate() const { return sample_rate_; }
    uint16_t bits_per_sample() const { return bits_per_sample_; }
    const uint8_t* data() const { return data_ptr_; }
    size_t data_size() const { return data_size_; }

    void generate_delayed_script(const std::string& out_filename, const std::string& wav_filename) {
        FILE* f = fopen(out_filename.c_str(), "w");
        if (!f) return;

        fprintf(f, "import time\n");
        fprintf(f, "import os\n\n");
        fprintf(f, "print('Processing %s with 1 second delay per frame...')\n", wav_filename.c_str());
        
        size_t frame_size = sample_rate_ * num_channels_ * (bits_per_sample_ / 8);
        size_t num_frames = data_size_ / frame_size;
        if (num_frames == 0 && data_size_ > 0) num_frames = 1;

        fprintf(f, "num_frames = %zu\n", num_frames);
        fprintf(f, "for i in range(num_frames):\n");
        fprintf(f, "    print(f'Processing frame {i+1}/{num_frames}...')\n");
        fprintf(f, "    time.sleep(1)\n");
        fprintf(f, "print('Done.')\n");

        fclose(f);
    }

    template<typename T>
    const T* samples() const { return reinterpret_cast<const T*>(data_ptr_); }
    size_t num_samples() const { return data_size_ / (bits_per_sample_ / 8); }

private:
    uint8_t* mmap_data_;
    size_t mmap_size_;
    int fd_;

    const uint8_t* data_ptr_;
    size_t data_size_;
    uint16_t num_channels_;
    uint32_t sample_rate_;
    uint16_t bits_per_sample_;
};

class WavWriter {
public:
    static bool write(const std::string& filename, const uint8_t* data, uint32_t data_size,
                      uint16_t num_channels, uint32_t sample_rate, uint16_t bits_per_sample) {
        
        int fd = ::open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) return false;

        RiffHeader riff;
        std::memcpy(riff.chunk_id, "RIFF", 4);
        riff.chunk_size = 4 + (sizeof(ChunkHeader) + sizeof(FmtChunk)) + (sizeof(ChunkHeader) + data_size);
        std::memcpy(riff.format, "WAVE", 4);

        ChunkHeader fmt_header;
        std::memcpy(fmt_header.chunk_id, "fmt ", 4);
        fmt_header.chunk_size = sizeof(FmtChunk);

        FmtChunk fmt;
        fmt.audio_format = 1; // PCM
        fmt.num_channels = num_channels;
        fmt.sample_rate = sample_rate;
        fmt.bits_per_sample = bits_per_sample;
        fmt.byte_rate = sample_rate * num_channels * bits_per_sample / 8;
        fmt.block_align = num_channels * bits_per_sample / 8;

        ChunkHeader data_header;
        std::memcpy(data_header.chunk_id, "data", 4);
        data_header.chunk_size = data_size;

        // Use writev for a single syscall or just sequential writes
        if (::write(fd, &riff, sizeof(riff)) != sizeof(riff)) { ::close(fd); return false; }
        if (::write(fd, &fmt_header, sizeof(fmt_header)) != sizeof(fmt_header)) { ::close(fd); return false; }
        if (::write(fd, &fmt, sizeof(fmt)) != sizeof(fmt)) { ::close(fd); return false; }
        if (::write(fd, &data_header, sizeof(data_header)) != sizeof(data_header)) { ::close(fd); return false; }
        
        size_t total_written = 0;
        while (total_written < data_size) {
            ssize_t written = ::write(fd, data + total_written, data_size - total_written);
            if (written <= 0) break;
            total_written += written;
        }

        ::close(fd);
        return total_written == data_size;
    }
};

} // namespace wav_lib
