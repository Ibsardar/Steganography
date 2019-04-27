#include "File2ImageStegoTools.h"

#include <string>

int main() {

    std::string img_filename = "tiger.bmp";
    std::string file_filename = "LAA.exe";
    int least_significant_bits = 7; // must be between 1 and 7, inclusive

    // encrypt file data into image data
    f2i_stego_tools::encrypt(img_filename, file_filename, least_significant_bits);

    // decrypt image and save both the image and data file
    f2i_stego_tools::decrypt("encrypted.bmp");

    return 0;
}



