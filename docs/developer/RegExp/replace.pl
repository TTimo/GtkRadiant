#!perl
rename("$ARGV[0]", "$ARGV[0].old");
open(FILE, "$ARGV[0].old");
open(OFILE, ">$ARGV[0]");
while(<FILE>)
{
if($. != $ARGV[1])
{
print OFILE;
next;
}
s/Sys_Printf \(/Sys_FPrintf \(SYS_VRB,/;
s/Sys_Printf\(/Sys_FPrintf \(SYS_VRB,/;
print OFILE;
}
close(OFILE);
close(FILE);
