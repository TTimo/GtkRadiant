#!/usr/bin/env perl
# see ./setup.pl for usage info

# the templated source
$TEMPLATE_DIR='template';
# the 'meta' directory (no longer a template, not a final setup dir yet)
$WORK_DIR='WorkDir';

# Source common perl functions
BEGIN {
  push @INC, "../common";
}
use setup;
$common = new setup();

# replace in file
sub replace {
  # only wrap setup::replace() for now
  $common->replace(@_);
}

sub svn_cleanup {
	$CLEAN = shift(@_);
  # take out .svn/ directories
  print "SVN file cleanup in $CLEAN...\n";
  my $findscan;
  open($findscan, "find $CLEAN -name .svn | ");
  my $line = <$findscan>;
  do
  {
    chop($line);
    system("rm -rf '$line'");
  } until (!($line = <$findscan>));
  close($findscan);
}

# make a work version from the template
# for IS developer use, and for custom setups generation
sub build_work_tree()
{
  print "Building a work tree in $WORK_DIR/ from $TEMPLATE_DIR/\n";
  
  # make a clean copy
  print "File copy...\n";
  system("rm -rf $WORK_DIR");
  system("cp -R $TEMPLATE_DIR $WORK_DIR");
  
  svn_cleanup( "$WORK_DIR" );

  my $ESCAPED_GTKRAD_DIR = $GTKRAD_DIR;
  $ESCAPED_GTKRAD_DIR =~ s/\\/\\\\/g;
  
  # proceed to replace <<GTKRAD_DIR>> in .fgl files
  print "search/replace for base dir...\n";
  open($findscan, "find $WORK_DIR -name '*.fgl' | ");
  my $line = <$findscan>;
  do
  {
    chop($line);
    replace($line, '<<GTKRAD_DIR>>', $ESCAPED_GTKRAD_DIR);
  } until (!($line = <$findscan>));
  close($findscan);  
}

# finalize from a work tree version
# this sets some common settings, but doesn't set customizable stuff (like game packs selection)
# as we customize more things, this should shrink
sub finalize_tree
{
  my $DIR = shift(@_);
  # what is the version?
  # NOTE: I've had countless shit getting this extracted correctly
  my $GTKRAD_VERSION=`cat ../../include/version.default | tr -d \\\\n\\\\r\\\\f`;
  
  # what is the name of the binary?
  my $GTKRAD_BIN="GtkRadiant-$GTKRAD_VERSION.exe";

  print "  Finalizing $DIR/...\n";

  # copy the Radiant binary to a generated file with the right name
  #print "the thing: cp ../../radiant/Release/GtkRadiant.exe ./bin/$GTKRAD_BIN\n";
  system("cp ../../radiant/Release/GtkRadiant.exe ./bin/$GTKRAD_BIN");
  
  # copy the splash screen - ET version
  system("cp ../setup.bmp '$DIR/Setup Files/Uncompressed Files/Language Independent/OS Independent'");
   
  # PRODUCT_VERSION
  print "  Product version $GTKRAD_VERSION...\n";
  replace("$DIR/String Tables/0009-English/value.shl", '<<GTKRAD_VERSION>>', "$GTKRAD_VERSION ");

  # set the binary name
  print "  Binary name $GTKRAD_BIN...\n";
  replace("$DIR/File Groups/Program Executable Files.fgl", '<<GTKRAD_BIN>>', "$GTKRAD_BIN");
  replace("$DIR/String Tables/0009-English/value.shl", '<<GTKRAD_BIN>>', "$GTKRAD_BIN");
  replace("$DIR/Shell Objects/Default.shl", '<<GTKRAD_BIN>>', "$GTKRAD_BIN");

  # install path prompt  
  replace("$SETUP_DIR/String Tables/0009-English/value.shl", '<<GTKRAD_FOLDER>>', "GtkRadiant $GTKRAD_VERSION");

  # uuid
  my $MAIN_GUID=`uuidgen`;
  chop($MAIN_GUID);
  chop($MAIN_GUID);
  print "  $SETUP_DIR/ has uuid: $MAIN_GUID...\n";
  replace("$SETUP_DIR/GtkRadiant.ipr", '<<GTKRAD_ISGUID>>', "$MAIN_GUID");
  replace("$SETUP_DIR/String Tables/0009-English/value.shl", '<<GTKRAD_ISGUID>>', "$MAIN_GUID");
  
}

