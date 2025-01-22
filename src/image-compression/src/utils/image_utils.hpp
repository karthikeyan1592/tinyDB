#ifndef IMAGE_UTILS_HPP
#define IMAGE_UTILS_HPP

#include <string>
#include <vector>

class ImageUtils {
public:
    static std::vector<unsigned char> loadImage(const std::string& filePath);
    static void saveImage(const std::string& filePath, const std::vector<unsigned char>& imageData);
};

#endif // IMAGE_UTILS_HPP