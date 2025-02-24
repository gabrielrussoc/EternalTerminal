#!/bin/bash

set -euxo pipefail

# Detect the operating system
os_type=$(uname)

# Check the OS and execute commands accordingly
if [ "$os_type" = "Darwin" ]; then
    echo "Running on macOS, building 'et' client binary"
    bazelisk build -c opt //:et
elif [ "$os_type" = "Linux" ]; then
    echo "Running on Linux, building 'etserver' and 'etterminal' binaries"
    bazelisk build -c opt //:etserver //:etterminal
else
    echo "Unsupported operating system: $os_type"
fi

