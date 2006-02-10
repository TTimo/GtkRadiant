python.exe -V
if errorlevel=1 echo please install python and add python.exe to the path (http://www.python.org)
if errorlevel=1 goto end

echo python.exe %1 %2 %3 %4 %5 %6 %7 %8 %9
rem FIXME: for some wacked reason, output of the python script doesn't get back to VC window ..
python.exe %1 %2 %3 %4 %5 %6 %7 %8 %9

:end