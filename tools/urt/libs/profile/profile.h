
#if !defined( INCLUDED_PROFILE_PROFILE_H )
#define INCLUDED_PROFILE_PROFILE_H

// profile functions - kind of utility lib
// they are kind of dumb, they expect to get the path to the .ini file or to the prefs directory when called
// load_buffer and save_buffer expect the path only, theyll build a $(pszName).bin file
bool profile_save_int( const char *filename, const char *section, const char *key, int value );
bool profile_save_float( const char *filename, const char *section, const char *key, float value );
bool profile_save_string( const char *filename, const char *section, const char *key, const char *value );
bool profile_save_buffer( const char *rc_path, const char *pszName, void *pvBuf, unsigned int lSize );
bool profile_load_buffer( const char *rc_path, const char *pszName, void *pvBuf, unsigned int *plSize );
int profile_load_int( const char *filename, const char *section, const char *key, int default_value );
float profile_load_float( const char *filename, const char *section, const char *key, float default_value );
char* profile_load_string( const char *filename, const char *section, const char *key, const char *default_value );
// used in the command map code
bool read_var( const char *filename, const char *section, const char *key, char *value );

#endif
