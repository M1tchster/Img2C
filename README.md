# Img2C

A simple Windows program for converting small or basic images into C file(s) to embed in your projects.
The image data is RLE compressed and paletted, so it's best with simple or small images with repeat strips of color.

Usage: IMG2C <PATH_TO_GRAPHICS> <COLOR_KEY> (optional)

PATH_TO_GRAPHICS - the path to a folder containing the images you wish to embed into a .c file.
COLOR_KEY (default: FF00FF - format: BBGGRR) - hexadecimal color to be replaced with transparency.

IMG2C will:
1. Automatically scan for any GDIplus compatible images (.png, bmp, gif, and more) in the chosen path.
2. Create a palette which all of the images will share, with a maximum of 255 colors (to keep things light).
3. RLE compress the images (ImageData.h), to be compiled into a program, with functions to decode that data.

It will output a few files in the same directory from which it was run:

1. ImageData.h     - Contains the RLE compressed image data and that's about it.
2. ImageData.c     - Includes ImageData.h and a handy function which will decode the data
3. ImageDef.h      - Holds an enum for use with ImageData.c to reference the images in code
4. Palette.h       - Contains all of the color data to decode the images. Used by ImageData.c


If you use this program in your project:
Try to implement it so that when you edit a graphic all you need to do is run the program
(without needing to move or change any of the output files before compiling).

...Otherwise you can edit the source code for your specific needs
(or simply copy the RLE compressed ImageData.h and discard the rest).
