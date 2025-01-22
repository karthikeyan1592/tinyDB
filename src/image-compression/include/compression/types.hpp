#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <vector>

namespace compression {

struct ImageData {
    std::vector<uint8_t> data; // Raw image data
    int width;                 // Width of the image
    int height;                // Height of the image
    int channels;              // Number of color channels (e.g., RGB has 3 channels)
};

} // namespace compression

#endif // TYPES_HPP