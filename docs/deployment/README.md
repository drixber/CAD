# Deployment – Hydra CAD Backend (Phase 5)

Artefakte für das Produktions-Deployment des Auth/Lizenz/Community-Backends.  
Vollständige Anleitung: [PHASE5_TESTS_DEPLOYMENT.md](../PHASE5_TESTS_DEPLOYMENT.md).

| Datei | Zweck |
|-------|--------|
| `hydracad-api.service.example` | systemd-Unit; nach `/etc/systemd/system/hydracad-api.service` kopieren, User/Rechte anlegen, JWT-Secret setzen. |
| `nginx-api.example.conf` | nginx Reverse-Proxy mit HTTPS; Zertifikat (z. B. Let's Encrypt) vorher anlegen. |

**Docker:** Siehe [backend/Dockerfile](../../backend/Dockerfile). Build vom Repo-Root:  
`docker build -t hydracad-api -f backend/Dockerfile .`  
Run:  
`docker run -d -p 8000:8000 -e HYDRACAD_JWT_SECRET="<secret>" -v hydracad-data:/data hydracad-api`

**App für Produktion:** API-URL setzen (`CAD_API_BASE_URL` oder in Einstellungen). Siehe PHASE5_TESTS_DEPLOYMENT.md § 5.
