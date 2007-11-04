#!/usr/bin/env python

import os.path, sys, shutil

def install_file( path, src_path, f ):
	src = os.path.join( src_path, f )
	dst = os.path.join( path, f )
	print '%s -> %s' % ( src, dst )
	shutil.copyfile( src, dst )

def install( path, src_path ):
	for f in [ 'radiant.exe', 'radiant.pdb' ]:
		install_file( path, src_path, f )
		
	modules_path = os.path.join( path, 'modules' )
	try:
		os.makedirs( modules_path )
	except:
		pass
	assert( os.path.exists( modules_path ) )

	modules_src = os.path.join( src_path, 'modules' )
	assert( os.path.exists( modules_src ) )

	for e in os.listdir( modules_src ):
		if ( e[-4:] == '.dll' or e[-4:] == '.pdb' ):
			install_file( modules_path, modules_src, e )
	
	plugins_path = os.path.join( path, 'plugins' )
	try:
		os.makedirs( plugins_path )
	except:
		pass
	assert( os.path.exists( plugins_path ) )
	
	plugins_src = os.path.join( src_path, 'plugins' )
	assert( os.path.exists( plugins_src ) )

	for e in os.listdir( plugins_src ):
		if ( e[-4:] == '.dll' or e[-4:] == '.pdb' ):
			install_file( plugins_path, plugins_src, e )

if __name__ == '__main__':
	if ( len( sys.argv ) <= 2 or not os.path.exists( sys.argv[1] ) or not os.path.exists( sys.argv[2] ) ):
		print 'usage: install [target directory] [source directory]'
		sys.exit(1)		
	print 'Install %s into %s' % ( sys.argv[2], sys.argv[1] )
	install( sys.argv[1], sys.argv[2] )
