"""Pytest config: test DB path and FastAPI TestClient. Set HYDRACAD_DB_PATH before importing app."""
import os
import sys
import tempfile

import pytest

# Use a temp DB file so all connections share the same DB (SQLite :memory: is per-connection)
_tf = tempfile.NamedTemporaryFile(suffix=".db", delete=False)
_tf.close()
os.environ["HYDRACAD_DB_PATH"] = _tf.name

# Ensure backend is on path (run from repo root: pytest backend/tests, or from backend: pytest tests)
_backend_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _backend_dir not in sys.path:
    sys.path.insert(0, _backend_dir)

from main import app
from fastapi.testclient import TestClient


@pytest.fixture
def client():
    """FastAPI TestClient using test DB."""
    return TestClient(app)


@pytest.fixture
def auth_headers(client):
    """Register a user, login, return headers with Bearer token."""
    username = f"testuser_{os.getpid()}_{id(object())}"
    email = f"{username}@test.local"
    password = "testpass123"
    r = client.post("/api/auth/register", json={"username": username, "email": email, "password": password})
    assert r.status_code == 200
    token = r.json()["access_token"]
    return {"Authorization": f"Bearer {token}"}
