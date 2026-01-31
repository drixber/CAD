# Arch Linux – Paketierung

Diese Verzeichnis enthält eine **Vorlage** für ein Arch-Linux-Paket (PKGBUILD). Sie dient dazu, Hydra CAD unter Arch zu bauen und ggf. für AUR oder lokale Nutzung zu paketieren.

- **Nicht Teil des automatischen Release-Prozesses** – die Vorlage wird bei Bedarf manuell angepasst.
- **Weiterentwicklung auf Arch:** Siehe [docs/BUILD_ARCH.md](../../docs/BUILD_ARCH.md) für Build und Entwicklung aus dem Quellcode.
- **PKGBUILD:** `pkgver` und `source` (Tag/Branch) bei Nutzung anpassen; `sha256sums=('SKIP')` bei Git-Sources üblich.

Build (im Repo-Root, mit dieser PKGBUILD im Unterordner):

```bash
cd packaging/arch
makepkg -sf
# oder: makepkg -si  # baut und installiert
```
