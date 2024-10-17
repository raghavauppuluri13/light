#!/bin/bash

# Check if two arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <old_name> <new_name>"
    exit 1
fi

old_string=$1
new_string=$2

# Replace in file contents (for .h, .c, and .py files, ignoring case)
find . -type f \( -name "*.h" -o -name "*.c" -o -name "*.py" \) -exec sed -i "s/${old_string}/${new_string}/gI" {} +

# Replace in filenames and directory names, ignoring case
find . -depth -name "*${old_string}*" -execdir rename "y/$(echo ${old_string} | tr '[:lower:]' '[:upper:]')/$(echo ${new_string} | tr '[:lower:]' '[:upper:]')/" "{}" \;

echo "Rename complete!"
