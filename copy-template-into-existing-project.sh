#!/usr/bin/env bash
set -eu

# Usage
# Go to the project you want to improve via this template
# cd ~/git/my-project
# Run this script from the working directory of that project
# ~/git/platformio-template/copy-template-into-existing-project.sh

git diff --quiet || (echo "repo unclean. stage or commit first" && exit 1)

name=$(basename "$PWD")
templatedir="$(dirname "$0")"

cp -r \
	"$templatedir/"{.github,.gitignore,LICENSE,platformio.ini} \
	.

echo "everything copied"

# Replace template name with folder name
# macOS: add '' after -i like this: sed -i '' "s/…
sed -i "s/platformio-template/$name/g" .github/**/*.yml

git --no-pager status --short
