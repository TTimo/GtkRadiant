/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.

This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qe3.h"
#include "io.h"

eclass_t	*eclass;
eclass_t	*eclass_bad;
char		eclass_directory[1024];

/*

the classname, color triple, and bounding box are parsed out of comments
A ? size means take the exact brush size.

/*QUAKED <classname> (0 0 0) ?
/*QUAKED <classname> (0 0 0) (-8 -8 -8) (8 8 8)

Flag names can follow the size description:

/*QUAKED func_door (0 .5 .8) ? START_OPEN STONE_SOUND DOOR_DONT_LINK GOLD_KEY SILVER_KEY

*/
char	*debugname;

eclass_t *Eclass_InitFromText (char *text)
{
	char	*t;
	int		len;
	int		r, i;
	char	parms[256], *p;
	eclass_t	*e;
	char	color[128];

	e = qmalloc(sizeof(*e));
	memset (e, 0, sizeof(*e));

	text += strlen("/*QUAKED ");

// grab the name
	text = COM_Parse (text);
	e->name = qmalloc (strlen(com_token)+1);
	strcpy (e->name, com_token);
	debugname = e->name;

// grab the color, reformat as texture name
	r = sscanf (text," (%f %f %f)", &e->color[0], &e->color[1], &e->color[2]);
	if (r != 3)
		return e;
	sprintf (color, "(%f %f %f)", e->color[0], e->color[1], e->color[2]);
	strcpy (e->texdef.name, color);

	while (*text != ')')
	{
		if (!*text)
			return e;
		text++;
	}
	text++;

// get the size
	text = COM_Parse (text);
	if (com_token[0] == '(')
	{	// parse the size as two vectors
		e->fixedsize = true;
		r = sscanf (text,"%f %f %f) (%f %f %f)", &e->mins[0], &e->mins[1], &e->mins[2],
			&e->maxs[0], &e->maxs[1], &e->maxs[2]);
		if (r != 6)
			return e;

		for (i=0 ; i<2 ; i++)
		{
			while (*text != ')')
			{
				if (!*text)
					return e;
				text++;
			}
			text++;
		}
	}
	else
	{	// use the brushes
	}

// get the flags


// copy to the first /n
	p = parms;
	while (*text && *text != '\n')
		*p++ = *text++;
	*p = 0;
	text++;

// any remaining words are parm flags
	p = parms;
	for (i=0 ; i<8 ; i++)
	{
		p = COM_Parse (p);
		if (!p)
			break;
		strcpy (e->flagnames[i], com_token);
	}

// find the length until close comment
	for (t=text ; t[0] && !(t[0]=='*' && t[1]=='/') ; t++)
	;

// copy the comment block out
	len = t-text;
	e->comments = qmalloc (len+1);
	memcpy (e->comments, text, len);
#if 0
	for (i=0 ; i<len ; i++)
		if (text[i] == '\n')
			e->comments[i] = '\r';
		else
			e->comments[i] = text[i];
#endif
	e->comments[len] = 0;

	return e;
}


/*
=================
Eclass_InsertAlphabetized
=================
*/
void Eclass_InsertAlphabetized (eclass_t *e)
{
	eclass_t	*s;

	if (!eclass)
	{
		eclass = e;
		return;
	}


	s = eclass;
	if (stricmp (e->name, s->name) < 0)
	{
		e->next = s;
		eclass = e;
		return;
	}

	do
	{
		if (!s->next || stricmp (e->name, s->next->name) < 0)
		{
			e->next = s->next;
			s->next = e;
			return;
		}
		s=s->next;
	} while (1);
}


/*
=================
Eclass_ScanFile
=================
*/
void Eclass_ScanFile (char *filename)
{
	int		size;
	char	*data;
	eclass_t	*e;
	int		i;
	char    temp[1024];

	QE_ConvertDOSToUnixName( temp, filename );

	Sys_Printf ("ScanFile: %s\n", temp);

	size = LoadFile (filename, (void *)&data);

	for (i=0 ; i<size ; i++)
		if (!strncmp(data+i, "/*QUAKED",8))
		{
			e = Eclass_InitFromText (data+i);
			if (e)
				Eclass_InsertAlphabetized (e);
			else
				printf ("Error parsing: %s in %s\n",debugname, filename);
		}

	free (data);
}



void Eclass_InitForSourceDirectory (char *path)
{
	struct _finddata_t fileinfo;
	int		handle;
	char	filename[1024];
	char	filebase[1024];
	char    temp[1024];
	char	*s;

	QE_ConvertDOSToUnixName( temp, path );

	Sys_Printf ("Eclass_InitForSourceDirectory: %s\n", temp );

	strcpy (filebase, path);
	s = filebase + strlen(filebase)-1;
	while (*s != '\\' && *s != '/' && s!=filebase)
		s--;
	*s = 0;

	eclass = NULL;

	handle = _findfirst (path, &fileinfo);
	if (handle != -1)
	{
		do
		{
			sprintf (filename, "%s\\%s", filebase, fileinfo.name);
			Eclass_ScanFile (filename);
		} while (_findnext( handle, &fileinfo ) != -1);

		_findclose (handle);
	}

	eclass_bad = Eclass_InitFromText ("/*QUAKED UNKNOWN_CLASS (0 0.5 0) ?");
}

eclass_t *Eclass_ForName (char *name, qboolean has_brushes)
{
	eclass_t	*e;
	char		init[1024];

	if (!name)
		return eclass_bad;

	for (e=eclass ; e ; e=e->next)
		if (!strcmp (name, e->name))
			return e;

	// create a new class for it
	if (has_brushes)
	{
		sprintf (init, "/*QUAKED %s (0 0.5 0) ?\nNot found in source.\n", name);
		e = Eclass_InitFromText (init);
	}
	else
	{
		sprintf (init, "/*QUAKED %s (0 0.5 0) (-8 -8 -8) (8 8 8)\nNot found in source.\n", name);
		e = Eclass_InitFromText (init);
	}

	Eclass_InsertAlphabetized (e);

	return e;
}

