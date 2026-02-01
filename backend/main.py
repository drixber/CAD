"""Hydra CAD Auth & License API. Run: uvicorn main:app --reload."""
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from database import init_db
from auth import router as auth_router
from licenses import router as license_router
from community import router as community_router

app = FastAPI(title="Hydra CAD API", version="1.0.0")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.on_event("startup")
def startup():
    init_db()

app.include_router(auth_router)
app.include_router(license_router)
app.include_router(community_router)
