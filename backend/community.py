"""Community: feed, item detail, download, upload (Makerworld-style)."""
import os
import json
from datetime import datetime
from fastapi import APIRouter, Depends, HTTPException, Query, UploadFile, File, Form  # noqa: F401
from fastapi.responses import FileResponse

from database import get_db
from auth import get_current_user_id_optional

router = APIRouter(prefix="/api/community", tags=["community"])
UPLOAD_DIR = os.environ.get("HYDRACAD_UPLOAD_DIR", os.path.join(os.path.dirname(__file__), "uploads"))


def _now() -> str:
    return datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")


def _ensure_upload_dir():
    os.makedirs(UPLOAD_DIR, exist_ok=True)


@router.get("/feed")
def get_feed(
    sort: str = Query("new", regex="^(trending|new|popular)$"),
    q: str = Query(None),
):
    """Return list of community items (id, title, author, likes, downloads, description, created_at)."""
    order = {"new": "ci.created_at DESC", "popular": "ci.downloads DESC", "trending": "ci.likes DESC"}.get(sort, "ci.created_at DESC")
    with get_db() as conn:
        rows = conn.execute(
            f"""
            SELECT ci.id, ci.title, ci.description, ci.likes, ci.downloads, ci.created_at, u.username AS author
            FROM community_items ci
            LEFT JOIN users u ON ci.user_id = u.id
            ORDER BY {order}
            LIMIT 100
            """
        ).fetchall()
    items = []
    for row in rows:
        id_, title, desc, likes, downloads, created_at, author = row
        author = author or "anonymous"
        if q and q.lower() not in (title or "").lower() and q.lower() not in (desc or "").lower():
            continue
        items.append({
            "id": str(id_),
            "title": title,
            "author": author,
            "likes": likes or 0,
            "downloads": downloads or 0,
            "description": desc or "",
            "thumbnail_url": None,
            "created_at": created_at,
        })
    return {"items": items}


@router.get("/items/{item_id}")
def get_item(item_id: int):
    """Return single item with download URL."""
    with get_db() as conn:
        row = conn.execute(
            """
            SELECT ci.id, ci.title, ci.description, ci.file_path, ci.likes, ci.downloads, ci.created_at, u.username
            FROM community_items ci
            LEFT JOIN users u ON ci.user_id = u.id
            WHERE ci.id = ?
            """,
            (item_id,),
        ).fetchone()
    if not row:
        raise HTTPException(status_code=404, detail="Item not found")
    id_, title, desc, file_path, likes, downloads, created_at, author = row
    return {
        "id": str(id_),
        "title": title,
        "author": author or "anonymous",
        "likes": likes or 0,
        "downloads": downloads or 0,
        "description": desc or "",
        "thumbnail_url": None,
        "created_at": created_at,
        "file_url": f"/api/community/items/{id_}/download",
    }


@router.get("/items/{item_id}/download")
def download_item(item_id: int):
    """Serve file for item; increment downloads."""
    with get_db() as conn:
        row = conn.execute(
            "SELECT file_path, downloads FROM community_items WHERE id = ?", (item_id,)
        ).fetchone()
    if not row:
        raise HTTPException(status_code=404, detail="Item not found")
    file_path, downloads = row
    full_path = os.path.join(UPLOAD_DIR, file_path) if not os.path.isabs(file_path) else file_path
    if not os.path.isfile(full_path):
        raise HTTPException(status_code=404, detail="File not found")
    with get_db() as conn:
        conn.execute("UPDATE community_items SET downloads = ? WHERE id = ?", ((downloads or 0) + 1, item_id))
    return FileResponse(full_path, filename=os.path.basename(file_path))


@router.post("/items")
def upload_item(
    title: str = Form(...),
    description: str = Form(""),
    file: UploadFile = File(...),
    user_id: int | None = Depends(get_current_user_id_optional),
):
    """Upload a design (auth optional; if no auth, user_id is null)."""
    _ensure_upload_dir()
    safe_name = "".join(c if c.isalnum() or c in "._-" else "_" for c in file.filename or "upload")[:200]
    base, ext = os.path.splitext(safe_name)
    if not ext:
        ext = ".step"
    unique = f"{base}_{datetime.utcnow().strftime('%Y%m%d%H%M%S')}{ext}"
    path = os.path.join(UPLOAD_DIR, unique)
    with open(path, "wb") as f:
        f.write(file.file.read())
    rel_path = unique
    now = _now()
    with get_db() as conn:
        cur = conn.execute(
            "INSERT INTO community_items (user_id, title, description, file_path, likes, downloads, created_at) VALUES (?, ?, ?, ?, 0, 0, ?)",
            (user_id, title, description, rel_path, now),
        )
        item_id = cur.lastrowid
    return {"id": str(item_id), "title": title, "file_url": f"/api/community/items/{item_id}/download"}