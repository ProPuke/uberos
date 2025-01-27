#!/bin/sh
echo --allow-read="$1" --allow-write="$1" $(dirname "$(realpath "$0")")/pack-image.ts "$@"
deno run --allow-read="$1" --allow-write="$1" $(dirname "$(realpath "$0")")/pack-image.ts "$@"
