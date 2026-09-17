#!/bin/bash
cmake cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cd build
make
cd ../assets/shaders
for file in $(ls *vert.slang); do
#    [ -e "$file" ] || continue
    echo "Compiling $file"
    slangc "$file" -target spirv -profile spirv_1_4 -stage vertex -o "${file}.spv"
done
for file in $(ls *frag.slang); do
    echo Compiling $file
    slangc "$file" -target spirv -profile spirv_1_4 -stage fragment -o "${file}.spv"
done
#find . -type f \( -name "*.vert" -o -name "*.frag" \) | xargs -I {} -P $(nproc) \
#slangc "{}" -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -entry main -o "{}.spv"
cd ../../
