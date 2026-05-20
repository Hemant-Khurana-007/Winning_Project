# Winning_Project
ts gonna win big gang

## App.py Overview

`App.py` is a Python script that demonstrates how to use the `wav_lib` shared library to process WAV files. It uses `ctypes` to interface with the C++ backend.

### Structure

The script follows a standard organization:
1.  **Library Loading**: It locates and loads the `libwav_lib.so` shared library from the `wav_lib/lib/` directory.
2.  **FFI Setup**: It defines the argument and return types for the C++ functions exposed by the library, ensuring type safety when calling them from Python.
3.  **Main Logic**: The `main()` function orchestrates the reading, processing, and writing of audio data.
4.  **Execution Hook**: Standard `if __name__ == "__main__":` block to run the script.

### Functions

#### `main()`
The primary Python function that:
- Defines input (`test.wav`) and output (`out.wav`) file paths.
- Initializes a WAV reader object via the library.
- Extracts metadata (sample rate, bit depth, channel count) and raw audio data.
- Checks if the file is 16-bit (currently required for processing).
- Calls the C++ `wav_apply_soft_clipping` function to modify the audio.
- Writes the result to disk using the library's writer function.
- Ensures resources are cleaned up (destroying the reader) in a `finally` block.

#### Library Functions (C++ Interface)
The script interfaces with several functions from `wav_lib`:
- `wav_reader_create()`: Allocates a new reader object.
- `wav_reader_open()`: Maps a WAV file into memory for reading.
- `wav_reader_get_*()`: A suite of functions to retrieve file metadata and data pointers.
- `wav_apply_soft_clipping()`: Applies a soft-clipping distortion algorithm (`y = tanh(k*x)`) directly to the memory-mapped audio data.
- `wav_writer_write()`: Encapsulates the logic for writing a valid WAV file with the provided data and metadata.
- `wav_reader_destroy()`: Frees the memory allocated for the reader.
