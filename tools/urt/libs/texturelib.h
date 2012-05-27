
#if !defined (INCLUDED_TEXTURELIB_H)
#define INCLUDED_TEXTURELIB_H

#include "math/vector.h"
typedef Vector3 Colour3;
typedef unsigned int GLuint;

// describes a GL texture
struct qtexture_t
{
  // name of the texture file (the physical image file we are using)
  // NOTE: used for lookup, must be unique .. vfs path of the texture, lowercase, NO FILE EXTENSION
  // ex textures/gothic_wall/iron
  std::size_t width, height;
  GLuint texture_number; // gl bind number
  Colour3 color; // for flat shade mode
  int surfaceFlags, contentFlags, value;
};

#endif
