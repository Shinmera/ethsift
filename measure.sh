#!/bin/bash
readonly BASE=${1:-$(dirname "$0")}
readonly TESTER=${2:-$1/build/tester}
readonly IMAGES=(auto-240p.pgm auto-360p.pgm auto-480p.pgm auto-720p.pgm auto-1080p.pgm auto-2160p.pgm auto-4320p.pgm)
readonly VARIANT=${VARIANT:-$(basename $(dirname "$TESTER"))}

function run-all-images(){
    for image in "${IMAGES[@]}"; do
        "$TESTER" $image
    done
}

function run-variant(){
    mkdir -p "$BASE/logs"
    rm $BASE/logs/*.csv
    run-all-images
    if [ -n "$1" ]; then
        mkdir -p "$BASE/logs/$1/"
        mv $BASE/logs/*.csv "$BASE/logs/$1/"
    fi
}

function main(){
    echo "Measuring for variant $VARIANT"
    run-variant "$VARIANT"
}

main
