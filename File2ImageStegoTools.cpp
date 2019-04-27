////////////////////////////////////////////////////////////////////////////////
//
//  Author:         Ibrahim Sardar
//  Class:          CSCI 557
//  Filename:       File2ImageStegoTools.cpp
//  Date:           04/20/2018
//  Description:    Main implementation for Stenography: File-to-Image Tools namespace.
//
////////////////////////////////////////////////////////////////////////////////
//
//  Honor Pledge:
//
//  I pledge that I have neither given nor received any help on this project.
//
//  ibsardar
//
////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018 Copyright Holder All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "File2ImageStegoTools.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <sstream>
#include "math.h"
#include "CImg.h"

namespace f2i_stego_tools {

    //
    // structures
    //

    struct Header {
        std::bitset<32> f_size;                 // number bits file data is taking up
        std::bitset<11> h_size;                 // number bits header is taking up
        std::bitset<3> lsbs;                    // least significant bits
        std::bitset<2> code;                    // "11" = file to image encryption
        std::bitset<8> separator;               // separates header expressions
        std::bitset<8> separator2;              // separates header expressions (backup just in case)
        std::bitset<11> width;                  // width of image
        std::bitset<11> height;                 // height of image
        std::vector<std::bitset<8> > extension; // extension of data file
    };

    //
    // functions
    //

    int s2i(std::string str) {
        // converts string to int
        std::stringstream ss(str);
        int i;
        ss >> i;
        return i;
    }

    std::string i2s (int i) {
        // converts int to string
        std::stringstream ss;
        std::string str;
        ss << i;
        ss >> str;
        return str;
    }

    unsigned int b2ui(unsigned char c) {

        // convert character byte into binary int
        std::bitset<8> binary(c);
        return (unsigned int)(binary.to_ulong());
    }

    unsigned char ui2b(unsigned int i) {

        // convert binary int into character byte
        std::bitset<8> binary(i);
        return (unsigned char)(binary.to_ulong());
    }

    unsigned int bs2ui(std::bitset<8>& byte) {

        // convert byte into int value
        return (unsigned int)(byte.to_ulong());
    }

    unsigned char bs2b(std::bitset<8> binary) {

        // static cast bitset to u-char
        return static_cast<unsigned char>(binary.to_ulong());
    }

    std::bitset<8> s2bs(std::string s) {

        // place binary string into a bitset
        return std::bitset<8>(s);
    }

    std::bitset<8> ui2bs(unsigned int i) {

        // place binary int into a bitset
        return std::bitset<8> (i);
    }

    std::bitset<8> b2bs(unsigned char c) {

        // place binary character into a bitset
        return std::bitset<8> (c);
    }

    std::vector<std::bitset<8> > get_data_from_img(cimg_library::CImg<unsigned char>& img) {
        std::vector<std::bitset<8> > bytes;
        for (int y=0; y<img.height(); y++) {
            for (int x=0; x<img.width(); x++) {
                bytes.push_back(b2bs(img(x,y,0))); //R
                bytes.push_back(b2bs(img(x,y,1))); //G
                bytes.push_back(b2bs(img(x,y,2))); //B
            }
        }

        return bytes;
    }

    std::vector<std::bitset<8> > get_extension_data(std::string f) {
        std::vector<std::bitset<8> > bytes;
        int index = (int) f.find_last_of(".");
        std::string ext = f.substr(index);
        for (int i=1; i<ext.size(); i++) {  // skip the "." character
            bytes.push_back(b2bs(ext[i]));
        }
        return bytes;
    }

    Header create_header_data(cimg_library::CImg<unsigned char>& img,
                              std::vector<std::bitset<8> >& e, int code,
                              int bits, std::vector<std::bitset<8> >& fdata){
        Header h;
        h.width = std::bitset<11> (img.width());
        h.height = std::bitset<11> (img.height());
        h.code = std::bitset<2> (i2s(code));
        h.lsbs = std::bitset<3> (bits);
        h.separator = std::bitset<8>('x');
        h.separator2 = std::bitset<8>('\n');
        h.extension = e;
        // code,least significant bits,x,width,x,height,x,f_size,x,extension,x,
        h.h_size = std::bitset<11>(2+3+8+11+8+11+8+32+8+(e.size()*8)+8);
        // number of bits making up the data file to encrypt
        h.f_size = std::bitset<32>(fdata.size()*8);
        return h;
    }

