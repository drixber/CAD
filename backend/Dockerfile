# Hydra CAD API – Auth, License, Community
# Build: docker build -t hydracad-api -f backend/Dockerfile .
# Run:   docker run -d -p 8000:8000 -e HYDRACAD_JWT_SECRET="<secret>" -v hydracad-data:/data hydracad-api
FROM python:3.11-slim

WORKDIR /app

COPY backend/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY backend/ .

ENV HYDRACAD_DB_PATH=/data/hydracad.db
ENV HYDRACAD_UPLOAD_DIR=/data/uploads

VOLUME /data

EXPOSE 8000

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
