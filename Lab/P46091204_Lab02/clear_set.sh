#!/bin/sh

echo " The clear_env script $0 is now running "

# if src exists , remove the folder of Code

if [ -e src ]
then
  echo " Do you really want to remove the folder of Code ?"
  echo " Please answer y or n "
  read answer
  if [ $answer = "y" ]
  then  
  echo " The folder is now removed ! "
  if [ -e ./src ]
  then
    rm -r ./src
  fi
  if [ -e ./bin ]
  then
    rm -r ./bin
  fi
fi  # end remove

# if Code does not exist , echo " Code does not exist ! "
else
  echo " Code does not exist ! "
fi
echo " The script is now complete "
exit 0 
