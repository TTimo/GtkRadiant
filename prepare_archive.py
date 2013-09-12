import os, time, zipfile, functools, pprint, subprocess

if ( __name__ == '__main__' ):
    date_tag = time.strftime('%Y%m%d')
    folder_name = 'GtkRadiant-1.6.4-%s' % date_tag
    base_name = '%s.zip' % folder_name
    full_path = os.path.join( r'D:\\', base_name )

    def write_file( z, prefix_path, folder_name, root, fn ):
        fullpath = os.path.join( root, fn )
        arcname = fullpath.replace( prefix_path, folder_name )
        print( '%s -> %s' % ( fullpath, arcname ) )
        z.write( fullpath, arcname )

    z = zipfile.ZipFile( full_path, 'w', zipfile.ZIP_DEFLATED )
    prefix_path = r'D:\GtkRadiant\install'
    for root, dirs, files in os.walk( prefix_path, topdown = True ):
        if ( root.find( '.svn' ) >= 0 ):
            continue
        files = filter(
            lambda n : not (
                n.endswith( '.lib' )
                or n.endswith( '.pdb' )
                or n.endswith( '.exp' ) ),
            files )
        map( functools.partial( write_file, z, prefix_path, folder_name, root ), files )
    z.close()

    # could be nicer to import s3cmd
    subprocess.check_call( [ r'C:\Python27\python.exe', r'C:\Python27\Scripts\s3cmd', 'put', full_path, 's3://gtkradiant' ] )
