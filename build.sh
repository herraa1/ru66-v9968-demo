#!/bin/bash

# V9968_demo Build Script
# Builds the V9968 hardware demonstration project for MSXgl

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Navigate to the MSXgl root directory (../..)
MSXGL_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

# Build the project using MSXgl's build system
# The build system will use Node.js to execute the build process
if [ -f "$MSXGL_ROOT/engine/script/js/build.js" ]; then
	cd "$SCRIPT_DIR"
	node "$MSXGL_ROOT/engine/script/js/build.js" projname=RU66 $@
else
	echo "Error: Could not find MSXgl build system at $MSXGL_ROOT/engine/script/js/build.js"
	echo "Please run this script from within an MSXgl project directory."
	exit 1
fi
