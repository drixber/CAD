"""Phase 5.1: Tests for Auth – Registrierung, Login, Refresh, Logout (GET /me + Token weg)."""
import os
import sys

import pytest

# Ensure backend on path
_backend_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _backend_dir not in sys.path:
    sys.path.insert(0, _backend_dir)


def test_register(client):
    """POST /api/auth/register → User anlegen, JWT zurück."""
    r = client.post(
        "/api/auth/register",
        json={"username": "newuser", "email": "newuser@test.local", "password": "password123"},
    )
    assert r.status_code == 200
    data = r.json()
    assert "access_token" in data
    assert "refresh_token" in data
    assert data.get("token_type") == "bearer"
    assert data.get("expires_in") > 0


def test_register_short_username(client):
    """Username < 3 Zeichen → 400."""
    r = client.post(
        "/api/auth/register",
        json={"username": "ab", "email": "ab@test.local", "password": "password123"},
    )
    assert r.status_code == 400


def test_register_short_password(client):
    """Password < 8 Zeichen → 400."""
    r = client.post(
        "/api/auth/register",
        json={"username": "validuser", "email": "valid@test.local", "password": "short"},
    )
    assert r.status_code == 400


def test_register_duplicate_username(client):
    """Doppelte Registrierung gleicher Username → 400."""
    payload = {"username": "dupuser", "email": "dupuser@test.local", "password": "password123"}
    client.post("/api/auth/register", json=payload)
    r = client.post("/api/auth/register", json=payload)
    assert r.status_code == 400


def test_login(client):
    """POST /api/auth/login → JWT (Access + Refresh)."""
    client.post(
        "/api/auth/register",
        json={"username": "loginuser", "email": "loginuser@test.local", "password": "password123"},
    )
    r = client.post(
        "/api/auth/login",
        json={"username": "loginuser", "password": "password123"},
    )
    assert r.status_code == 200
    data = r.json()
    assert "access_token" in data
    assert "refresh_token" in data


def test_login_wrong_password(client):
    """Login mit falschem Passwort → 401."""
    client.post(
        "/api/auth/register",
        json={"username": "wrongpw", "email": "wrongpw@test.local", "password": "password123"},
    )
    r = client.post("/api/auth/login", json={"username": "wrongpw", "password": "wrongpassword"})
    assert r.status_code == 401


def test_me(client, auth_headers):
    """GET /api/auth/me (Bearer) → User (id, username, email)."""
    r = client.get("/api/auth/me", headers=auth_headers)
    assert r.status_code == 200
    data = r.json()
    assert "id" in data
    assert "username" in data
    assert "email" in data


def test_me_unauthorized(client):
    """GET /api/auth/me ohne Token → 401."""
    r = client.get("/api/auth/me")
    assert r.status_code == 403  # FastAPI HTTPBearer returns 403 when no credentials


def test_refresh(client):
    """POST /api/auth/refresh → neues Access-Token."""
    r_reg = client.post(
        "/api/auth/register",
        json={"username": "refuser", "email": "refuser@test.local", "password": "password123"},
    )
    assert r_reg.status_code == 200
    refresh_token = r_reg.json()["refresh_token"]
    old_access = r_reg.json()["access_token"]
    r = client.post("/api/auth/refresh", json={"refresh_token": refresh_token})
    assert r.status_code == 200
    data = r.json()
    assert "access_token" in data
    assert data["access_token"] != old_access


def test_refresh_invalid_token(client):
    """POST /api/auth/refresh mit ungültigem Token → 401."""
    r = client.post("/api/auth/refresh", json={"refresh_token": "invalid-token"})
    assert r.status_code == 401


def test_logout_behavior(client, auth_headers):
    """Logout = Token lokal löschen; erneuter Aufruf mit altem Token nicht mehr gültig wenn Server es invalidiert.
    Da wir keine Token-Revocation haben: Logout ist Client-seitig (Token weg). GET /me mit altem Token bleibt 200.
    Test: Nach „Logout“ (kein Token gesendet) → GET /me ohne Header → 403."""
    r = client.get("/api/auth/me")  # no headers = no token
    assert r.status_code == 403
