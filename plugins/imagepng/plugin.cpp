/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "plugin.h"

#include "debugging/debugging.h"

#include "ifilesystem.h"
#include "iimage.h"

#include "imagelib.h"

// ====== PNG loader functionality ======

#include "png.h"
#include <stdlib.h>

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
  globalErrorStream() << "libpng warning: " << warning_msg << "\n";
}

void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
  globalErrorStream() << "libpng error: " << error_msg << "\n";
  longjmp(png_ptr->jmpbuf, 0);
}

void user_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
  png_bytep *p_p_fbuffer = (png_bytep*)png_get_io_ptr(png_ptr);
  memcpy(data, *p_p_fbuffer, length);
  *p_p_fbuffer += length;
}

Image* LoadPNGBuff (unsigned char* fbuffer)
{
  png_byte** row_pointers;
  png_bytep p_fbuffer;

  p_fbuffer = fbuffer;
	
  // the reading glue
  // http://www.libpng.org/pub/png/libpng-manual.html

  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
    user_error_fn, user_warning_fn);
  if (!png_ptr)
  {
    globalErrorStream() << "libpng error: png_create_read_struct\n";
    return 0;
  }
		
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr,
      (png_infopp)NULL, (png_infopp)NULL);
    globalErrorStream() << "libpng error: png_create_info_struct (info_ptr)\n";
    return 0;
  }
	
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr,
      (png_infopp)NULL);
    globalErrorStream() << "libpng error: png_create_info_struct (end_info)\n";
    return 0;
  }

  // configure the read function
  png_set_read_fn(png_ptr, (voidp)&p_fbuffer, (png_rw_ptr)&user_read_data);

  if (setjmp(png_ptr->jmpbuf)) {
    png_destroy_read_struct(&png_ptr, &info_ptr,
      &end_info);
    return 0;
  }

  png_read_info(png_ptr, info_ptr);

  int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  int color_type = png_get_color_type(png_ptr, info_ptr);

  // we want to treat all images the same way
  //   The following code transforms grayscale images of less than 8 to 8 bits, 
  //   changes paletted images to RGB, and adds a full alpha channel if there is 
  //   transparency information in a tRNS chunk.
  if (color_type == PNG_COLOR_TYPE_PALETTE)
   png_set_palette_to_rgb(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_gray_1_2_4_to_8(png_ptr);

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);

  if ( ! ( color_type & PNG_COLOR_MASK_ALPHA ) ) {
    // Set the background color to draw transparent and alpha images over.
    png_color_16 my_background, *image_background;

    if (png_get_bKGD(png_ptr, info_ptr, &image_background))
      png_set_background(png_ptr, image_background, 
      PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    else
      png_set_background(png_ptr, &my_background,
      PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

    // Add alpha byte after each RGB triplet
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
  }

  // read the sucker in one chunk
  png_read_update_info(png_ptr, info_ptr);

  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  int width = png_get_image_width(png_ptr, info_ptr);
  int height = png_get_image_height(png_ptr, info_ptr);

  // allocate the pixel buffer, and the row pointers
  RGBAImage* image = new RGBAImage(width, height);

  row_pointers = (png_byte**) malloc((height) * sizeof(png_byte*));

  int i;
  for(i = 0; i < (height); i++)
    row_pointers[i] = (png_byte*)(image->getRGBAPixels()) + i * 4 * (width);

  // actual read
  png_read_image(png_ptr, row_pointers);

  /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
  png_read_end(png_ptr, info_ptr);

  /* free up the memory structure */
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

  free(row_pointers);

  return image;
}

Image* LoadPNG(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadPNGBuff( buffer.buffer );
}


#include "modulesystem/singletonmodule.h"


class ImageDependencies : public GlobalFileSystemModuleRef
{
};

class ImagePNGAPI
{
  _QERPlugImageTable m_imagepng;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "png");

  ImagePNGAPI()
  {
    m_imagepng.loadImage = LoadPNG;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagepng;
  }
};

typedef SingletonModule<ImagePNGAPI, ImageDependencies> ImagePNGModule;

ImagePNGModule g_ImagePNGModule;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  initialiseModule(server);

  g_ImagePNGModule.selfRegister();
}