# configure a tree:
# - what is included in the media (core, game packs)
# - full/nightly
# TODO: this could be cleaner
#   the variable names could be generated from DO_CORE DO_GAME_Q3 DO_GAME_WOLF
sub configure_tree
{
  my $DIR = $SETUP_DIR;
  print "  Configuring $DIR/...\n";
  # get the major and minor
  my $RADIANT_MAJOR=`cat ../../include/RADIANT_MAJOR`;
  chomp($RADIANT_MAJOR);
  chomp($RADIANT_MAJOR);
  my $RADIANT_MINOR=`cat ../../include/RADIANT_MINOR`;
  chomp($RADIANT_MINOR);
  chomp($RADIANT_MINOR);
  print "    version 1.$RADIANT_MAJOR.$RADIANT_MINOR\n";
  replace("$DIR/Script Files/Setup.rul", '<<RADIANT_MAJOR>>', "$RADIANT_MAJOR");
  replace("$DIR/Script Files/Setup.rul", '<<RADIANT_MINOR>>', "$RADIANT_MINOR");
  if ($DO_CORE == 1)
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_CORE_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_CORE_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_CORE_BOOL_FULL_YESNO>>', 'No');
      replace("$DIR/Script Files/Setup.rul", '<<DO_NIGHTLY_BOOL>>', '1');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_CORE_BOOL_FULL_YESNO>>', 'Yes');      
      replace("$DIR/Script Files/Setup.rul", '<<DO_NIGHTLY_BOOL>>', '0');
    }
  }
  else
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_CORE_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_CORE_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_CORE_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_Q3 == 1)
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_Q3_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q3_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q3_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q3_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_Q3_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q3_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q3_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_WOLF == 1)
  {  
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_WOLF_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_WOLF_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_WOLF_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_WOLF_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {    
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_WOLF_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_WOLF_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_WOLF_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_JKII == 1)
  {  
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_JKII_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JKII_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JKII_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JKII_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {    
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_JKII_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JKII_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JKII_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_STVEF == 1)
  {  
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_STVEF_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_STVEF_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_STVEF_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_STVEF_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {    
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_STVEF_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_STVEF_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_STVEF_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_HALFLIFE == 1)
  {  
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_HALFLIFE_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HALFLIFE_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HALFLIFE_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HALFLIFE_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {    
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_HALFLIFE_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HALFLIFE_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HALFLIFE_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_SOF2 == 1)
  {  
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_SOF2_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_SOF2_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_SOF2_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_SOF2_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {    
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_SOF2_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_SOF2_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_SOF2_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_ET == 1)
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_ET_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_ET_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_ET_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_ET_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_ET_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_ET_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_ET_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_Q1 == 1)
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_Q1_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q1_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q1_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q1_BOOL_FULL_YESNO>>', 'Yes');      
    }
  }
  else
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_Q1_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q1_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q1_BOOL_FULL_YESNO>>', 'No');
  }
  if ($DO_GAME_JA == 1)
  {
  	# prepare the dynamic JA media file group
  	print "  Prepare JA media file group..\n";
  	# would go faster by copying only the right files in the first place
  	# rsync might even go faster - if we use it on a toplevel JAPack copy
  	system( "cp -R ../../../JAPack/base $DIR/JAPack" );
  	svn_cleanup( "$DIR/JAPack" );
  	replace( "$DIR/File Groups/JA Media Files.fgl", '<<JA_MEDIA_PATH>>', "$GTKRAD_DIR/GtkRadiant/setup/win32/$DIR/JAPack" );
  	
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_JA_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JA_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JA_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JA_BOOL_FULL_YESNO>>', 'Yes');      
    }
	}
	else
	{
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_JA_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JA_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_JA_BOOL_FULL_YESNO>>', 'No');
	}
  if ($DO_GAME_Q2 == 1)
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_Q2_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q2_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q2_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q2_BOOL_FULL_YESNO>>', 'Yes');      
    }
	}
	else
	{
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_Q2_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q2_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_Q2_BOOL_FULL_YESNO>>', 'No');
	}
  if ($DO_GAME_HER2 == 1)
  {
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_HER2_BOOL>>', '1');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HER2_BOOL_YESNO>>', 'Yes');
    if ($DO_NIGHTLY == 1)
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HER2_BOOL_FULL_YESNO>>', 'No');
    }
    else
    {
      replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HER2_BOOL_FULL_YESNO>>', 'Yes');      
    }
	}
	else
	{
    replace("$DIR/Script Files/Setup.rul", '<<DO_GAME_HER2_BOOL>>', '0');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HER2_BOOL_YESNO>>', 'No');
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_GAME_HER2_BOOL_FULL_YESNO>>', 'No');
	}
  # and now the nightly only stuff (i.e. media upgrade)
  if ($DO_NIGHTLY == 1)
  {
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_NIGHTLY_BOOL_YESNO>>', 'Yes');
  }
  else
  {
    replace("$DIR/Component Definitions/Default.cdf", '<<DO_NIGHTLY_BOOL_YESNO>>', 'No');
  }
}

