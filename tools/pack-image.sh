#!/bin/sh
deno run --allow-run=inkscape --allow-read="$1" --allow-write="$1" $(dirname "$(realpath "$0")")/pack-image.ts "$@"
