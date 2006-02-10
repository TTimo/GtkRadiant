#!/usr/bin/env perl
#
# Common setup functions for building release packages
#

# Gef - Jan3 2002
# - Initial framework for migrating setup/release building to perl

# TODO:
# Gef - Cleaner handling of cons builds, currently doesn't catch cons errors

package setup;

# Package constructor
sub new
{
  my $this = {};
  bless $this;

  return $this;
}

# Replace in file parm-1; parm-2 with parm-3
sub replace
{
  shift;
  my $file = shift(@_);
  my $search = shift(@_);
  my $replace_with = shift(@_);
  
  # need to use | instead of / with sed since the variables will contain /'s which confuse it
  system("cat '$file' | sed -e 's|$search|$replace_with|g' > '$file.tmp'");
  system("cp '$file.tmp' '$file'; rm '$file.tmp'");
}

# Not exactly common (between win32/linux), but useful here all the same
sub cons_build
{
  shift;
  my $BUILD_DIR = shift(@_);
  my $BUILD_CMD = shift(@_);
  # use a direct system() call since syscmd doesnt catch cons errors
  system("cd $BUILD_DIR; $BUILD_CMD"); 
}

# Maintain a list of errors that have occured
sub collate_error
{
  #shift;
  my $err_type = shift(@_);     # unused
  my $err_command = shift(@_);

  @errors[$err_count++] = "$err_command";
}

# Output the list of errors stored
sub print_errors
{
  my $count = 0;
  
  if($err_count gt 0)
  {
    if($err_count > 25)
    {
      print("$err_count Errors!! Ouch, looks like something screwed up.\n");
    }
    else
    {
      print("$err_count Error(s) encountered\n");
    }
    
    for($count; $count lt $err_count; $count++)
    {
      if(@errors[$count] ne "")
      {
        print("-> @errors[$count]\n");
      }
    }
  }
  #else
  #{
  #  print("No errors encountered.\n");
  #}
}

# A wrapper for system() calls that catches errors
sub syscmd
{
  shift;
  my $command_string = shift(@_);

  # todo: identify multiple commands (commands split with ;'s)
  # todo: catch cons errors (cons doesn't return a value)
  system("$command_string");
  my $sysretval = $?;

  if(($sysretval gt 0) && ($sysretval lt 257))
  {
    @cmdlist = split(" ", $command_string);
    if(@cmdlist[0] eq "cp")
    {
      collate_error("copy", $command_string);
    }
    elsif(@cmdlist[0] eq "mv")
    {
      collate_error("move", $command_string);
    }
    elsif(@cmdlist[0] eq "cons")
    {
      collate_error("cons", $command_string);
    }
    elsif(@cmdlist[0] eq "cd")
    {
      collate_error("changed dir", $command_string);
    }
    elsif(@cmdlist[0] eq "mkdir")
    {
      collate_error("make dir", $command_string);
    }
    elsif(@cmdlist[0] eq "cat")
    {
      collate_error("cat", $command_string);
    }
    elsif(@cmdlist[0] eq "rm")
    {
      collate_error("remove", $command_string);
    }    
    else
    {
      collate_error("unhandled", $command_string);
    }
  }

  return $sysretval;  
}


# Close package
1;
