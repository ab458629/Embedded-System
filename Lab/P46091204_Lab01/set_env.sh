#!/bin/sh

echo "The set_env script $0 is now running"

# if Code exists , echo the Code exists  
if [ -e ./code ]
then 
  echo "lab exists"
# if code folder does not exists , make the code folder of lab-04
else
  cp -rf ./.src ./code
  echo "lab builded !"
fi

echo "The script is now complete"

