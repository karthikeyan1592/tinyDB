#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <vector>
#include <string>
#include <opencv2/opencv.hpp> // Assuming OpenCV is used for image handling

namespace ImageCompression {

class Encoder {
public:
    // Method to encode an image and return a compressed byte array
    std::vector<unsigned char> encode(const cv::Mat& image);

private:
    // Additional private methods for compression algorithms can be declared here
};

} // namespace ImageCompression

#endif // ENCODER_HPP