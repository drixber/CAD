#!/usr/bin/env python3
"""Create a new license key and insert into DB. Usage: python admin_create_license.py [free|trial|pro|enterprise] [expires_at]"""
import sys
import os
sys.path.insert(0, os.path.dirname(__file__))
from database import init_db, get_db
from licenses import create_license_key

def main():
    init_db()
    license_type = (sys.argv[1] or "pro").lower()
    if license_type not in ("free", "trial", "pro", "enterprise"):
        license_type = "pro"
    expires_at = sys.argv[2] if len(sys.argv) > 2 else None  # e.g. 2026-12-31T23:59:59Z
    key = create_license_key()
    now = __import__("datetime").datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
    with get_db() as conn:
        conn.execute(
            "INSERT INTO licenses (license_key, license_type, starts_at, expires_at, machine_ids, created_at) VALUES (?, ?, ?, ?, ?, ?)",
            (key, license_type, now, expires_at, "[]", now),
        )
    print(f"Created {license_type} license: {key}")
    if expires_at:
        print(f"Expires: {expires_at}")

if __name__ == "__main__":
    main()
