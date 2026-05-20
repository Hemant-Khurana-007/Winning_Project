import ctypes
import time
import os

# Load the shared library
lib_path = os.path.abspath("wav_lib/lib/libwav_lib.so")
wav_lib = ctypes.CDLL(lib_path)

# Define argument and return types
wav_lib.wav_reader_create.restype = ctypes.c_void_p
wav_lib.wav_reader_open.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
wav_lib.wav_reader_open.restype = ctypes.c_bool
wav_lib.wav_reader_get_data_size.argtypes = [ctypes.c_void_p]
wav_lib.wav_reader_get_data_size.restype = ctypes.c_size_t
wav_lib.wav_reader_get_data.argtypes = [ctypes.c_void_p]
wav_lib.wav_reader_get_data.restype = ctypes.POINTER(ctypes.c_uint8)
wav_lib.wav_reader_get_num_channels.argtypes = [ctypes.c_void_p]
wav_lib.wav_reader_get_num_channels.restype = ctypes.c_uint16
wav_lib.wav_reader_get_sample_rate.argtypes = [ctypes.c_void_p]
wav_lib.wav_reader_get_sample_rate.restype = ctypes.c_uint32
wav_lib.wav_reader_get_bits_per_sample.argtypes = [ctypes.c_void_p]
wav_lib.wav_reader_get_bits_per_sample.restype = ctypes.c_uint16
wav_lib.wav_reader_destroy.argtypes = [ctypes.c_void_p]

# New functions
wav_lib.wav_apply_soft_clipping.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_float]
wav_lib.wav_writer_write.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint16, ctypes.c_uint32, ctypes.c_uint16]
wav_lib.wav_writer_write.restype = ctypes.c_bool

def main():
    input_file = "test.wav"
    output_file = "out.wav"
    drive = 2.0 # k parameter for soft clipping
    
    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found.")
        return

    # Create reader
    reader = wav_lib.wav_reader_create()
    
    try:
        # Open/Map operation
        success = wav_lib.wav_reader_open(reader, input_file.encode('utf-8'))
        
        if not success:
            print(f"Failed to open {input_file}")
            return

        # Get metadata
        data_size = wav_lib.wav_reader_get_data_size(reader)
        channels = wav_lib.wav_reader_get_num_channels(reader)
        sample_rate = wav_lib.wav_reader_get_sample_rate(reader)
        bits = wav_lib.wav_reader_get_bits_per_sample(reader)
        data_ptr = wav_lib.wav_reader_get_data(reader)

        print(f"--- WAV Info ---")
        print(f"File: {input_file}")
        print(f"Data size: {data_size} bytes")
        print(f"Format: {sample_rate}Hz, {channels} channels, {bits}-bit")

        if bits != 16:
            print(f"Error: Only 16-bit WAV files are supported for processing in this script.")
            return

        print(f"\nProcessing {input_file} to {output_file}...")
        print(f"Applying soft clipping drive (y = tanh(k*x)) with k={drive}...")

        start_process = time.perf_counter()
        
        # Apply soft clipping formula in C++
        num_samples = data_size // 2
        wav_lib.wav_apply_soft_clipping(data_ptr, num_samples, drive)

        # Write result using the library's writer
        success = wav_lib.wav_writer_write(
            output_file.encode('utf-8'),
            data_ptr,
            data_size,
            channels,
            sample_rate,
            bits
        )

        if not success:
            print(f"Failed to write {output_file}")
            return

        process_time = time.perf_counter() - start_process
        print(f"Done. Process Time: {process_time:.6f} seconds")
        print(f"Output saved to {output_file}")

    finally:
        wav_lib.wav_reader_destroy(reader)

if __name__ == "__main__":
    main()
