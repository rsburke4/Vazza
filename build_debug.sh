#!/bin/bash
cmake cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cd build
make
cd ../assets/shaders
for file in *.slang; do
#    [ -e "$file" ] || continue
    echo "Compiling $file"
    slangc "$file" -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -entry main -o "${file}.spv"
done
#find . -type f \( -name "*.vert" -o -name "*.frag" \) | xargs -I {} -P $(nproc) \
#slangc "{}" -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -entry main -o "{}.spv"
cd ../../
