#include <gtest/gtest.h>
#include "../src/compression/encoder.hpp"
#include "../src/compression/decoder.hpp"

TEST(CompressionTests, EncoderCanCompressImage) {
    Encoder encoder;
    // Assuming we have a function to create a test image
    Image testImage = createTestImage();
    auto compressedData = encoder.encode(testImage);
    
    ASSERT_FALSE(compressedData.empty());
}

TEST(CompressionTests, DecoderCanDecompressImage) {
    Decoder decoder;
    Encoder encoder;
    Image testImage = createTestImage();
    auto compressedData = encoder.encode(testImage);
    
    Image decompressedImage = decoder.decode(compressedData);
    
    ASSERT_EQ(testImage.getWidth(), decompressedImage.getWidth());
    ASSERT_EQ(testImage.getHeight(), decompressedImage.getHeight());
    // Further checks can be added to compare pixel data
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}