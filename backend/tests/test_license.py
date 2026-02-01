"""Phase 5.2: Tests for License – Aktivieren, Prüfen, Offline-Verhalten (Cache in App; hier nur API)."""
import os
import sys

import pytest

_backend_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _backend_dir not in sys.path:
    sys.path.insert(0, _backend_dir)

from licenses import add_license


@pytest.fixture
def license_key():
    """Insert a new pro license and return its key."""
    return add_license("pro")


@pytest.fixture
def license_key_expired():
    """Insert a license that is already expired."""
    return add_license("trial", "2020-01-01T00:00:00Z")


def test_activate(client, auth_headers, license_key):
    """POST /api/license/activate (Bearer, license_key, machine_id) → valid True."""
    r = client.post(
        "/api/license/activate",
        headers=auth_headers,
        json={"license_key": license_key, "machine_id": "machine-test-1"},
    )
    assert r.status_code == 200
    data = r.json()
    assert data["valid"] is True
    assert data.get("license_type") == "pro"


def test_activate_invalid_key(client, auth_headers):
    """POST /api/license/activate mit ungültigem Schlüssel → valid False, error."""
    r = client.post(
        "/api/license/activate",
        headers=auth_headers,
        json={"license_key": "INVALID-KEY-12345", "machine_id": "machine-1"},
    )
    assert r.status_code == 200
    data = r.json()
    assert data["valid"] is False
    assert "error" in data


def test_check_after_activate(client, auth_headers, license_key):
    """Nach Aktivierung: POST /api/license/check → valid True."""
    client.post(
        "/api/license/activate",
        headers=auth_headers,
        json={"license_key": license_key, "machine_id": "machine-check-1"},
    )
    r = client.post(
        "/api/license/check",
        headers=auth_headers,
        json={"machine_id": "machine-check-1"},
    )
    assert r.status_code == 200
    data = r.json()
    assert data["valid"] is True
    assert data.get("license_type") == "pro"


def test_check_no_license(client, auth_headers):
    """POST /api/license/check ohne vorherige Aktivierung → valid False."""
    r = client.post(
        "/api/license/check",
        headers=auth_headers,
        json={"machine_id": "machine-no-license"},
    )
    assert r.status_code == 200
    data = r.json()
    assert data["valid"] is False


def test_activate_expired_license(client, auth_headers, license_key_expired):
    """Aktivierung einer abgelaufenen Lizenz → valid False, error."""
    r = client.post(
        "/api/license/activate",
        headers=auth_headers,
        json={"license_key": license_key_expired, "machine_id": "machine-expired"},
    )
    assert r.status_code == 200
    data = r.json()
    assert data["valid"] is False
    assert "error" in data


def test_activate_requires_auth(client, license_key):
    """POST /api/license/activate ohne Bearer → 401."""
    r = client.post(
        "/api/license/activate",
        json={"license_key": license_key, "machine_id": "machine-1"},
    )
    assert r.status_code == 403  # FastAPI HTTPBearer: no credentials → 403


def test_check_requires_auth(client):
    """POST /api/license/check ohne Bearer → 403."""
    r = client.post(
        "/api/license/check",
        json={"machine_id": "machine-1"},
    )
    assert r.status_code == 403
