# Contents of /image-compression/image-compression/README.md

# Image Compression Project

This project implements various image compression techniques. It includes both encoding and decoding functionalities, allowing users to compress images into byte arrays and decompress them back to their original form.

## Project Structure

- `src/compression/`: Contains the implementation of the compression algorithms.
  - `encoder.cpp` / `encoder.hpp`: Implementation and declaration of the `Encoder` class.
  - `decoder.cpp` / `decoder.hpp`: Implementation and declaration of the `Decoder` class.
  
- `src/utils/`: Contains utility functions for image processing.
  - `image_utils.cpp` / `image_utils.hpp`: Implementation and declaration of image utility functions.

- `src/main.cpp`: The entry point of the application.

- `tests/`: Contains unit tests for the compression and utility functionalities.
  - `compression_tests.cpp`: Tests for the `Encoder` and `Decoder` classes.
  - `utils_tests.cpp`: Tests for the utility functions.

- `include/compression/`: Contains header files for types and constants used in the compression module.
  - `types.hpp`: Defines types and structures.
  - `constants.hpp`: Defines constants used in the algorithms.

- `CMakeLists.txt`: Configuration file for building the project.

## Setup Instructions

1. Clone the repository.
2. Navigate to the project directory.
3. Run `cmake .` to configure the project.
4. Build the project using `make`.

## Usage

To use the image compression functionalities, include the necessary headers and create instances of the `Encoder` and `Decoder` classes. Refer to the source files for detailed usage examples.