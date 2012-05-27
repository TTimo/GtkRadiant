
#if !defined(INCLUDED_GTKUTIL_GLFONT_H)
#define INCLUDED_GTKUTIL_GLFONT_H

typedef unsigned int GLuint;

class GLFont
{
  GLuint m_displayList;
  int m_pixelHeight;
public:
  GLFont(GLuint displayList, int pixelHeight) : m_displayList(displayList), m_pixelHeight(pixelHeight)
  {
  }
  GLuint getDisplayList() const
  {
    return m_displayList;
  }
  int getPixelHeight() const
  {
    return m_pixelHeight;
  }
};

GLFont glfont_create(const char* font_string);
void glfont_release(GLFont& font);

#endif
