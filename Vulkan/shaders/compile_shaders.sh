#!/bin/bash

/Users/timi/VulkanSDK/1.4.328.1/macOS/bin/glslc shader.vert -o vert.spv
/Users/timi/VulkanSDK/1.4.328.1/macOS/bin/glslc shader.frag -o frag.spv

echo "Shaders compiled!"
