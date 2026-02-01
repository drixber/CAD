#!/usr/bin/env bash
# Lokalen Paketbau: Tarball aus aktuellem Repo erzeugen, dann makepkg -si.
# Nutzung: aus packaging/arch ausführen – nicht als root.
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$SCRIPT_DIR"
PKGVER="${1:-3.0.22}"
TARBALL="CAD-${PKGVER}.tar.gz"
echo "[+] Erzeuge $TARBALL aus $REPO_ROOT"
(cd "$REPO_ROOT" && git archive --prefix=CAD/ -o "$SCRIPT_DIR/$TARBALL" HEAD)
echo "[+] Starte makepkg -si (nicht als root ausführen)"
exec makepkg -si
