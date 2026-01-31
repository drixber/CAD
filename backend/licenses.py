"""License: activate, check. Free/Trial/Pro/Enterprise."""
import os
import json
import uuid
from datetime import datetime
from fastapi import APIRouter, Depends

from database import get_db, get_connection
from models import LicenseActivateRequest, LicenseCheckRequest, LicenseStatusResponse
from auth import get_current_user_id

router = APIRouter(prefix="/api/license", tags=["license"])
MAX_MACHINES_PER_USER = 5


def _now() -> str:
    return datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")


@router.post("/activate", response_model=LicenseStatusResponse)
def activate(req: LicenseActivateRequest, user_id: int = Depends(get_current_user_id)):
    """Activate a license key for this user and machine_id."""
    with get_db() as conn:
        row = conn.execute(
            "SELECT id, user_id, license_type, starts_at, expires_at, machine_ids FROM licenses WHERE license_key = ?",
            (req.license_key.strip(),),
        ).fetchone()
    if not row:
        return LicenseStatusResponse(valid=False, error="Invalid license key")
    lic_id, lic_user_id, lic_type, starts_at, expires_at, machine_ids_json = row
    if lic_user_id is not None and lic_user_id != user_id:
        return LicenseStatusResponse(valid=False, error="License already assigned to another user")
    now = _now()
    if expires_at and expires_at < now:
        return LicenseStatusResponse(valid=False, error="License expired", expires_at=expires_at)
    machine_ids = json.loads(machine_ids_json) if machine_ids_json else []
    if req.machine_id in machine_ids:
        return LicenseStatusResponse(valid=True, license_type=lic_type, expires_at=expires_at)
    if len(machine_ids) >= MAX_MACHINES_PER_USER:
        return LicenseStatusResponse(valid=False, error=f"Maximum {MAX_MACHINES_PER_USER} machines per license")
    machine_ids.append(req.machine_id)
    with get_db() as conn:
        conn.execute(
            "UPDATE licenses SET user_id = ?, machine_ids = ?, starts_at = ? WHERE id = ?",
            (user_id, json.dumps(machine_ids), now, lic_id),
        )
    return LicenseStatusResponse(valid=True, license_type=lic_type, expires_at=expires_at)


@router.post("/check", response_model=LicenseStatusResponse)
def check(req: LicenseCheckRequest, user_id: int = Depends(get_current_user_id)):
    """Check license status for this user and machine_id."""
    with get_db() as conn:
        rows = conn.execute(
            "SELECT license_type, expires_at, machine_ids FROM licenses WHERE user_id = ?",
            (user_id,),
        ).fetchall()
    now = _now()
    for row in rows:
        lic_type, expires_at, machine_ids_json = row
        if expires_at and expires_at < now:
            continue
        machine_ids = json.loads(machine_ids_json) if machine_ids_json else []
        if req.machine_id in machine_ids or len(machine_ids) == 0:
            return LicenseStatusResponse(valid=True, license_type=lic_type, expires_at=expires_at)
    return LicenseStatusResponse(valid=False, error="No valid license for this machine")


def create_license_key() -> str:
    """Generate a new license key (e.g. for admin script)."""
    return str(uuid.uuid4()).replace("-", "").upper()


def add_license(license_type: str, expires_at: str | None = None) -> str:
    """Insert a new license into DB; returns the new license_key."""
    key = create_license_key()
    now = _now()
    with get_db() as conn:
        conn.execute(
            "INSERT INTO licenses (license_key, license_type, starts_at, expires_at, machine_ids, created_at) VALUES (?, ?, ?, ?, ?, ?)",
            (key, license_type, now, expires_at or None, "[]", now),
        )
    return key