# command line options, default to generating the setups
# optional, build a template from a work version

# usage instructions
sub usage {
  print <<eof;
Usage:
Builds IS script trees
  setup.pl <gtkrad_dir> <config_file>
  ex: setup.pl 'C:\\\\home\\\\Id' q3.cf
  where q3.cf holds Q3 only game pack config
or
Takes a work tree and turns it back into a template tree
  setup.pl <gtkrad_dir> -template <template_dir>
  ex: setup.pl 'C:\\\\home\\\\Id' -template template-gen
eof
};

# main

# configure some shit
$UUIDGEN=`which uuidgen.exe 2>/dev/null`;
chop($UUIDGEN);
if (!-e $UUIDGEN)
{
  print "uuidgen not found\n";
  exit;
}

if (scalar(@ARGV)<2)
{
  usage();
  exit;
};

$GTKRAD_DIR=$ARGV[0];
print "Configured for base GtkRadiant directory: '$GTKRAD_DIR'\n";

if (@ARGV>2 && $ARGV[1] eq '-template')
{
  # building a template from a work dir
  $TEMPLATE_GEN_DIR=$ARGV[2];
  print "Building a template version of $WORK_DIR/ into $TEMPLATE_GEN_DIR/\n";
  print "Copy files...\n";
  system("rm -rf $TEMPLATE_GEN_DIR");
  system("cp -R $WORK_DIR $TEMPLATE_GEN_DIR\n");
  # replace UUID
  print "Templating UUID...\n";
  system("cat $TEMPLATE_GEN_DIR/GtkRadiant.ipr | sed -e 's/InstallationGUID.*/InstallationGUID=<<GTKRAD_ISGUID>>/' > $TEMPLATE_GEN_DIR/GtkRadiant.ipr.tmp");
  system("cp $TEMPLATE_GEN_DIR/GtkRadiant.ipr.tmp $TEMPLATE_GEN_DIR/GtkRadiant.ipr; rm $TEMPLATE_GEN_DIR/GtkRadiant.ipr.tmp");
  # base dir
  print "Processing '$GTKRAD_DIR' into '<<GTKRAD_DIR>>'\n";
  open($findscan, "find $TEMPLATE_GEN_DIR -name '*.fgl' | ");
  my $line = <$findscan>;
  do
  {
    chop($line);
    replace($line, $GTKRAD_DIR, '<<GTKRAD_DIR>>');
  } until (!($line = <$findscan>));
  close($findscan);
  exit;
}

# set default config
$SETUP_DIR = 'SetupOutput';
$DO_CORE = 1;
$DO_GAME_Q3 = 0;
$DO_GAME_WOLF = 0;
$DO_GAME_JKII = 0;
$DO_GAME_STVEF = 0;
$DO_GAME_HALFLIFE = 0;
$DO_GAME_SOF2 = 0;
$DO_GAME_ET = 0;
$DO_GAME_Q1 = 0;
$DO_GAME_JA = 0;
$DO_GAME_Q2 = 0;
$DO_GAME_HER2 = 0;
$DO_NIGHTLY = 0;
  
# load config

my $config_file = $ARGV[1];

print "Reading config from $config_file\n";
eval "require \"$config_file\"";
if ($SETUP_DIR eq $WORK_DIR)
{
  die "Error: $WORK_DIR is reserved\n";
}

print "Config:\n";
print "  Output directory : $SETUP_DIR\n";
print "  DO_CORE          : $DO_CORE\n";
print "  DO_GAME_Q3       : $DO_GAME_Q3\n";
print "  DO_GAME_WOLF     : $DO_GAME_WOLF\n";
print "  DO_GAME_JKII     : $DO_GAME_JKII\n";
print "  DO_GAME_STVEF    : $DO_GAME_STVEF\n";
print "  DO_GAME_HALFLIFE : $DO_GAME_HALFLIFE\n";
print "  DO_GAME_SOF2     : $DO_GAME_SOF2\n";
print "  DO_GAME_ET       : $DO_GAME_ET\n";
print "  DO_GAME_Q1       : $DO_GAME_Q1\n";
print "  DO_GAME_JA       : $DO_GAME_JA\n";
print "  DO_GAME_Q2       : $DO_GAME_Q2\n";
print "  DO_GAME_HER2     : $DO_GAME_HER2\n";
if ($DO_NIGHTLY == 1)
{
  print "  Configured for nightly setup\n";
}
else
{
  print "  Configured for full setup\n";
}

build_work_tree();

print "Preparing $SETUP_DIR/...\n";
system("rm -rf $SETUP_DIR");
system("cp -R $WORK_DIR $SETUP_DIR");
finalize_tree("$SETUP_DIR");
configure_tree();
