#!/usr/bin/env bash
# Run this ONCE after clicking "Create the first page" on:
# https://github.com/2E0LXY/wsjt-zii/wiki
set -e
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP=$(mktemp -d)
git clone https://github.com/2E0LXY/wsjt-zii.wiki.git "$TMP"
cp "$REPO_ROOT"/wiki/*.md "$TMP/"
cd "$TMP"
git add .
git commit -m "Initialise WSJT-Zii wiki"
git push
rm -rf "$TMP"
