"""Auth: register, login, refresh, me. JWT + bcrypt."""
import os
import sqlite3
from datetime import datetime, timedelta
from jose import JWTError, jwt
from passlib.context import CryptContext
from fastapi import APIRouter, Depends, HTTPException, status
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials, HTTPHeader

from database import get_db, init_db
from models import RegisterRequest, LoginRequest, RefreshRequest, TokenResponse, UserResponse

SECRET_KEY = os.environ.get("HYDRACAD_JWT_SECRET", "change-me-in-production-min-32-chars")
ALGORITHM = "HS256"
ACCESS_EXPIRE_MINUTES = 60
REFRESH_EXPIRE_DAYS = 30

pwd_ctx = CryptContext(schemes=["bcrypt"], deprecated="auto")
router = APIRouter(prefix="/api/auth", tags=["auth"])
security = HTTPBearer(auto_error=False)


def hash_password(password: str) -> str:
    return pwd_ctx.hash(password)


def verify_password(plain: str, hashed: str) -> bool:
    return pwd_ctx.verify(plain, hashed)


def create_tokens(user_id: int, username: str) -> tuple[str, str]:
    now = datetime.utcnow()
    access_exp = now + timedelta(minutes=ACCESS_EXPIRE_MINUTES)
    refresh_exp = now + timedelta(days=REFRESH_EXPIRE_DAYS)
    access_payload = {"sub": str(user_id), "username": username, "type": "access", "exp": access_exp}
    refresh_payload = {"sub": str(user_id), "username": username, "type": "refresh", "exp": refresh_exp}
    access_token = jwt.encode(access_payload, SECRET_KEY, algorithm=ALGORITHM)
    refresh_token = jwt.encode(refresh_payload, SECRET_KEY, algorithm=ALGORITHM)
    return access_token, refresh_token


def decode_token(token: str) -> dict:
    return jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])


@router.post("/register", response_model=TokenResponse)
def register(req: RegisterRequest):
    if len(req.username) < 3:
        raise HTTPException(status_code=400, detail="Username must be at least 3 characters")
    if len(req.password) < 8:
        raise HTTPException(status_code=400, detail="Password must be at least 8 characters")
    now = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
    password_hash = hash_password(req.password)
    with get_db() as conn:
        try:
            cur = conn.execute(
                "INSERT INTO users (username, email, password_hash, created_at) VALUES (?, ?, ?, ?)",
                (req.username, req.email, password_hash, now),
            )
            user_id = cur.lastrowid
        except sqlite3.IntegrityError as e:
            if "username" in str(e).lower() or "UNIQUE" in str(e):
                raise HTTPException(status_code=400, detail="Username already taken")
            raise HTTPException(status_code=400, detail="Email already registered")
    access_token, refresh_token = create_tokens(user_id, req.username)
    return TokenResponse(
        access_token=access_token,
        refresh_token=refresh_token,
        expires_in=ACCESS_EXPIRE_MINUTES * 60,
    )


@router.post("/login", response_model=TokenResponse)
def login(req: LoginRequest):
    with get_db() as conn:
        row = conn.execute(
            "SELECT id, username, email, password_hash FROM users WHERE username = ? OR email = ?",
            (req.username, req.username),
        ).fetchone()
    if not row or not verify_password(req.password, row["password_hash"]):
        raise HTTPException(status_code=401, detail="Invalid username or password")
    now = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
    with get_db() as conn:
        conn.execute("UPDATE users SET last_login_at = ? WHERE id = ?", (now, row["id"]))
    access_token, refresh_token = create_tokens(row["id"], row["username"])
    return TokenResponse(
        access_token=access_token,
        refresh_token=refresh_token,
        expires_in=ACCESS_EXPIRE_MINUTES * 60,
    )


@router.post("/refresh", response_model=TokenResponse)
def refresh(req: RefreshRequest):
    try:
        payload = decode_token(req.refresh_token)
        if payload.get("type") != "refresh":
            raise HTTPException(status_code=401, detail="Invalid token type")
        user_id = int(payload["sub"])
        username = payload.get("username", "")
    except (JWTError, ValueError, KeyError):
        raise HTTPException(status_code=401, detail="Invalid or expired refresh token")
    with get_db() as conn:
        row = conn.execute("SELECT id, username FROM users WHERE id = ?", (user_id,)).fetchone()
    if not row:
        raise HTTPException(status_code=401, detail="User not found")
    access_token, refresh_token = create_tokens(user_id, row["username"])
    return TokenResponse(
        access_token=access_token,
        refresh_token=refresh_token,
        expires_in=ACCESS_EXPIRE_MINUTES * 60,
    )


def get_current_user_id(credentials: HTTPAuthorizationCredentials = Depends(security)) -> int:
    if not credentials:
        raise HTTPException(status_code=401, detail="Not authenticated")
    try:
        payload = decode_token(credentials.credentials)
        if payload.get("type") != "access":
            raise HTTPException(status_code=401, detail="Invalid token type")
        return int(payload["sub"])
    except (JWTError, ValueError, KeyError):
        raise HTTPException(status_code=401, detail="Invalid or expired token")


@router.get("/me", response_model=UserResponse)
def me(user_id: int = Depends(get_current_user_id)):
    with get_db() as conn:
        row = conn.execute("SELECT id, username, email FROM users WHERE id = ?", (user_id,)).fetchone()
    if not row:
        raise HTTPException(status_code=404, detail="User not found")
    return UserResponse(id=row["id"], username=row["username"], email=row["email"])
