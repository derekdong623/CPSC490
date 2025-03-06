#!/bin/bash

# Get the directory of the script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set the path to the pokemon-showdown executable
PS_EXEC="$SCRIPT_DIR/../pokemon-showdown/pokemon-showdown"
JSON_DIR="$SCRIPT_DIR/json_format.json"
EXPORT_DIR="$SCRIPT_DIR/export_format.txt"
TMP_PACK_DIR="$SCRIPT_DIR/tmp_pack.txt"
PACK_DIR="$SCRIPT_DIR/packed_format.txt"

# Run the commands using the absolute path
"$PS_EXEC" pack-team < "$JSON_DIR" > "$TMP_PACK_DIR"
sed 's/\]/\n/g' "$TMP_PACK_DIR" > "$PACK_DIR"
"$PS_EXEC" export-team < "$JSON_DIR" > "$EXPORT_DIR"