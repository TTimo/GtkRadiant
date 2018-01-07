# -*- mode: python -*-
# GtkRadiant build scripts
# TTimo <ttimo@ttimo.net>
# http://scons.org/

import os, commands, platform, xml.sax, re, string, platform

class vcxproj( xml.sax.handler.ContentHandler ):
	def __init__( self, filepath ):
		self.source_files = []
		self.misc_files = []
		self._files = []
		print 'parse %s' % filepath
		xml.sax.parse( filepath, self )

	def getSourceFiles( self ):
		return self.source_files

	def filterSource( self, expression, filelist = None ):
		if ( filelist is None ):
			filelist = self.source_files
		match = []
		nomatch = []
		for s in filelist:
			if ( re.match( expression, s ) ):
				match.append( s )
			else:
				nomatch.append( s )
		return ( match, nomatch )

	def startElement( self, name, attrs ):
		if ( name == 'ClCompile' ):
                        if ( attrs.has_key('Include') ):
			        self._files.append( attrs.getValue('Include') )

	def endDocument( self ):
		# split into source and headers, remap path seperator to the platform
		for f in self._files:
			if ( platform.system() != 'Windows' ):
				f = f.replace( '\\', '/' )
			if ( f[-2:] == '.c' or f[-4:] == '.cpp' ):
				self.source_files.append( f.encode('ascii') )
			else:
				self.misc_files.append( f )
		print '%d source files' % len( self.source_files )

# action uses LDD to verify that the source doesn't hold unresolved symbols
# setup as an AddPostAction of a regular SharedLibrary call
def CheckUnresolved( source, target, env ):
	# TODO: implement this for OSX
	if ( platform.system() == 'Darwin' ):
		return None
        # TODO: implement this for FreeBSD
        if ( platform.system() == 'FreeBSD' ):
                return None
	print 'CheckUnresolved %s' % target[0].abspath
	if ( not os.path.isfile( target[0].abspath ) ):
		print 'CheckUnresolved: %s does not exist' % target[0]
		return 1 # fail
	( status, output ) = commands.getstatusoutput( 'ldd -r %s' % target[0] )
	if ( status != 0 ):
		print 'CheckUnresolved: ldd command failed (exit code %d)' % status
		os.system( 'rm %s' % target[ 0 ] )
		return 1 # fail
	lines = string.split( output, '\n' )
	have_undef = 0
	for i_line in lines:
		regex = re.compile('undefined symbol: (.*)\t\\((.*)\\)')
		if ( regex.match( i_line ) ):
			symbol = regex.sub( '\\1', i_line )
			try:
				env['ALLOWED_SYMBOLS'].index( symbol )
			except:
				have_undef = 1
	if ( have_undef ):
		print output
		print "CheckUnresolved: undefined symbols"
		os.system('rm %s' % target[0])
		return 1

# http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/413486
# OH_GOD, I used to think code like that was cool and useful
def Enum(*names):
   ##assert names, "Empty enums are not supported" # <- Don't like empty enums? Uncomment!

   class EnumClass(object):
      __slots__ = names
      def __iter__(self):        return iter(constants)
      def __len__(self):         return len(constants)
      def __getitem__(self, i):  return constants[i]
      def __repr__(self):        return 'Enum' + str(names)
      def __str__(self):         return 'enum ' + str(constants)

   class EnumValue(object):
      __slots__ = ('__value')
      def __init__(self, value): self.__value = value
      Value = property(lambda self: self.__value)
      EnumType = property(lambda self: EnumType)
      def __hash__(self):        return hash(self.__value)
      def __cmp__(self, other):
         # C fans might want to remove the following assertion
         # to make all enums comparable by ordinal value {;))
         assert self.EnumType is other.EnumType, "Only values from the same enum are comparable"
         return cmp(self.__value, other.__value)
      def __invert__(self):      return constants[maximum - self.__value]
      def __nonzero__(self):     return bool(self.__value)
      def __repr__(self):        return str(names[self.__value])

   maximum = len(names) - 1
   constants = [None] * len(names)
   for i, each in enumerate(names):
      val = EnumValue(i)
      setattr(EnumClass, each, val)
      constants[i] = val
   constants = tuple(constants)
   EnumType = EnumClass()
   return EnumType

#if __name__ == '__main__':
#   print '\n*** Enum Demo ***'
#   print '--- Days of week ---'
#   Days = Enum('Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa', 'Su')
#   print Days
#   print Days.Mo
#   print Days.Fr
#   print Days.Mo < Days.Fr
#   print list(Days)
#   for each in Days:
#      print 'Day:', each
#   print '--- Yes/No ---'
#   Confirmation = Enum('No', 'Yes')
#   answer = Confirmation.No
#   print 'Your answer is not', ~answer

