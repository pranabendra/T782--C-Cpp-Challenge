# T782 C/C++ Challenge
My submission for Apertus T872 C/C++ Challenge for Google Summer of Code 2018


## For compilers supporting C++11 and above, run the following in the terminal:

1. `g++ main11.cpp lodepng.cpp -o raw12_to_png.out`
2. `./raw12_to_png.out <file_name> <bayer_pattern>`
    
    Eg. `./raw12_to_png.out portrait-gainx2-offset2047-20ms-02.raw12 rggb`


## For compilers <i>NOT</i> supporting C++11 and above, run the following in the terminal:

1. `g++ main.cpp lodepng.cpp -o raw12_to_png.out`
2. `./raw12_to_png.out <file_name> <bayer_pattern>`
    
    Eg. `./raw12_to_png.out portrait-gainx2-offset2047-20ms-02.raw12 rggb`


### Note:

`<bayer_pattern>` can <b>only</b> be any one of the following from the list 
{rggb, bggr, gbrg, grbg, RGGB, BGGR, GBRG, GRBG}
