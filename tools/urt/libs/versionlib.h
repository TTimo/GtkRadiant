
#if !defined(INCLUDED_VERSIONLIB_H)
#define INCLUDED_VERSIONLIB_H

#include <cstddef>
#include <string.h>
#include <algorithm>

class Version
{
public:
  int major;
  int minor;
};

inline bool operator<(const Version& version, const Version& other)
{
  return version.major < other.major || (!(other.major < version.major) && version.minor < other.minor);
}

template<typename TextOutputStreamType>
TextOutputStreamType& ostream_write(TextOutputStreamType& outputStream, const Version& version)
{
  return outputStream << version.major << '.' << version.minor;
}

/// \brief Returns true if \p version (code) is compatible with \p other (data).
inline bool version_compatible(const Version& version, const Version& other)
{
  return version.major == other.major  // different major-versions are always incompatible
    && !(version.minor < other.minor); // data minor-version is incompatible if greater than code minor-version
}

inline int string_range_parse_int(const char* first, const char* last)
{
  const std::size_t bufferSize = 32;
  char buffer[bufferSize];
  strncpy(buffer, first, std::min(std::size_t(last - first), bufferSize - 1));
  buffer[bufferSize - 1] = '\0';
  return atoi(buffer);
}

inline Version version_parse(const char* versionString)
{
  Version version;
  const char* endVersion = versionString + strlen(versionString);

  const char* endMajor = strchr(versionString, '.');
  if(endMajor == 0)
  {
    endMajor = endVersion;

    version.minor = 0;
  }
  else
  {
    const char* endMinor = strchr(endMajor + 1, '.');
    if(endMinor == 0)
    {
      endMinor = endVersion;
    }
    version.minor = string_range_parse_int(endMajor + 1, endMinor);
  }
  version.major = string_range_parse_int(versionString, endMajor);

  return version;
}

#endif
