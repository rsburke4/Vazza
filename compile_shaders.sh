find assets/shaders -type f \( -name "*.vert.slang" -o -name "*.frag.slang" \) | while read -r file; do
    if [[ "$file" == *.vert.slang ]]; then
        # Compile Vertex Shaders
        slangc "$file" -entry main -stage vertex -profile sm_6_5 -target spirv -profile spirv_1_3 -o "${file%.vert.slang}.vert.spv"
    else
        # Compile Fragment Shaders
        slangc "$file" -entry main -stage fragment -profile sm_6_5 -target spirv -profile spirv_1_3 -o "${file%.frag.slang}.frag.spv"
    fi
done
