#!perl
rename("brush.c", "brush.c.old");
open(FILE, "brush.c.old");
open(OFILE, ">brush.c");
while(<FILE>)
{
if($. != 150)
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
