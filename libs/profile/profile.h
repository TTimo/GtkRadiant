/*
Copyright (c) 2001, Loki software, inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Loki software nor the names of its contributors may be used 
to endorse or promote products derived from this software without specific prior 
written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY 
DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#if !defined(INCLUDED_PROFILE_PROFILE_H)
#define INCLUDED_PROFILE_PROFILE_H

// profile functions - kind of utility lib
// they are kind of dumb, they expect to get the path to the .ini file or to the prefs directory when called
// load_buffer and save_buffer expect the path only, theyll build a $(pszName).bin file
bool profile_save_int (const char *filename, const char *section, const char *key, int value);
bool profile_save_float (const char *filename, const char *section, const char *key, float value);
bool profile_save_string (const char *filename, const char *section, const char *key, const char *value);
bool profile_save_buffer (const char *rc_path, const char *pszName, void *pvBuf, unsigned int lSize);
bool profile_load_buffer (const char *rc_path, const char *pszName, void *pvBuf, unsigned int *plSize);
int profile_load_int (const char *filename, const char *section, const char *key, int default_value);
float profile_load_float (const char *filename, const char *section, const char *key, float default_value);
char* profile_load_string (const char *filename, const char *section, const char *key, const char *default_value);
// used in the command map code
bool read_var (const char *filename, const char *section, const char *key, char *value);
bool save_var (const char *filename, const char *section, const char *key, const char *value);

#endif
