#!/bin/sh

svn co http://crosswire.org/svn/sword/trunk sword
sed -i -e "s/1.8.900/1.8.0.0/g" sword/CMakeLists.txt
mkdir -p sword_build
cd sword_build
cmake ../sword
make sword
