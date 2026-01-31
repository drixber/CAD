# Arch Linux – Paketierung (pacman / yay)

Dieses Verzeichnis enthält den **PKGBUILD** für Hydra CAD. Damit kannst du:

- **Lokal mit pacman installieren:** `makepkg -si` (baut und installiert über pacman).
- **Über yay installieren:** Dafür muss das Paket einmal auf der AUR liegen (siehe [docs/INSTALL_ARCH_PACMAN_YAY.md](../../docs/INSTALL_ARCH_PACMAN_YAY.md)).

**Installation (lokal):**

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-tools git
git clone https://github.com/drixber/CAD.git
cd CAD/packaging/arch
makepkg -si
```

- **Weiterentwicklung auf Arch:** [docs/BUILD_ARCH.md](../../docs/BUILD_ARCH.md)
- **pacman/yay und AUR:** [docs/INSTALL_ARCH_PACMAN_YAY.md](../../docs/INSTALL_ARCH_PACMAN_YAY.md)
- **PKGBUILD:** `pkgver` bei neuem Release anpassen (z. B. 3.0.7); `sha256sums=('SKIP')` bei Git-Source üblich.
