
#if !defined(INCLUDED_OS_DIR_H)
#define INCLUDED_OS_DIR_H

/// \file
/// \brief OS directory-listing object.

#include <glib/gdir.h>

typedef GDir Directory;

inline bool directory_good(Directory* directory)
{
  return directory != 0;
}

inline Directory* directory_open(const char* name)
{
  return g_dir_open(name, 0, 0);
}

inline void directory_close(Directory* directory)
{
  g_dir_close(directory);
}

inline const char* directory_read_and_increment(Directory* directory)
{
  return g_dir_read_name(directory);
}

template<typename Functor>
void Directory_forEach(const char* path, const Functor& functor)
{
  Directory* dir = directory_open(path);

  if(directory_good(dir))
  {
    for(;;)
    {
      const char* name = directory_read_and_increment(dir);
      if(name == 0)
      {
        break;
      }

      functor(name);
    }

    directory_close(dir);
  }
}


#endif
