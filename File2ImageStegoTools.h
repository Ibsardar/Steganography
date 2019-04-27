////////////////////////////////////////////////////////////////////////////////
//
//  Author:         Ibrahim Sardar
//  Class:          CSCI 557
//  Filename:       File2ImageStegoTools.h
//  Date:           04/20/2018
//  Description:    Header for Stenography: File-to-Image Tools namespace.
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

#ifndef _FILE2IMAGESTEGOTOOLS_H_
#define _FILE2IMAGESTEGOTOOLS_H_

#include <string>
#include <vector>
#include <bitset>
#include "CImg.h"

namespace f2i_stego_tools {

    // contains header info of an encrypted image
    struct Header;

    // string 2 int
    int s2i(std::string);

    // int 2 string
    std::string i2s (int);

    // char (byte) 2 unsigned int
    unsigned int b2ui(unsigned char);

    // unsigned int 2 char (byte)
    unsigned char ui2b(unsigned int);

    // bitset 2 unsigned int
    unsigned int bs2ui(std::bitset<8>&);

    // string 2 bitset
    std::bitset<8> s2bs(std::string);

    // unsigned int 2 bitset
    std::bitset<8> ui2bs(unsigned int i);

    // char (byte) 2 bitset
    std::bitset<8> b2bs(unsigned char);

    // returns a list bytes from an image
    std::vector<std::bitset<8> > get_data_from_img(cimg_library::CImg<unsigned char>&);

    // returns a list of bytes from a filename's extension
    std::vector<std::bitset<8> > get_extension_data(std::string);

    // constructs and returns a header structure from the input data
    Header create_header_data(cimg_library::CImg<unsigned char>&, std::vector<std::bitset<8> >&,
                              int, int, std::vector<std::bitset<8> >&);

    // encrypts an arbitrary file into a bitmap image
    void encrypt(std::string, std::string, int lsbs=1);

    // decrypts an arbitrary file from an encrypted bitmap image
    void decrypt(std::string);
}

#endif   // !defined _FILE2IMAGESTEGOTOOLS_H_
