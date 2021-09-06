#!/bin/sh
archive=archive.tar.gz
touch "$archive"
tar czf "$archive" --exclude="$archive" .