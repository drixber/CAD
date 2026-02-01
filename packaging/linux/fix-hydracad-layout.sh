#!/bin/sh
# Einmal-Skript: Korrigiert die Installation, wenn die Binary fälschlich in
# /usr/bin/cad_desktop liegt statt in /usr/lib/hydracad. Führe mit sudo aus.
set -e
BINARY_SRC="/usr/bin/cad_desktop"
BINARY_DEST="/usr/lib/hydracad/cad_desktop"
WRAPPER="/usr/bin/cad_desktop"

# Nur wenn die Binary noch in /usr/bin liegt (groß, ~1.9M) und /usr/lib/hydracad fehlt
if [ -f "$BINARY_SRC" ] && [ ! -d "/usr/lib/hydracad" ]; then
  echo "[*] Erstelle /usr/lib/hydracad"
  mkdir -p /usr/lib/hydracad
  echo "[*] Verschiebe Binary nach $BINARY_DEST"
  mv "$BINARY_SRC" "$BINARY_DEST"
  echo "[*] Erstelle Wrapper in $WRAPPER"
  cat > "$WRAPPER" << 'WRAPPER_EOF'
#!/bin/sh
if [ -z "$QT_QPA_PLATFORM" ] && [ -n "$WAYLAND_DISPLAY" ]; then
  export QT_QPA_PLATFORM=xcb
fi
exec /usr/lib/hydracad/cad_desktop "$@"
WRAPPER_EOF
  chmod 755 "$WRAPPER"
  echo "[✓] Layout korrigiert. Start: /usr/bin/cad_desktop oder hydracad"
else
  echo "[i] Nichts zu tun (Layout bereits korrekt oder Binary nicht in /usr/bin)."
fi
