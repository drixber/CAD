#!/usr/bin/env bash
# Build .deb package for Ubuntu/Debian.
# Run from the repository root: ./packaging/ubuntu/build-deb.sh
# Requires: build-essential, cmake, qt6-base-dev, debhelper, dpkg-dev

set -e
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"

if [ ! -d packaging/debian ]; then
  echo "packaging/debian not found."
  exit 1
fi

# Copy debian/ into place (do not modify packaging/debian in place)
rm -rf debian
cp -r packaging/debian debian

# Ensure rules is executable
chmod +x debian/rules

echo "Building .deb (binary package, unsigned)..."
dpkg-buildpackage -b -us -uc -rfakeroot

echo "Done. .deb is in parent directory: $(dirname "$ROOT")/"
ls -la ../*.deb 2>/dev/null || true
