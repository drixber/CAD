# Arch Linux – Paketierung (pacman / yay)

Dieses Verzeichnis enthält den **PKGBUILD** für Hydra CAD. Damit kannst du:

- **Lokal mit pacman installieren:** `makepkg -si` (baut und installiert über pacman).
- **Über yay installieren:** Dafür muss das Paket einmal auf der AUR liegen (siehe [docs/INSTALL_ARCH_PACMAN_YAY.md](../../docs/INSTALL_ARCH_PACMAN_YAY.md)).

**Installation (lokal):**

- **Wichtig:** `makepkg` **nicht als root** ausführen (normale Benutzer-Shell).

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-tools git
git clone https://github.com/drixber/CAD.git
cd CAD/packaging/arch
makepkg -si
```

**Build aus lokalem Repo (ohne GitHub-Tag):**

Wenn du im Repo Änderungen hast und direkt bauen willst (ohne Tag/Push):

```bash
cd ~/GitHub/CAD/packaging/arch
chmod +x build-local.sh
./build-local.sh
```

Das Skript erzeugt einen Tarball aus dem aktuellen Stand und führt danach `makepkg -si` aus.

- **Weiterentwicklung auf Arch:** [docs/BUILD_ARCH.md](../../docs/BUILD_ARCH.md)
- **pacman/yay und AUR:** [docs/INSTALL_ARCH_PACMAN_YAY.md](../../docs/INSTALL_ARCH_PACMAN_YAY.md)
- **PKGBUILD:** `pkgver` bei neuem Release anpassen (z. B. 3.0.7); `sha256sums=('SKIP')` bei Git-Source üblich.

### AUR-Upload (automatisch)

Skript **`aur-upload.sh`** erzeugt PKGBUILD + .SRCINFO und pusht sie ins AUR-Repo (ohne Rückfragen).

**Voraussetzungen:** Arch Linux, AUR-Account, SSH-Key bei AUR hinterlegt. Das AUR-Paket muss auf [aur.archlinux.org](https://aur.archlinux.org) einmal angelegt sein („Add Package“ → Paketname z. B. `hydracad`).

**AUR-SSH-Fingerabdrücke** (zum Prüfen beim ersten Verbinden, siehe [AUR Wiki](https://wiki.archlinux.org/title/Arch_User_Repository#Submitting_packages)):

| Key-Typ | SHA256-Fingerprint |
|---------|--------------------|
| Ed25519 | `SHA256:RFzBCUItH9LZS0cKB5UE6ceAYhBD5C8GeOBip8Z11+4` |
| ECDSA   | `SHA256:uTa/0PndEgPZTf76e1DFqXKJEXKsn7m9ivhLQtzGOCI` |
| RSA     | `SHA256:5s5cIyReIfNNVGRFdDbe3hdYiI5OelHGpw2rOUud3Q8` |

```bash
# Im Projektroot (CAD-Repo, dort wo README.md und packaging/ liegen)
cd packaging/arch
# Variablen oben in aur-upload.sh anpassen (PKGNAME, PKGVER, GITHUB_USER, …)
./aur-upload.sh
```

Das Skript: erzeugt PKGBUILD mit den gesetzten Variablen, generiert .SRCINFO per `makepkg --printsrcinfo`, klont das AUR-Repo per SSH, kopiert PKGBUILD und .SRCINFO, committed und pusht.

**„Permission denied (publickey)“ beim Klonen:**  
AUR akzeptiert nur SSH-Keys, die in deinem AUR-Account hinterlegt sind.

1. **SSH-Key bei AUR eintragen:** [aur.archlinux.org](https://aur.archlinux.org) → **Login** → **My Account** → **Edit Profile** → bei **SSH Public Key** deinen **öffentlichen** Key einfügen (z. B. Inhalt von `cat ~/.ssh/id_ed25519.pub` oder `~/.ssh/id_rsa.pub`). Speichern.
2. **Richtigen Key verwenden:** Wenn du mehrere Keys hast, teste mit `ssh -T aur@aur.archlinux.org`. Die Meldung sollte deinen AUR-Benutzernamen anzeigen; wenn „Permission denied“ kommt, wird ein anderer Key genutzt – dann z. B. `ssh-add ~/.ssh/id_ed25519` (oder deinen Key) und erneut versuchen.
3. **Paket muss existieren:** Das AUR-Paket **hydracad** muss einmal angelegt sein (Submit Package); der Account, bei dem der SSH-Key eingetragen ist, muss Maintainer dieses Pakets sein.
