#!/bin/bash

# Cross-platform library linking script
# Uses PowerShell on Windows for better reliability

# Get the directory where script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="$SCRIPT_DIR/src/Melkens_Lib"
SOURCE_DIR="$SCRIPT_DIR/../Melkens_Lib"

echo "Script directory: $SCRIPT_DIR"
echo "Target directory: $TARGET_DIR"
echo "Source directory: $SOURCE_DIR"

# Check if target already exists
if [ -e "$TARGET_DIR" ]; then
    echo "Link/folder 'Melkens_Lib' already exists. Skipping creation."
    exit 0
fi

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory '$SOURCE_DIR' does not exist!"
    exit 1
fi

# Detect OS and create appropriate link
case "$(uname -s)" in
    MINGW64_NT*|MSYS_NT*|CYGWIN_NT*)
        # Windows - use PowerShell directly
        echo "Detected Windows environment - using PowerShell"
        
        # Convert to absolute Windows paths
        WIN_SOURCE=$(realpath "$SOURCE_DIR")
        WIN_TARGET=$(realpath "$SCRIPT_DIR")/Melkens_Lib
        
        # Convert to Windows format
        WIN_SOURCE=$(cygpath -w "$WIN_SOURCE")
        WIN_TARGET=$(cygpath -w "$WIN_TARGET")
        
        echo "Creating junction: $WIN_TARGET -> $WIN_SOURCE"
        
        # Use PowerShell directly
        powershell.exe -Command "New-Item -ItemType Junction -Path '$WIN_TARGET' -Target '$WIN_SOURCE'"
        
        if [ $? -eq 0 ]; then
            echo "Successfully created junction via PowerShell"
        else
            echo "Failed to create junction. You may need Administrator privileges."
            exit 1
        fi
        ;;
    Linux*)
        # Linux
        echo "Detected Linux environment"
        ln -s "$SOURCE_DIR" "$TARGET_DIR"
        if [ $? -eq 0 ]; then
            echo "Successfully created symbolic link"
        else
            echo "Failed to create symbolic link"
            exit 1
        fi
        ;;
    *)
        echo "Unsupported operating system: $(uname -s)"
        exit 1
        ;;
esac

echo "Link creation completed successfully!"
