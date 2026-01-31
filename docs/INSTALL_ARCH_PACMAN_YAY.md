# Hydra CAD unter Arch: Installation mit pacman / yay

## Übersicht

| Art | Wie | Voraussetzung |
|-----|-----|----------------|
| **pacman (lokal)** | Aus dem Repo bauen, dann `pacman -U` (via `makepkg -si`) | Repo klonen, `base-devel` installiert |
| **yay (AUR)** | `yay -S hydracad` | Paket muss auf der AUR liegen (siehe unten) |
| **pacman (offiziell)** | `pacman -S hydracad` | Nur wenn das Paket später in [community] o. Ä. aufgenommen wird (Arch-seitig) |

---

## 1. Installation mit pacman (lokal bauen)

Du baust das Paket selbst und installierst es mit pacman. Dafür brauchst du das Repo und `base-devel`.

```bash
# Abhängigkeiten
sudo pacman -S --needed base-devel cmake qt6-base qt6-tools qt6-translations git

# Repo klonen und Paket bauen + installieren
git clone https://github.com/drixber/CAD.git
cd CAD/packaging/arch
makepkg -si
```

`makepkg -si` baut das Paket und installiert es mit **pacman** (lokal erzeugtes `.pkg.tar.zst`). Danach startest du Hydra CAD mit `cad_desktop` oder über das Anwendungsmenü (Desktop-Eintrag).

**Hinweis:** Im PKGBUILD steht ein fester Tag (`pkgver=3.0.7`). Für eine neuere Version `pkgver` in `packaging/arch/PKGBUILD` anpassen und erneut `makepkg -si` ausführen.

---

## 2. Installation mit yay (AUR)

Mit **yay** (oder einem anderen AUR-Helper) installierst du Pakete aus der **AUR** (Arch User Repository). Damit `yay -S hydracad` funktioniert, muss das Paket **einmal auf der AUR existieren**.

### Wenn es schon ein AUR-Paket „hydracad“ gibt

```bash
yay -S hydracad
```

(yay fragt ggf. nach Bestätigung zum Bauen/Installieren.)

### Wenn es noch kein AUR-Paket gibt – AUR-Paket anlegen

Damit alle per **yay** (oder `pacman` nach AUR-Einreichung) installieren können, muss jemand mit einem **AUR-Account** das Paket anlegen:

1. **AUR-Account:** https://aur.archlinux.org → Account anlegen (falls noch nicht vorhanden). Pakete werden per **Git over SSH** eingereicht; SSH-Key im AUR-Profil hinterlegen.
2. **Neues Paket anlegen:**  
   AUR → „Submit Package“ (Paketname z. B. `hydracad`). Danach per SSH klonen:
   ```bash
   git clone ssh://aur@aur.archlinux.org/hydracad.git
   cd hydracad
   ```
   Beim ersten Verbinden den AUR-SSH-Fingerabdruck prüfen (offizielle Fingerprints: [AUR Wiki – Submitting packages](https://wiki.archlinux.org/title/Arch_User_Repository#Submitting_packages); siehe auch `packaging/arch/README.md`).
3. **PKGBUILD und .SRCINFO einbringen:**  
   In dieses `hydracad`-Repo den Inhalt von `CAD/packaging/arch/PKGBUILD` kopieren (und ggf. anpassen). Dann:
   ```bash
   makepkg --printsrcinfo > .SRCINFO
   ```
4. **Commit & Push:**  
   PKGBUILD und .SRCINFO committen und nach AUR pushen (per AUR-Git-URL und SSH/HTTPS mit AUR-Login).
5. **Ab dann:** Jeder kann mit **yay** installieren:
   ```bash
   yay -S hydracad
   ```

**Wichtig:** Der PKGBUILD im Projekt liegt unter `packaging/arch/PKGBUILD`. Beim Anlegen des AUR-Pakets `pkgver` und ggf. `pkgrel` bei neuen Releases anpassen.

---

## 3. pacman „offiziell“ (Repos [core]/[extra]/[community])

Installation mit **nur** `pacman -S hydracad` (ohne AUR/yay) gibt es nur, wenn Hydra CAD in die **offiziellen Arch-Repositories** (z. B. [community]) aufgenommen wird. Das entscheiden **Arch Trusted User / Maintainer**; oft wird zuerst ein AUR-Paket gepflegt und bei Interesse übernommen. Dafür musst du nichts Zusätzliches im Projekt tun – der PKGBUILD und diese Doku reichen als Grundlage.

---

## Kurzfassung

- **„Mit pacman installieren“:** Lokal bauen mit `makepkg -si` in `CAD/packaging/arch` → Installation erfolgt mit pacman.
- **„Mit yay installieren“:** AUR-Paket `hydracad` anlegen (einmalig, mit AUR-Account), PKGBUILD aus diesem Repo verwenden; danach reicht `yay -S hydracad`.
