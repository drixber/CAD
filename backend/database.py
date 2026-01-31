"""SQLite database and tables for users and licenses."""
import os
import sqlite3
from contextlib import contextmanager

DB_PATH = os.environ.get("HYDRACAD_DB_PATH", os.path.join(os.path.dirname(__file__), "hydracad.db"))


def get_connection():
    return sqlite3.connect(DB_PATH)


@contextmanager
def get_db():
    conn = get_connection()
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    finally:
        conn.close()


def init_db():
    """Create tables (users, licenses)."""
    with get_db() as conn:
        conn.execute("""
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                email TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                created_at TEXT NOT NULL,
                last_login_at TEXT
            )
        """)
        conn.execute("""
            CREATE TABLE IF NOT EXISTS licenses (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER,
                license_key TEXT UNIQUE NOT NULL,
                license_type TEXT NOT NULL,
                starts_at TEXT NOT NULL,
                expires_at TEXT,
                machine_ids TEXT,
                created_at TEXT NOT NULL,
                FOREIGN KEY (user_id) REFERENCES users(id)
            )
        """)
