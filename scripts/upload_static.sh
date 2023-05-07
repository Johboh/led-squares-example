#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WORK_DIR=`mktemp -d -p "$DIR"`

function cleanup {
  rm -rf "$WORK_DIR"
}

trap cleanup EXIT

f="$(basename -- $1)"

convert $1 -scale 56x40^ -gravity center -background black -extent 56x40 -level 20%,100% -background black -gravity center -depth 8 -type truecolor -define bmp:subtype=RGB24 BMP3:$WORK_DIR/output.bmp &&
python3 upload.py -f $f -u "http://192.168.1.103" $WORK_DIR &&
curl "http://192.168.1.103/select?filename=/$f&blendDuration=5000"
