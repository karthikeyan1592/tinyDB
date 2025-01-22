#include <gtest/gtest.h>
#include "../src/utils/image_utils.hpp"

// Test case for loading an image
TEST(ImageUtilsTest, LoadImage) {
    // Assuming loadImage function returns a pointer to an Image object
    Image* img = loadImage("test_image.png");
    ASSERT_NE(img, nullptr);
    // Additional checks can be added here
    delete img; // Clean up
}

// Test case for saving an image
TEST(ImageUtilsTest, SaveImage) {
    Image img; // Assuming Image is a valid type
    // Assuming saveImage function returns a boolean indicating success
    bool result = saveImage("output_image.png", img);
    ASSERT_TRUE(result);
}

// Add more tests as needed