    void encrypt(std::string img_filename, std::string file_filename,
                 int least_significant_bits) {

        //
        //  Preparation
        //

        // read binary
        std::ifstream ifs(file_filename.c_str(), std::ios::binary|std::ios::in);

        // couldn't read :(
        if (!ifs) {
            std::cout << "ERROR: Data file could not be opened." << std::endl;
            return;
        }

        // get binary bitsets from data file to encrypt
        std::vector<std::bitset<8> > fdata;
        while (ifs.peek() != -1) {
            fdata.push_back(b2bs(ifs.get()));
        }

        // close the filestream
        ifs.close();

        // exception if something went wrong
        if (ifs.bad()) {
            std::cout << "ERROR: Data file could not be opened properly." << std::endl;
            return;
        }

        // get CImg representation
        cimg_library::CImg<unsigned char> img(img_filename.c_str());

        // get binary bitsets from image to encrypt
        std::vector<std::bitset<8> > idata = get_data_from_img(img);

        // get extension of file into bitset vector
        std::vector<std::bitset<8> > extension = get_extension_data(file_filename);
        int ext_size = 8 * extension.size();

        // construct header data
        Header hdata = create_header_data(img, extension, 11, least_significant_bits, fdata);

        //
        //  Validate input size
        //

        // disallow encryption if file cannot fit into image
        int total_bits = hdata.h_size.to_ulong() + hdata.f_size.to_ulong();
        int bytes_available = idata.size();
        int bytes_needed = (int)ceil((double)total_bits/least_significant_bits);
        int result = bytes_available - bytes_needed;
        int result_bits = bytes_available*least_significant_bits - total_bits;
        std::cout<<"("<<total_bits<<" encryption bits/"<<bytes_available*8<<" image bits/"<<least_significant_bits<<" least significant bits)"<<std::endl;
        if (result < 0) {
            std::cout<<"ERROR: Data file is too large/image file is too small."<<std::endl;
            std::cout<<-1*result<<" bytes ("<<-1*result_bits<<" bits or "<<-1*result/3<<" pixels) are needed to encrypt the given image."<<std::endl;
            return;
        } else {
            std::cout<<"There is a surplus of "<<result<<" bytes ("<<result_bits<<" bits or "<<result/3<<" pixels) in the image."<<std::endl;
        }

        //
        //  Gather bits to encrypt
        //

        // add header data to list of booleans
        std::vector<bool> bools;
        //code
        bools.push_back(hdata.code[0]); bools.push_back(hdata.code[1]);
        //lsbs
        bools.push_back(hdata.lsbs[0]); bools.push_back(hdata.lsbs[1]);
        bools.push_back(hdata.lsbs[2]);
        //separator
        for (int i=0;i<8;i++) bools.push_back(hdata.separator[i]);
        //width
        for (int i=0;i<11;i++) bools.push_back(hdata.width[i]);
        //separator
        for (int i=0;i<8;i++) bools.push_back(hdata.separator[i]);
        //height
        for (int i=0;i<11;i++) bools.push_back(hdata.height[i]);
        //separator
        for (int i=0;i<8;i++) bools.push_back(hdata.separator[i]);
        //extension
        for (int i=0;i<hdata.extension.size();i++)
            for (int j=0;j<8;j++)
                bools.push_back(hdata.extension[i][j]);
        //separator (backup)
        for (int i=0;i<8;i++) bools.push_back(hdata.separator2[i]);
        //data file size (in bits)
        for (int i=0;i<32;i++) bools.push_back(hdata.f_size[i]);
        //separator
        for (int i=0;i<8;i++) bools.push_back(hdata.separator[i]);

        // add file data to boolean list
        for(int i=0; i<fdata.size(); i++) {
            for (int j=0; j<8; j++) {
                bools.push_back(fdata[i][j]);
            }
        }

        //
        // encrypt
        //

        // helper vars
        int changed = 0;
        unsigned char rgb_tmp[3];
        std::bitset<8> pixdata[3] = {std::bitset<8>(0), std::bitset<8>(0), std::bitset<8>(0)};

        // First encrypt the first 5 bits of bools into the first 4 bytes (lsbs=2 for first 2 bits, =1 for next 3 bits)
        pixdata[0] = idata[0]; pixdata[1] = idata[1]; pixdata[2] = idata[2];
        pixdata[0][0] = bools[0]; pixdata[0][1] = bools[1]; pixdata[1][0] = bools[2]; pixdata[2][0] = bools[3];
        rgb_tmp[0] = bs2b(pixdata[0]); rgb_tmp[1] = bs2b(pixdata[1]); rgb_tmp[2] = bs2b(pixdata[2]);
        img.draw_point(0,0,rgb_tmp);
        changed++;
        pixdata[0] = idata[3];
        pixdata[0][0] = bools[4];

        // encrypt the rest
        for (int i=32,j=5;j<bools.size();i++) { // i=image index, j=data index (bools)
            int bit = i % 8;
            int byte = i / 8;
            int color = byte % 3; //R=0, G=1, B=2
            int pixel = byte / 3;
            int x = pixel % img.width();
            int y = pixel / img.width();
            bool zone = (bit >= hdata.lsbs.to_ulong() ? 0 : 1); // zone indicates whether current bit is in lsbs area
            // if zone is 0, fill an idata bit, if zone is 1, fill a bools bit
            if (zone == 0) {
                pixdata[color][bit] = idata[byte][bit];
            } else if (zone == 1) {
                pixdata[color][bit] = bools[j];
                j++;
            }
            // draw a pixel if the index is at the end of a set of 3 bytes OR
            //              if end of encryption data reached
            if ((color == 2 && bit == 7) || j==bools.size()-1) {
                unsigned char rgb[3] = {bs2b(pixdata[0]), bs2b(pixdata[1]), bs2b(pixdata[2])};
                img.draw_point(x,y,rgb);
                changed++;
            }
        }
        std::string new_img_filename = "encrypted.bmp";
        img.save(new_img_filename.c_str());
        std::cout<<changed<<"/"<<img.width()*img.height()<<" pixels were encrypted."<<std::endl;
        std::cout << "Encryption successful; saved as \"encrypted.bmp\"." << std::endl;
    }

