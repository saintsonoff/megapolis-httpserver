# MEGAPOLIS HTTP SERVER
http server, async + multihreatding handling

# requirements
C++23, boost >= 1.86, cmake >=3.31

# endpoints
`/info` ANY 200 ("all ok")
`/upload` POST 200 ("Success")
another:

# building
local:
from project root
```
mkdir build
cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE
cmake --build ./build
```

docker container:
1. (optional) build toolchain docker image from ./toolchain/ARCHITETURE/development/Dockerfile
2. add .env to ./toolchain/ARCHITETURE/building, like .env-example from ./toolchain/ARCHITETURE/building
3. run ./toolchain/ARCHITETURE/building/building.sh script
4. find elf in LOCAL_EXPORT_PATH

# run
`./build/app/httpserver`

# documentation
requirements:
doxygen, graphviz

from project root:
`doxygen Doxyfile`

open:
open docs/html/index.html file in browser