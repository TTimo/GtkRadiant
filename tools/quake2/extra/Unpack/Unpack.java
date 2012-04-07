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

/*
 * Unpack -- a completely non-object oriented utility...
 *
 */

import java.io.*;

class Unpack {
	static final int IDPAKHEADER	= (('K'<<24)+('C'<<16)+('A'<<8)+'P');

	static int intSwap(int i) {
		int		a, b, c, d;

		a = i & 255;
		b = (i >> 8) & 255;
		c = (i >> 16) & 255;
		d = (i >> 24) & 255;

		return (a << 24) + (b << 16) + (c << 8) + d;
	}

	static boolean	patternMatch (String pattern, String s) {
		int		index;
		int		remaining;

		if (pattern.equals(s)) {
			return true;
		}

		// fairly lame single wildcard matching
		index = pattern.indexOf('*');
		if (index == -1) {
			return false;
		}
		if (!pattern.regionMatches(0, s, 0, index)) {
			return false;
		}

		index += 1;	// skip the *
		remaining = pattern.length() - index;
		if (s.length() < remaining) {
			return false;
		}

		if (!pattern.regionMatches(index, s, s.length()-remaining, remaining)) {
			return false;
		}

		return true;
	}

	static void usage() {
		System.out.println ("Usage: unpack <packfile> <match> <basedir>");
		System.out.println ("   or: unpack -list <packfile>");
		System.out.println ("<match> may contain a single * wildcard");
		System.exit (1);
	}

	public static void main (String[] args) {
		int			ident;
		int			dirofs;
		int			dirlen;
		int			i;
		int			numLumps;
		byte[]		name = new byte[56];
		String		nameString;
		int			filepos;
		int			filelen;
		RandomAccessFile	readLump;
		DataInputStream		directory;
		String		pakName;
		String		pattern;

		if (args.length == 2) {
			if (!args[0].equals("-list")) {
				usage();
			}
			pakName = args[1];
			pattern = null;
		} else if (args.length == 3) {
			pakName = args[0];
			pattern = args[1];
		} else {
			pakName = null;
			pattern = null;
			usage ();
		}

		try	{
			// one stream to read the directory
			directory = new DataInputStream(new FileInputStream(pakName));

			// another to read lumps
			readLump = new RandomAccessFile(pakName, "r");

			// read the header
			ident = intSwap(directory.readInt());
			dirofs = intSwap(directory.readInt());
			dirlen = intSwap(directory.readInt());

			if (ident != IDPAKHEADER) {
				System.out.println ( pakName + " is not a pakfile.");
				System.exit (1);
			}

			// read the directory
			directory.skipBytes (dirofs - 12);
			numLumps = dirlen / 64;

			System.out.println (numLumps + " lumps in " + pakName);

			for (i = 0 ; i < numLumps ; i++) {
				directory.readFully(name);
				filepos = intSwap(directory.readInt());
				filelen = intSwap(directory.readInt());

				nameString = new String (name, 0);
				// chop to the first 0 byte
				nameString = nameString.substring (0, nameString.indexOf(0));

				if (pattern == null) {
					// listing mode
					System.out.println (nameString + " : " + filelen + "bytes");
				} else if (patternMatch (pattern, nameString) ) {
					File				writeFile;
					DataOutputStream	writeLump;
					byte[]				buffer = new byte[filelen];
					StringBuffer		fixedString;
					String				finalName;
					int					index;

					System.out.println ("Unpaking " + nameString + " " + filelen
						+ " bytes");

					// load the lump
					readLump.seek(filepos);
					readLump.readFully(buffer);

					// quake uses forward slashes, but java requires
					// they only by the host's seperator, which
					// varies from win to unix
					fixedString = new StringBuffer (args[2] + File.separator + nameString);
					for (index = 0 ; index < fixedString.length() ; index++) {
						if (fixedString.charAt(index) == '/') {
							fixedString.setCharAt(index, File.separatorChar);
						}
					}
					finalName = fixedString.toString ();

					index = finalName.lastIndexOf(File.separatorChar);
					if (index != -1) {
						String		finalPath;
						File		writePath;

						finalPath = finalName.substring(0, index);
						writePath = new File (finalPath);
						writePath.mkdirs();
					}

					writeFile = new File (finalName);
					writeLump = new DataOutputStream ( new FileOutputStream(writeFile) );
					writeLump.write(buffer);
					writeLump.close();

				}
			}

			readLump.close();
			directory.close();

		} catch (IOException e) {
			System.out.println ( e.toString() );
		}
	}

}