    void decrypt(std::string fp) {

        //
        //  Read image file via CImg
        //

        // get CImg representation
        cimg_library::CImg<unsigned char> img(fp.c_str());

        // get image data
        std::vector<std::bitset<8> > img_data = get_data_from_img(img);

        //
        //  Gather header info
        //

        // this will store gathered header info
        Header h;

        // bits >> code, lsbs
        // first 4 bytes contain the code and least significant bit amount
        h.code[0] = img_data[0][0];
        h.code[1] = img_data[0][1];
        h.lsbs[0] = img_data[1][0];
        h.lsbs[1] = img_data[2][0];
        h.lsbs[2] = img_data[3][0];
        int code = h.code.to_ulong();
        int lsbs = h.lsbs.to_ulong();
        // validate
        if (code != 3 || (lsbs < 1 || lsbs > 7)) {
            std::cout << "ERROR: Encryption corrupted." << std::endl;
            return;
        }

        // bits >> separator #1
        int curr_img_bit = 32, curr_encoded_bit = 5, bit_cnt = 0;
        for (int i=curr_img_bit, j=curr_encoded_bit;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.separator[bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+8) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }
        // validate
        if (bs2b(h.separator) != 'x') {
            std::cout << "ERROR: Encryption corrupted." << std::endl;
            return;
        }

        // bits >> width, separator #2, height
        for (int i=curr_img_bit, j=curr_encoded_bit, bit_cnt=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.width[bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+11) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }
        for (int i=curr_img_bit, j=curr_encoded_bit, bit_cnt=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.separator[bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+8) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }
        for (int i=curr_img_bit, j=curr_encoded_bit, bit_cnt=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.height[bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+11) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }
        // validate
        if (h.width.to_ulong() != img.width() ||
            bs2b(h.separator) != 'x' ||
            h.height.to_ulong() != img.height()) {
            std::cout << "ERROR: Encryption corrupted." << std::endl;
            return;
        }

        // bits >> separator #3, extension, separator #4
        for (int i=curr_img_bit, j=curr_encoded_bit, bit_cnt=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.separator[bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+8) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }
        h.extension.push_back(std::bitset<8>());
        for (int i=curr_img_bit, j=curr_encoded_bit, index=0, bit_cnt=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.extension[index][bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+8) { // if 1 byte completed
                curr_img_bit = 1+i; curr_encoded_bit = j; index++; bit_cnt=0;
                if (bs2b(h.extension[index-1]) == '\n') { h.extension.pop_back(); break; }
                h.extension.push_back(std::bitset<8>());
            }
        }
        // validate
        if (bs2b(h.separator) != 'x') {
            std::cout << "ERROR: Encryption corrupted." << std::endl;
            return;
        }

        // bits >> f_size, separator #5
        for (int i=curr_img_bit, j=curr_encoded_bit, bit_cnt=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.f_size[bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+32) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }
        for (int i=curr_img_bit, j=curr_encoded_bit, bit_cnt=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { h.separator[bit_cnt] = img_data[byte][bit]; j++; bit_cnt++; }
            if (j >= curr_encoded_bit+8) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }
        // validate
        if (bs2b(h.separator) != 'x') {
            std::cout << "ERROR: Encryption corrupted." << std::endl;
            return;
        }

        // this will store the encrypted file's data, bit by bit
        std::vector<bool> f_data;

        // size of encrypted data in bits
        long f_size = h.f_size.to_ulong();

        // loop through each bit until we reach the limit (f_size)
        for (int i=curr_img_bit, j=0;; i++) { // j = encoded bit, i = image bit
            int bit = i % 8, byte = i / 8;
            bool zone = (bit >= lsbs ? 0 : 1); // zone indicates whether current bit is in lsbs area
            if (zone) { f_data.push_back(img_data[byte][bit]); j++; }
            if (j >= f_size) { curr_img_bit = ++i; curr_encoded_bit = j; break; }
        }

        // get extension for filename
        std::string ext = ".";
        for (int i=0; i<h.extension.size(); i++)
            ext += bs2b(h.extension[i]);

        // report
        std::cout<<"---------------------------\n";
        std::cout<<"Decrypted with header info:\n";
        std::cout<<"\tCode:\t\t"<<h.code.to_ulong()<<"\n";
        std::cout<<"\tLeast bits:\t"<<lsbs<<" bits\n";
        std::cout<<"\tSource Width:\t"<<h.width.to_ulong()<<" px\n";
        std::cout<<"\tSource Height:\t"<<h.height.to_ulong()<<" px\n";
        std::cout<<"\tExtension:\t"<<ext<<"\n";
        std::cout<<"\tData Size:\t"<<f_size<<" bits\n";
        std::cout<<"\tSeparator:\t"<<bs2b(h.separator)<<"\n";
        std::cout<<"---------------------------\n";

        //
        //  Write decrypted file
        //

        // make the filename
        std::string fname = "decrypted" + ext;

        // open binary mode
        std::ofstream ofs(fname.c_str(), std::ios::binary | std::ios::out);

        // abort if failed to open
        if (!ofs) {
            std::cout << "ERROR: Data file could not be written to." << std::endl;
            return;
        }

        // loop through each bit in data & write it to the output file stream
        std::bitset<8> f_byte;
        for (int i=0; i<f_data.size(); i++) {
            int bit = i % 8;
            f_byte[bit] = f_data[i];
            if (bit == 7)
                ofs.put(bs2b(f_byte));
        }

        // close the filestream
        ofs.flush();
        ofs.close();

        // exception if something went wrong
        if (ofs.bad()) {
            std::cout << "ERROR: Data file could not be written properly." << std::endl;
            return;
        } else {
            std::cout << "Decryption successful; saved as \"decrypted"<<ext<<"\"." << std::endl;
        }
    }
} // end of namespace
