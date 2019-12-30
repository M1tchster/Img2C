# Img2C

A simple Windows program for converting small or basic images into C file(s) to embed in your projects.
The image data is RLE compressed and paletted, so it's best with simple or small images with repeat strips of color.

Usage: IMG2C <PATH_TO_GRAPHICS>

IMG2C will scan for any GDIplus compatible images (.png, bmp, gif and more), create a global palette for all of the images to share, with a maximum of 255 colors (why would you want more if you're embedding a simple image in a program anyways?)

It will then output a few files in the same directory from which it is ran:

ImageData.h     - Contains the RLE compressed image data and that's about it.
ImageData.c     - Includes ImageData.h and a handy function which will decode the data
ImageDef.h      - Holds an enum for use with ImageData.c to reference the images in code
Palette.h       - Contains all of the color data to decode the images. Used by ImageData.c

If you use this program in your project, it may be ideal to position it so that when you edit a graphic all you need to do is run the program (without needing to move or change the output files). However, you can also edit the source code for your needs or simply copy the RLE compressed ImageData.h and discard the rest.
