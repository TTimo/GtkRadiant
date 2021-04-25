# -*- mode: python -*-
# GtkRadiant build scripts
# TTimo <ttimo@ttimo.net>
# http://scons.org/

from __future__ import print_function

import sys, os, platform, pickle

import utils, config

conf_filename = 'site.sconf'

try:
	sys.argv.index( '-h' )
except:
	pass
else:
	Help(
"""
======================================================================
GtkRadiant build system quick help

You need scons v0.97.0d20070918.r2446 or newer

Default build (release), just run scons at the toplevel

debug build:
$ scons config=debug

build using 8 parallel build jobs
but do not download any game packs
$ scons -j8 --no-packs
======================================================================
""" )
	Return()

AddOption('--no-packs',
	dest='no_packs',
	action='store_true',
	help="don't fetch game packs")

active_configs = []

# load up configurations from the save file
if ( os.path.exists( conf_filename ) ):
	f = open( conf_filename )
	print( 'reading saved configuration from site.conf' )
	try:
		while ( True ):
			c = pickle.load( f )
			active_configs.append( c )
	except:
		pass

# read the command line and build configs
config_statements = sys.argv[1:]
active_configs = config.ConfigParser().parseStatements( active_configs, config_statements )
assert( len( active_configs ) >= 1 )

# save the config
print( 'saving updated configuration' )
f = open( conf_filename, 'wb' )
for c in active_configs:
	pickle.dump( c, f, -1 )

print( 'emit build rules' )
for c in active_configs:
	print( 'emit configuration: %s' % repr( c ) )
	c.emit()
