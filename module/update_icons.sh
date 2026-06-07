#!/usr/bin/env bash


GRAY='\033[1;30m'
GREEN='\033[0;32m'
NC='\033[0m' #No Color

QRC_FILE="./haqml_resources.qrc"
ICONS_DIR="assets/themes/icons/material"

if [ ! -f "$QRC_FILE" ]; then
    echo "Error: File $QRC_FILE not found!"
    exit 1
fi

echo "Scan icons..."

existing_icons=$(grep -o "assets/.*\.svg" "$QRC_FILE" | sort)

current_icons=$(find "$ICONS_DIR" -name "*.svg" | sort)

echo -e "${GRAY}--- Old icons QRC ---"
while read -r line; do
    if [ ! -z "$line" ]; then
        echo "  $line"
    fi
done <<< "$existing_icons"
echo -e "${NC}"

new_icons=$(comm -13 <(echo "$existing_icons") <(echo "$current_icons"))

if [ -z "$new_icons" ]; then
    echo "${GREEN}New icons not found. QRC is actual!."
    exit 0
fi

echo -e "${GREEN}--- Add new icons ---"
while read -r line; do
    echo "  + $line"
done <<< "$new_icons"
echo -e "${NC}"

themes=$(find assets/themes -maxdepth 1 -name "*.json" | sort)

TMP_QRC=$(mktemp)

echo '<!DOCTYPE RCC>' >> "$TMP_QRC"
echo '<RCC version="1.0">' >> "$TMP_QRC"
echo '    <qresource prefix="/haqml">' >> "$TMP_QRC"

while read -r file; do
    [ -n "$file" ] && echo "        <file>$file</file>" >> "$TMP_QRC"
done <<< "$themes"

echo "" >> "$TMP_QRC"

while read -r file; do
    [ -n "$file" ] && echo "        <file>$file</file>" >> "$TMP_QRC"
done <<< "$current_icons"

echo '    </qresource>' >> "$TMP_QRC"
echo '</RCC>' >> "$TMP_QRC"

mv "$TMP_QRC" "$QRC_FILE"

echo -e "${GREEN}Success!${NC}"