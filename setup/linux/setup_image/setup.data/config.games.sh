#!/bin/sh
#
# Script for generating .game files

# generate game support files
# Expects params (game, game path_prefix)

exec >/dev/tty

  if [ $# -lt 2 ] ; then
    echo "Incorrect number of params to GenGameFiles";
    return;
  fi

  # NOTE: the naming of the vars here doesn't follow attributes names in XML, it can be confusing
  game=$1;
  INPUT_FILE="games/$game.game";
  game_engine=`grep "enginepath_linux=" $INPUT_FILE | awk -F"\"" '{print $2 }'`
  game_engine_path=$SETUP_COMPONENT_PATH;
  game_tools=$2;
  OUT_FILE="$game_tools/games/$game.game";

  echo -e "Generating game file '$OUT_FILE' from '$INPUT_FILE' with the following values...";
  echo -e "\tGame        :\t $game";
  echo -e "\tGame Engine :\t $game_engine_path/$game_engine";
  echo -e "\tGame Tools  :\t $game_tools";

  if [ -f "$OUT_FILE" ] ; then
    rm -f $OUT_FILE;
  fi
  
  if [ ! -d "$game_tools/games" ] ; then
    mkdir "$game_tools/games";
  fi
  
sed -e 's!enginepath_linux=\".*.\"!enginepath_linux=\"'$game_engine_path'\"\n  gametools=\"'$game_tools/$game'\"!g' <$INPUT_FILE >$OUT_FILE

exit 0;
