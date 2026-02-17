#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/../../../")

if [ -f "$SCRIPT_DIR/.env" ]; then
    export $(grep -v '^#' "$SCRIPT_DIR/.env" | xargs)
else
    echo ".env file not found, put it to $SCRIPT_DIR/.env"
    exit 1
fi

mkdir -p "$PROJECT_ROOT/$LOCAL_EXPORT_PATH"
FULL_EXPORT_PATH=$(realpath "$PROJECT_ROOT/$LOCAL_EXPORT_PATH")

echo "export directory: $FULL_EXPORT_PATH"

docker build \
    -f "$SCRIPT_DIR/Dockerfile" \
    --build-arg TOOLCHAIN_IMAGE="$TOOLCHAIN_IMAGE" \
    -t armv7-builder "$PROJECT_ROOT"

docker run --rm \
    -e BUILD_TYPE="$BUILD_TYPE" \
    -v "$FULL_EXPORT_PATH":/export \
    armv7-builder

echo "done"