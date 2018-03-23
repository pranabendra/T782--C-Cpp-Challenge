//
//  main.cpp
//  program
//
//  Created by PRANAB on 3/9/18.
//  Copyright © 2018 PRANAB. All rights reserved.
//

#include <iostream>
#include <math.h>
#include <stdint.h>
#include <fstream>
#include <string.h>
#include "lodepng.h"
using namespace std;

int main(int argc, char *argv[])
{
    char* raw_file_name;
    char* pattern;
    uint32_t width = 0;
    
    long long int i = 0, j = 0, k = 0;
    
    if (argc == 1)
    {
        cout<<"Filename and Bayer pattern missing"<<endl;
        return -1;
    }
    else if (argc == 2)
    {
        cout<<"Bayer Pattern missing"<<endl;
        return -1;
    }
    else if (argc == 3)
    {
        // Checking for the correct argument
        raw_file_name = argv[1];
        pattern = argv[2];
    }
    else
    {
        cout<<"Too many arguments"<<endl;
        return -1;
    }
    
    ifstream original_raw_file (raw_file_name, ifstream::binary | ifstream::in);
    
    // Checking the existence of file
    if (original_raw_file)
    {
        original_raw_file.seekg(0, original_raw_file.end);
        width = original_raw_file.tellg();      // Getting the width of vector to be created
        original_raw_file.seekg(0, original_raw_file.beg);
        
        /*
        cout<<"Width of image = "<<width<<endl;
        cout<<"Pattern = "<<pattern<<endl;
        cout<<"Size = "<<strlen(pattern)<<endl;
        */
        
        // Allocating memory for backup file
        uint8_t* duplicate_raw_file = new uint8_t[width];
        
        // Copying the original file contents to backup file
        original_raw_file.read(reinterpret_cast<char*>(duplicate_raw_file), width);
        
        // Closing the file
        original_raw_file.close();
        
        // Allocating memory for storing 8-bit data
        uint8_t* data_8bit = new uint8_t [3*width/2];
        
        // Converting 12-bit data to 8-bit data
        j = 0;
        for(i = 0; i < width; i += 3)
        {
            data_8bit[j] = duplicate_raw_file[i];
            data_8bit[j+1] = ((duplicate_raw_file[i+1] & 0x0F) << 4) | (duplicate_raw_file[i+2] >> 4);
            j += 2;
        }
        
        // Deleting the backup file
        delete[] duplicate_raw_file;

        // Allocating memory to extract individual colour channels
        uint8_t* r_channel = new uint8_t [2048*1536];       //red channel
        uint8_t* g1_channel = new uint8_t [2048*1536];      //green1 channel
        uint8_t* g2_channel = new uint8_t [2048*1536];      //green2 channel
        uint8_t* b_channel = new uint8_t [2048*1536];       //blue channel
        
        // Allocation of memory for 1D Array which maps the RGB colour space
        uint8_t* final = new uint8_t [4096*3072*3];
        
        // Matching the pattern entered with Bayer patterns
        if ((strncmp(argv[2], "BGGR\0", 4) == 0) || (strncmp(argv[2], "bggr\0", 4) == 0)) //BGGR
        {
            i = 0;
            j = 0;
            
            // Extracting individual colour channels
            for(k = 0; k < 4096*3072; k += 2)
            {
                if (k % 8192 < 4096)
                {
                    b_channel[j] = data_8bit[k];
                    g1_channel[j] = data_8bit[k + 1];
                    j++;
                }
                else
                {
                    g2_channel[i] = data_8bit[k];
                    r_channel[i] = data_8bit[k + 1];
                    i++;
                }
            }
            
            // Bilinear Interpolation

            for (long long k = 0; k < 4096*3072; k += 2)
            {
                if (k < 8192 || k > 4096*3070 || (k % 4096) < 2 || (k % 4096) > 4094)
                {
                    // Avoids accessing the 2-pixel thickness along the boundary of the image
                    if ((k % 8192) < 4096)
                    {
                        // Blue Channel
                        final[k*3 + 2] = data_8bit[k];
                        final[k*3 + 1] = data_8bit[k + 1];
                        final[k*3] = data_8bit[k + 4097];
                        
                        // Green1 Channel
                        final[k*3+5] = data_8bit[k];
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+3] = data_8bit[k + 4097];
                    }
                    else
                    {
                        // Green2 Channel
                        final[k*3+2] = data_8bit[k - 4096];
                        final[k*3+1] = data_8bit[k];
                        final[k*3] = data_8bit[k + 1];
                        
                        // Red Pixel.
                        final[k*3+5] = data_8bit[k - 4096];
                        final[k*3+4] = data_8bit[k];
                        final[k*3+3] = data_8bit[k + 1];
                    }
                }
                else
                {
                    if ((k % 8192) < 4096)
                    {
                        // Blue Channel
                        final[k*3 + 2] = data_8bit[k];
                        final[k*3+1] = (data_8bit[k + 1] + data_8bit[k - 1] + data_8bit[k + 8193] + data_8bit[k - 8191]) >> 2;
                        final[k*3] = (data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4099] + data_8bit[k + 4095]) >> 2;
                        
                        // Green1 Channel
                        final[k*3+5] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 8192] + data_8bit[k - 8192]) >> 2;
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+3] = (data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4099] + data_8bit[k + 4095]) >> 2;
                    }
                    else
                    {
                        // Green2 Channel
                        final[k*3+2] = (data_8bit[k - 4096] + data_8bit[k + 4096] + data_8bit[k - 4094] + data_8bit[k - 4098]) >> 2;
                        final[k*3+1] = data_8bit[k];
                        final[k*3] = (data_8bit[k + 1] + data_8bit[k - 1] + data_8bit[k + 8193] + data_8bit[k - 8191]) >> 2;
                        
                        // Red Channel
                        final[k*3+5] = (data_8bit[k - 4096] + data_8bit[k + 4096] + data_8bit[k - 4094] + data_8bit[k - 4098]) >> 2;
                        final[k*3+4] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 8192] + data_8bit[k - 8192]) >> 2;
                        final[k*3+3] = data_8bit[k + 1];
                    }
                }
            }
            
            // Output to PNG file(s) : Final RGB image and four individual channel grayscale images
            cout << lodepng::encode("bggr_RGB.png", final, 4096,  3072, LCT_RGB) << endl;
            cout << lodepng::encode("bggr_B_channel.png", b_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("bggr_G1_channel.png", g1_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("bggr_G2_channel.png", g2_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("bggr_R_channel.png", r_channel, 2048, 1536, LCT_GREY) <<endl;
            
        }
        else if ((strncmp(argv[2], "RGGB\0", 4) == 0) || (strncmp(argv[2], "rggb\0", 4) == 0)) //RGGB
        {
            i = 0;
            j = 0;
            
            // Extracting individual colour channels
            for(k = 0; k < 4096*3072; k += 2)
            {
                if (k % 8192 < 4096)
                {
                    r_channel[j] = data_8bit[k];
                    g1_channel[j] = data_8bit[k + 1];
                    j++;
                }
                else
                {
                    g2_channel[i] = data_8bit[k];
                    b_channel[i] = data_8bit[k + 1];
                    i++;
                }
            }
            
            // Bilinear Interpolation
            
            for (long long k = 0; k < 4096*3072; k += 2)
            {
                if (k < 8192 || k > 4096*3070 || (k % 4096) < 2 || (k % 4096) > 4094)
                {
                    // Avoids accessing the 2-pixel thickness along the boundary of the image
                    if ((k % 8192) < 4096)
                    {
                        // Red Channel
                        final[k*3] = data_8bit[k];
                        final[k*3+1] = data_8bit[k + 1];
                        final[k*3+2] = data_8bit[k + 4097];
                        
                        // Green1 Channel
                        final[k*3+3] = data_8bit[k];
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+5] = data_8bit[k + 4097];
                    }
                    else
                    {
                        // Green2 Channel
                        final[k*3] = data_8bit[k - 4096];
                        final[k*3+1] = data_8bit[k];
                        final[k*3+2] = data_8bit[k + 1];
                        
                        // Blue Pixel.
                        final[k*3+3] = data_8bit[k - 4096];
                        final[k*3+4] = data_8bit[k];
                        final[k*3+5] = data_8bit[k + 1];
                    }
                }
                else
                {
                    if ((k % 8192) < 4096)
                    {
                        // Red Channel
                        final[k*3] = data_8bit[k];
                        final[k*3+1] = (data_8bit[k + 1] + data_8bit[k - 1] + data_8bit[k + 8193] + data_8bit[k - 8191]) >> 2;
                        final[k*3+2] = (data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4099] + data_8bit[k + 4095]) >> 2;
                        
                        // Green1 Channel
                        final[k*3+3] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 8192] + data_8bit[k - 8192]) >> 2;
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+5] = (data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4099] + data_8bit[k + 4095]) >> 2;
                    }
                    else
                    {
                        // Green2 Channel
                        final[k*3] = (data_8bit[k - 4096] + data_8bit[k + 4096] + data_8bit[k - 4094] + data_8bit[k - 4098]) >> 2;
                        final[k*3+1] = data_8bit[k];
                        final[k*3+2] = (data_8bit[k + 1] + data_8bit[k - 1] + data_8bit[k + 8193] + data_8bit[k - 8191]) >> 2;
                        
                        // Blue Channel
                        final[k*3+3] = (data_8bit[k - 4096] + data_8bit[k + 4096] + data_8bit[k - 4094] + data_8bit[k - 4098]) >> 2;
                        final[k*3+4] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 8192] + data_8bit[k - 8192]) >> 2;
                        final[k*3+5] = data_8bit[k + 1];
                    }
                }
            }
            
            // Output to PNG file(s) : Final RGB image and four individual channel grayscale images
            cout << lodepng::encode("rggb_RGB.png", final, 4096,  3072, LCT_RGB) << endl;
            cout << lodepng::encode("rggb_R_channel.png", r_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("rggb_G1_channel.png", g1_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("rggb_G2_channel.png", g2_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("rggb_B_channel.png", b_channel, 2048, 1536, LCT_GREY) <<endl;
            
        }
        else if ((strncmp(argv[2], "GRBG\0", 4) == 0) || (strncmp(argv[2], "grbg\0", 4) == 0)) //GRBG
        {
            i = 0;
            j = 0;
            
            //Extracting individual colour channels
            for(k = 0; k < 4096*3072; k += 2)
            {
                if (k % 8192 < 4096)
                {
                    g1_channel[j] = data_8bit[k];
                    r_channel[j] = data_8bit[k + 1];
                    j++;
                }
                else
                {
                    b_channel[i] = data_8bit[k];
                    g2_channel[i] = data_8bit[k + 1];
                    i++;
                }
            }
            
            // Bilinear Interpolation

            for (long long k = 0; k < 4096*3072; k += 2)
            {
                if (k < 8192 || k > 4096*3070 || (k % 4096) < 2 || (k % 4096) > 4094)
                {
                    // Avoids accessing the 2-pixel thickness along the boundary of the image
                    if ((k % 8192) < 4096)
                    {
                        // Green1 Channel
                        final[k*3+1] = data_8bit[k];
                        final[k*3] = data_8bit[k + 1];
                        final[k*3+2] = data_8bit[k + 4096];
                        
                        // Red Channel
                        final[k*3+3] = data_8bit[k];
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+5] = data_8bit[k + 4095];
                    }
                    else
                    {
                        // Blue Channel
                        final[k*3+2] = data_8bit[k];
                        final[k*3+1] = data_8bit[k + 1];
                        final[k*3] = data_8bit[k - 4095];
                        
                        // Green2 Pixel.
                        final[k*3+3] = data_8bit[k - 4096];
                        final[k*3+4] = data_8bit[k];
                        final[k*3+5] = data_8bit[k - 1];
                    }
                }
                else
                {
                    if ((k % 8192) < 4096)
                    {
                        // Green1 Channel
                        final[k*3+1] = data_8bit[k];
                        final[k*3] = (data_8bit[k + 1] + data_8bit[k - 1] + data_8bit[k + 8193] + data_8bit[k - 8191] + data_8bit[k + 8191] + data_8bit[k - 8193]) / 6;
                        final[k*3+2] = (data_8bit[k + 4096] + data_8bit[k - 4096] + data_8bit[k + 4094] + data_8bit[k - 4094] + data_8bit[k + 4098] + data_8bit[k - 4098]) / 6;
                        
                        // Red Channel
                        final[k*3+3] = data_8bit[k + 1];
                        final[k*3+4] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 4097] + data_8bit[k - 4095]) >> 2;
                        final[k*3+5] = (data_8bit[k + 4096] + data_8bit[k - 4096] + data_8bit[k + 4098] + data_8bit[k - 4094]) >> 2;
                    }
                    else
                    {
                        // Blue Channel
                        final[k*3] = (data_8bit[k - 4097] + data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4095]) >> 2;
                        final[k*3+1] = (data_8bit[k - 4096] + data_8bit[k + 4096] + data_8bit[k - 1] + data_8bit[k + 1]) >> 2;
                        final[k*3+2] = data_8bit[k];
                        
                        // Green2 Channel
                        final[k*3+3] = (data_8bit[k - 4097] + data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4095]) >> 2;
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+5] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 8192] + data_8bit[k - 8192] + data_8bit[k - 8190] + data_8bit[k + 8194]) / 6;
                    }
                }
            }
            
            // Output to PNG file(s) : Final RGB image and four individual channel grayscale images
            cout << lodepng::encode("grbg_RGB.png", final, 4096,  3072, LCT_RGB) << endl;
            cout << lodepng::encode("grbg_G1_channel.png", g1_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("grbg_R_channel.png", r_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("grbg_B_channel.png", b_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("grbg_G2_channel.png", g2_channel, 2048, 1536, LCT_GREY) <<endl;
            
        }
        else if ((strncmp(argv[2], "GBRG\0", 4) == 0) || (strncmp(argv[2], "gbrg\0", 4) == 0)) //GBRG
        {
            i = 0;
            j = 0;
            
            // Extracting individual colour channels
            for(k = 0; k < 4096*3072; k += 2)
            {
                if (k % 8192 < 4096)
                {
                    g1_channel[j] = data_8bit[k];
                    b_channel[j] = data_8bit[k + 1];
                    j++;
                }
                else
                {
                    r_channel[i] = data_8bit[k];
                    g2_channel[i] = data_8bit[k + 1];
                    i++;
                }
            }
            
            // Bilinear Interpolation

            for (long long k = 0; k < 4096*3072; k += 2)
            {
                if (k < 8192 || k > 4096*3070 || (k % 4096) < 2 || (k % 4096) > 4094)
                {
                    // Avoids accessing the 2-pixel thickness along the boundary of the image
                    if ((k % 8192) < 4096)
                    {
                        // Green1 Channel
                        final[k*3+1] = data_8bit[k];
                        final[k*3] = data_8bit[k + 1];
                        final[k*3+2] = data_8bit[k + 4096];
                        
                        // Blue Channel
                        final[k*3+5] = data_8bit[k];
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+3] = data_8bit[k + 4095];
                    }
                    else
                    {
                        // Red Channel
                        final[k*3] = data_8bit[k];
                        final[k*3+1] = data_8bit[k + 1];
                        final[k*3+2] = data_8bit[k - 4095];
                        
                        // Green2 Pixel.
                        final[k*3+3] = data_8bit[k - 4096];
                        final[k*3+4] = data_8bit[k];
                        final[k*3+5] = data_8bit[k - 1];
                    }
                }
                else
                {
                    if ((k % 8192) < 4096)
                    {
                        // Green1 Channel
                        final[k*3+1] = data_8bit[k];
                        final[k*3+2] = (data_8bit[k + 1] + data_8bit[k - 1] + data_8bit[k + 8193] + data_8bit[k - 8191] + data_8bit[k + 8191] + data_8bit[k - 8193]) / 6;
                        final[k*3] = (data_8bit[k + 4096] + data_8bit[k - 4096] + data_8bit[k + 4094] + data_8bit[k - 4094] + data_8bit[k + 4098] + data_8bit[k - 4098]) / 6;
                        
                        // Blue Channel
                        final[k*3+5] = data_8bit[k + 1];
                        final[k*3+4] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 4097] + data_8bit[k - 4095]) >> 2;
                        final[k*3+3] = (data_8bit[k + 4096] + data_8bit[k - 4096] + data_8bit[k + 4098] + data_8bit[k - 4094]) >> 2;
                    }
                    else
                    {
                        // Red Channel
                        final[k*3+2] = (data_8bit[k - 4097] + data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4095]) >> 2;
                        final[k*3+1] = (data_8bit[k - 4096] + data_8bit[k + 4096] + data_8bit[k - 1] + data_8bit[k + 1]) >> 2;
                        final[k*3] = data_8bit[k];
                        
                        // Green2 Channel
                        final[k*3+5] = (data_8bit[k - 4097] + data_8bit[k + 4097] + data_8bit[k - 4095] + data_8bit[k + 4095]) >> 2;
                        final[k*3+4] = data_8bit[k + 1];
                        final[k*3+3] = (data_8bit[k] + data_8bit[k + 2] + data_8bit[k + 8192] + data_8bit[k - 8192] + data_8bit[k - 8190] + data_8bit[k + 8194]) / 6;
                    }
                }
            }
            
            // Output to PNG file(s) : Final RGB image and four individual channel grayscale images
            cout << lodepng::encode("gbrg_RGB.png", final, 4096,  3072, LCT_RGB) << endl;
            cout << lodepng::encode("gbrg_G1_channel.png", g1_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("gbrg_B_channel.png", b_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("gbrg_R_channel.png", r_channel, 2048, 1536, LCT_GREY) <<endl;
            cout << lodepng::encode("gbrg_G2_channel.png", g2_channel, 2048, 1536, LCT_GREY) <<endl;
            
        }
        else
        {
            // Bayer Pattern not found
            cout<<"Pattern not valid"<<endl;
        }
        
    }
    else
    {
        //RAW12 file was not found
        cout<<"RAW12 file not found"<<endl;
    }
    return 0;
}

/*
References :-
1. https://github.com/TofuLynx
2. http://www.dmi.unict.it/~battiato/mm1112/Parte%207.3%20%20-%20Demosaicing.pdf
*/
