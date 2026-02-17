ARG TOOLCHAIN_IMAGE
FROM ${TOOLCHAIN_IMAGE}

WORKDIR /app

COPY ./src ./src

RUN printf '#!/bin/bash\n\
set -e\n\
echo "--- Starting build [Mode: $BUILD_TYPE] ---"\n\
mkdir -p /app/build\n\
cmake -S /app/src -B /app/build -DCMAKE_BUILD_TYPE=$BUILD_TYPE\n\
cmake --build /app/build --parallel $(nproc)\n\
\n\

TARGET="/app/build/app/httpserver"\n\
if [ -f "$TARGET" ]; then\n\
    cp "$TARGET" /export/httpserver\n\
    echo "--- Success: /export/httpserver is ready ---"\n\
else\n\
    echo "--- Error: Binary $TARGET not found ---"\n\
    exit 1\n\
fi\n' > /app/entrypoint.sh && chmod +x /app/entrypoint.sh

RUN mkdir -p /export

ENTRYPOINT ["/app/entrypoint.sh"]