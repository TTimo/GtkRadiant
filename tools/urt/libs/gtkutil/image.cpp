
#include "image.h"

#include <gtk/gtkimage.h>
#include <gtk/gtkstock.h>

#include "string/string.h"
#include "stream/stringstream.h"
#include "stream/textstream.h"


namespace
{
  CopiedString g_bitmapsPath;
}

void BitmapsPath_set(const char* path)
{
  g_bitmapsPath = path;
}

GdkPixbuf* pixbuf_new_from_file_with_mask(const char* filename)
{
  GdkPixbuf* rgb = gdk_pixbuf_new_from_file(filename, 0);
  if(rgb == 0)
  {
    return 0;
  }
  else
  {
    GdkPixbuf* rgba = gdk_pixbuf_add_alpha(rgb, TRUE, 255, 0, 255);
    gdk_pixbuf_unref(rgb);
    return rgba;
  }
}

GtkImage* image_new_from_file_with_mask(const char* filename)
{
  GdkPixbuf* rgba = pixbuf_new_from_file_with_mask(filename);
  if(rgba == 0)
  {
    return 0;
  }
  else
  {
    GtkImage* image = GTK_IMAGE(gtk_image_new_from_pixbuf(rgba));
    gdk_pixbuf_unref(rgba);
    return image;
  }
}

GtkImage* image_new_missing()
{
  return GTK_IMAGE(gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_SMALL_TOOLBAR));
} 

GtkImage* new_image(const char* filename)
{
  {
    GtkImage* image = image_new_from_file_with_mask(filename);
    if(image != 0)
    {
      return image;
    }
  }
  
  return image_new_missing();
} 

GtkImage* new_local_image(const char* filename)
{
  StringOutputStream fullPath(256);
  fullPath << g_bitmapsPath.c_str() << filename;
  return new_image(fullPath.c_str());
}

