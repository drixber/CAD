#!/usr/bin/env bash
# AUR-Upload: PKGBUILD + .SRCINFO erzeugen und ins AUR-Repo pushen.
# Voraussetzungen: Arch Linux, AUR-Account, SSH-Key bei AUR hinterlegt.
# Nutzung: Anpassen der Variablen unten, dann: ./aur-upload.sh
# Läuft ohne Rückfragen (non-interactive).

set -euo pipefail

# ========== ANPASSEN ==========
PKGNAME="hydracad"
# Version: 1. Argument (z. B. ./aur-upload.sh 3.0.20) oder Umgebungsvariable PKGVER
PKGVER="${1:-${PKGVER:-3.0.22}}"
PKGREL="${PKGREL:-1}"
PKGDESC="Professional CAD application (C++/Qt6)"
ARCH="x86_64"
LICENSE="custom"
GITHUB_USER="drixber"
GITHUB_REPO="CAD"
BINNAME="cad_desktop"
# Für Git-Commit im AUR-Repo (wird nur lokal im Clone gesetzt)
AUR_GIT_USER="${AUR_GIT_USER:-Hydra CAD}"
AUR_GIT_EMAIL="${AUR_GIT_EMAIL:-noreply@hydracad.local}"
# ==============================

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
echo "[+] Workdir: $WORKDIR"
cd "$WORKDIR"

echo "[+] Erzeuge PKGBUILD"
cat > PKGBUILD << EOF
# Hydra CAD – AUR (generiert von packaging/arch/aur-upload.sh)
pkgname=${PKGNAME}
pkgver=${PKGVER}
pkgrel=${PKGREL}
pkgdesc="${PKGDESC}"
arch=("${ARCH}")
url="https://github.com/${GITHUB_USER}/${GITHUB_REPO}"
license=("${LICENSE}")
depends=('qt6-base' 'qt6-tools' 'qt6-translations')
optdepends=('qt6-network: in-app updates without curl')
makedepends=('cmake' 'git')

source=("\${pkgname}-\${pkgver}.tar.gz::https://github.com/${GITHUB_USER}/${GITHUB_REPO}/archive/refs/tags/v\${pkgver}.tar.gz")
sha256sums=('SKIP')

build() {
  cd "\${srcdir}/${GITHUB_REPO}-\${pkgver}"
  cmake -B build -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON
  cmake --build build -j"\$(nproc)"
}

package() {
  cd "\${srcdir}/${GITHUB_REPO}-\${pkgver}"
  if [ -f packaging/linux/cad_desktop.wrapper ]; then
    install -Dm755 build/${BINNAME} -t "\${pkgdir}/usr/lib/hydracad"
    install -Dm755 packaging/linux/cad_desktop.wrapper "\${pkgdir}/usr/bin/cad_desktop"
  else
    install -Dm755 build/${BINNAME} -t "\${pkgdir}/usr/bin"
  fi
  ln -s cad_desktop "\${pkgdir}/usr/bin/${PKGNAME}"
  install -Dm644 packaging/linux/HydraCAD.desktop -t "\${pkgdir}/usr/share/applications"
  install -Dm644 installer/license.txt "\${pkgdir}/usr/share/licenses/\${pkgname}/LICENSE"
}
EOF

echo "[+] Generiere .SRCINFO"
makepkg --printsrcinfo > .SRCINFO

echo "[+] Klone AUR-Repo (SSH)"
git clone --depth 1 "ssh://aur@aur.archlinux.org/${PKGNAME}.git" aur-repo
cd aur-repo

git config user.name "$AUR_GIT_USER"
git config user.email "$AUR_GIT_EMAIL"

echo "[+] Kopiere PKGBUILD und .SRCINFO"
cp ../PKGBUILD .
cp ../.SRCINFO .

echo "[+] Commit und Push"
git add PKGBUILD .SRCINFO
git diff --staged --quiet && { echo "[!] Keine Änderungen, nichts zu pushen."; exit 0; }
git commit -m "Update to ${PKGVER}"
git push

echo
echo "[✓] Fertig. AUR: https://aur.archlinux.org/packages/${PKGNAME}"
echo "[i] Install-Test: yay -S ${PKGNAME}  bzw.  paru -S ${PKGNAME}"
