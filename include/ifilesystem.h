/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
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

#ifndef _IFILESYSTEM_H_
#define _IFILESYSTEM_H_

//
// Plugin interface for the virtual filesystem used by Radiant
//

// NOTE: If you want to write a VFS plugin then you must export
// "QERPlug_ListInterfaces" and "QERPlug_RequestInterface"
// (see qerplugin.h for more information)

#ifdef _WIN32
#define VFS_NATIVESEPARATOR '\\'
#else
#define VFS_NATIVESEPARATOR '/'
#endif

#define VFS_MAJOR "VFS"

// return the file system supported by the plugin, for example: "quake1" or "quake3"
//typedef const char* (WINAPI* PFN_VFSGETFORMAT) ();
// add all files from a directory to the vfs
typedef void ( *PFN_VFSINITDIRECTORY )( const char *path );
// free all resources used by the plugin
typedef void ( *PFN_VFSSHUTDOWN )();
// free memory allocated by VFS for this pointer
typedef void ( *PFN_VFSFREEFILE )( void *p );
// return a GSList with all the directories under basedir
typedef GSList* ( *PFN_VFSGETDIRLIST )( const char *basedir );
// return a GSList with all the files under basedir (extension can be NULL)
typedef GSList* ( *PFN_VFSGETFILELIST )( const char *basedir, const char *extension );
// free a dirlist or filelist returned from one of the above functions
typedef void ( *PFN_VFSCLEARFILEDIRLIST )( GSList **lst );
#define VFS_SEARCH_PAK 0x1
#define VFS_SEARCH_DIR 0x2
/*!
   \brief return the number of files with the exact name described in filename
   there can be several hits for a given file, or this can be used to check for existence
   \param flags is optional and can be used with VFS_SEARCH_* bits, if flag is 0, everything is searched, else only the specified bits
   paks are searched first, then search directories
 */
typedef int ( *PFN_VFSGETFILECOUNT )( const char *filename, int flags );
/*!
   \brief load file, allocate buffer
   \return -1 if fails or the size of the buffer allocated
   \param index is used to load the i-th file in the search directories (see vfsGetFileCount)
   this will scan in the search directories first, then it will search in the pak files
   WARNING: the allocated buffer must be freed with a g_free call
   NOTE TTimo: the g_free release is utter horror
 */
typedef int ( *PFN_VFSLOADFILE )( const char *filename, void **buffer, int index );
// load a file from it's full path into the buffer, returns the file size or -1
// the allocated buffer must be freed with a g_free call
typedef int ( *PFN_VFSLOADFULLPATHFILE )( const char *filename, void **buffer );
// takes an absolute file path, returns a shortened relative file path if the absolute path matches a valid basedir or NULL if an error occured
typedef char* ( *PFN_VFSEXTRACTRELATIVEPATH )( const char *in );
/*!
   \return the full path (in a static buff) to a file given it's relative path (NULL if not found)
   \param index if several files are matching (as returned in a call to vfsGetFileCount), get the index-th file
   \param flag 0 or a combination of VFS_SEARCH_PAK or VFS_SEARCH_DIR
   HYDRA:
   this now searches VFS/PAK files in addition to the filesystem
   if FLAG is 0 then ONLY dirs are searched.
   PAK's are searched before DIRs to mimic engine behaviour
   index is ignored when searching PAK files.
   when searching VFS, files are searched case insensitive.

   WARNING: if you use index from vfsGetFileCount, it works only with a vfsGetFileCount for the search directories only (not the pak files)
   FIXME TTimo our VFS names are case insensitive.
   this function is not able to build the full path from case-insensitive name
 */
typedef char* ( *PFN_VFSGETFULLPATH )( const char *in, int index, int flag );
/*!
   these return a static char*, doesn't need to be freed or anything
   get the base path to use when raising file dialogs
   we manually add "maps/" or "sounds/" or "mapobjects/models/" etc.
   FIXME: I'm not sure this is used / relevant anymore
 */
typedef const char* ( *PFN_VFSBASEPROMPTPATH )();

// VFS API
struct _QERFileSystemTable
{
	int m_nSize;
	PFN_VFSINITDIRECTORY m_pfnInitDirectory;
	PFN_VFSSHUTDOWN m_pfnShutdown;
	PFN_VFSFREEFILE m_pfnFreeFile;
	PFN_VFSGETDIRLIST m_pfnGetDirList;
	PFN_VFSGETFILELIST m_pfnGetFileList;
	PFN_VFSCLEARFILEDIRLIST m_pfnClearFileDirList;
	PFN_VFSGETFILECOUNT m_pfnGetFileCount;
	PFN_VFSLOADFILE m_pfnLoadFile;
	PFN_VFSLOADFULLPATHFILE m_pfnLoadFullPathFile;
	PFN_VFSEXTRACTRELATIVEPATH m_pfnExtractRelativePath;
	PFN_VFSGETFULLPATH m_pfnGetFullPath;
	PFN_VFSBASEPROMPTPATH m_pfnBasePromptPath;
};

#ifdef USE_VFSTABLE_DEFINE
#ifndef __VFSTABLENAME
#define __VFSTABLENAME g_FileSystemTable
#endif
#define vfsInitDirectory __VFSTABLENAME.m_pfnInitDirectory
#define vfsShutdown __VFSTABLENAME.m_pfnShutdown
#define vfsFreeFile __VFSTABLENAME.m_pfnFreeFile
#define vfsGetDirList __VFSTABLENAME.m_pfnGetDirList
#define vfsGetFileList __VFSTABLENAME.m_pfnGetFileList
#define vfsClearFileDirList __VFSTABLENAME.m_pfnClearFileDirList
#define vfsGetFileCount __VFSTABLENAME.m_pfnGetFileCount
#define vfsLoadFile __VFSTABLENAME.m_pfnLoadFile
#define vfsLoadFullPathFile __VFSTABLENAME.m_pfnLoadFullPathFile
#define vfsExtractRelativePath __VFSTABLENAME.m_pfnExtractRelativePath
#define vfsGetFullPath __VFSTABLENAME.m_pfnGetFullPath
#define vfsBasePromptPath __VFSTABLENAME.m_pfnBasePromptPath
#endif

#endif // _IFILESYSTEM_H_
