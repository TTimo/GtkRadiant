
#if !defined(INCLUDED_GTKUTIL_IMAGE_H)
#define INCLUDED_GTKUTIL_IMAGE_H

void BitmapsPath_set(const char* path);

typedef struct _GtkImage GtkImage;
typedef struct _GdkPixbuf GdkPixbuf;

GdkPixbuf* pixbuf_new_from_file_with_mask(const char* filename);
GtkImage* image_new_from_file_with_mask(const char* filename);
GtkImage* image_new_missing();
GtkImage* new_image(const char* filename); // filename is full path to image file
GtkImage* new_local_image(const char* filename); // filename is relative to local bitmaps path

#endif
