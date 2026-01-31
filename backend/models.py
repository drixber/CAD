"""Pydantic models for API request/response."""
from pydantic import BaseModel
from typing import Optional


class RegisterRequest(BaseModel):
    username: str
    email: str  # validated in auth
    password: str


class LoginRequest(BaseModel):
    username: str  # or email
    password: str


class RefreshRequest(BaseModel):
    refresh_token: str


class TokenResponse(BaseModel):
    access_token: str
    refresh_token: str
    token_type: str = "bearer"
    expires_in: int


class UserResponse(BaseModel):
    id: int
    username: str
    email: str


class LicenseActivateRequest(BaseModel):
    license_key: str
    machine_id: str


class LicenseCheckRequest(BaseModel):
    machine_id: str


class LicenseStatusResponse(BaseModel):
    valid: bool
    license_type: Optional[str] = None
    expires_at: Optional[str] = None
    error: Optional[str] = None
