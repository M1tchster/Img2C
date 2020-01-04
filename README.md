# Img2C

A simple Windows program for converting basic graphics into C file(s) to embed in software.
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
2. ImageSizes.h    - An array which contains the width/height of every image.
3. ImageNames.h    - Holds an enum to reference the images by name.
4. Palette.h       - Contains all of the color data to decode the images.

If you wish to use this in a project, check out the demo/template code in Res.c.


Troubleshooting:

  The color purple is invisible in my image!

      By default a color key is enabled in Img2C (255 Red 0 Green 255 Blue).
      You can specify '000000' to Img2C and delete the Color Key filter in Res.c.

  My images have holes in them while using a color key!

      Try including a dummy 1x1 image with the color key you use for translucency. Name it "_ColorKey".
      This can happen if the palette fills up very fast and the color key gets added after it fills.

  The software crashes when attempting to display my image!
  
      If your image is very very large, it's possible (depending on the compiler) that you may run out of stack.
      Try using another compiler, if it fixes it then try downscaling the image. Stack space may be the issue.
