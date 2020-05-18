#!/bin/bash
readonly VARIANT="$1"
readonly CMAKE_ARGS=("${@:2}")
readonly BASE=$(dirname "$0")

function configure-variant(){
    local build="$BASE/build-$1/"
    mkdir "$build"
    cmake "${@:2}" -S "$BASE" -B "$build"
}

function compile-variant(){
    local build="$BASE/build-$1/"
    make -C "$build" -j 8
}

function measure-variant(){
    local build="$BASE/build-$1/"
    make -C "$build" measure VARIANT="$1"
}

function main(){
    if [ -z "$VARIANT" ]; then
        echo "Must pass a variant name as first argument!"
        exit 1
    fi
    configure-variant "$VARIANT" "${CMAKE_ARGS[@]}"
    compile-variant "$VARIANT"
    measure-variant "$VARIANT"
}

main
