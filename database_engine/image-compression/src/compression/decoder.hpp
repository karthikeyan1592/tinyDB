#ifndef DECODER_HPP
#define DECODER_HPP

#include <vector>
#include <string>
#include <opencv2/opencv.hpp> // Assuming OpenCV is used for image handling

namespace compression {

class Decoder {
public:
    cv::Mat decode(const std::vector<unsigned char>& compressedData);
};

} // namespace compression

#endif // DECODER_HPP