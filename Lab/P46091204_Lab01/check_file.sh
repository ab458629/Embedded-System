#!/bin/sh

echo "The check_file script $0 is now running"

for source_file in $( ls ./code); do
    if [ -e code ];
    then
        echo " $source_file exist "
    else
        echo " $source_file does not exist "
    fi
done

exit 0



