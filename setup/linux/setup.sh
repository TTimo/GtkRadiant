#! /bin/sh
#
# Product setup script - Loki Entertainment Software

# TTimo FIXME need a way to configure this easily
critical_error="Please contact Id software technical support at bugs@idsoftware.com"

# Go to the proper setup directory (if not already there)
cd `dirname $0`

# Return the appropriate architecture string
DetectARCH()
{
	status=1
	case `uname -m` in
		i?86)  echo "x86"
			status=0;;
		*)     echo "`uname -m`"
			status=0;;
	esac
	return $status
}

# Return the appropriate version string
DetectLIBC()
{
      status=1
	  if [ `uname -s` != Linux ]; then
		  echo "glibc-2.1"
		  return $status
	  fi
      if [ -f `echo /lib/libc.so.6* | tail -1` ]; then
	      if fgrep GLIBC_2.1 /lib/libc.so.6* 2>&1 >/dev/null; then
	              echo "glibc-2.1"
	              status=0
	      else    
	              echo "glibc-2.0"
	              status=0
	      fi        
      elif [ -f /lib/libc.so.5 ]; then
	      echo "libc5"
	      status=0
      else
	      echo "unknown"
      fi
      return $status
}

# Detect the Linux environment
arch=`DetectARCH`
libc=`DetectLIBC`
os=`uname -s`

# Find the installation program
# try_run INSTALLER_NAME [-fatal] [PARAMETERS_PASSED]
#   INSTALLER_NAME: setup.gtk or setup
#   -fatal option: if you want verbose messages in case
#      - the script could not be found
#      - it's execution would fail
#   PARAMETERS_PASSED: additional arguments passed to the setup script
try_run()
{
    setup=$1
    shift
    # added safe check, direct test seems buggy on older bash
    if [ "$#" > 0 ]; then
      # looks like bash < 2.* don't like == operator, using = instead
      if [ "$1" = "-fatal" ]; then
        # got fatal
        fatal=$1
        shift
      fi
    fi

    # First find the binary we want to run
    failed=0
    setup_bin="setup.data/bin/$os/$arch/$libc/$setup"
    # trying $setup_bin
    if [ ! -f "$setup_bin" ]; then
        setup_bin="setup.data/bin/$os/$arch/$setup"
      	# libc dependant version failed, trying again
        if [ ! -f "$setup_bin" ]; then
            failed=1
        fi
    fi
    if [ "$failed" -eq 1 ]; then
        if [ "$fatal" != "" ]; then
            cat <<__EOF__
This installation doesn't support $libc on $os / $arch

$critical_error
__EOF__
            exit 1
        fi
        return $failed
    fi

    # Try to run the binary ($setup_bin)
    # The executable is here but we can't execute it from CD
    setup="$HOME/.setup$$"
    cp "$setup_bin" "$setup"
    chmod 700 "$setup"
    if [ "$fatal" != "" ]; then
        "$setup" $*
        failed=$?
    else
        "$setup" $* 2>/dev/null
        failed=$?
    fi
    rm -f "$setup"
    return $failed
}


# Try to run the setup program
status=0
rm -f "$setup"
try_run setup.gtk $* || try_run setup -fatal $* || {
    echo "The setup program seems to have failed on $arch/$libc"
    echo
    echo $critical_error
    status=1
}
exit $status
