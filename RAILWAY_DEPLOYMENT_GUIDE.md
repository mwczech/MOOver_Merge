# 🚀 MELKENS HIL Simulator - Railway Deployment Guide

## ✅ Project Status: PRODUCTION READY

Projekt został w pełni przygotowany do bezproblemowego deploymentu na Railway. Wszystkie wymagane pliki są gotowe i przetestowane.

## 📁 Przygotowana Struktura Projektu

```
web-simulator/                 # ← Katalog główny do deploymentu
├── 🐳 Dockerfile             # Multi-stage build (Node.js + Python)
├── 🚂 railway.toml           # Konfiguracja Railway
├── 📦 package.json           # Metadata projektu
├── 🚀 start.sh               # Skrypt startowy z logowaniem
├── 🔧 .env.example          # Przykładowe zmienne środowiskowe
├── 🚫 .dockerignore         # Exclusions dla Docker build
├── backend/                  # Python FastAPI backend
│   ├── main.py              # ✅ Skonfigurowany PORT z env variable
│   ├── scenario_runner.py   # Framework do fault injection
│   ├── requirements.txt     # Python dependencies
│   └── scenarios/           # Przykładowe scenariusze testowe
└── frontend/                # Statyczne pliki frontendu
    ├── index.html           # Dashboard
    ├── style.css            # Style
    └── main.js             # JavaScript
```

## 🚂 Deployment na Railway - Instrukcja Krok po Krok

### Metoda 1: GitHub Integration (Zalecana)

1. **Przygotowanie repozytorium**:
   ```bash
   # Przejdź do katalogu głównego projektu
   cd /workspace
   
   # Dodaj zmiany do git
   git add .
   git commit -m "🚀 Railway deployment ready - MELKENS HIL Simulator"
   git push origin main
   ```

2. **Deployment na Railway**:
   - Przejdź na [railway.app](https://railway.app)
   - Zaloguj się przez GitHub
   - Kliknij "Start a New Project"
   - Wybierz "Deploy from GitHub repo"
   - Wybierz swoje repozytorium
   - **WAŻNE**: Ustaw **Root Directory** na `web-simulator`
   - Railway automatycznie wykryje `Dockerfile` i `railway.toml`

### Metoda 2: Railway CLI

```bash
# Zainstaluj Railway CLI
npm install -g @railway/cli

# Zaloguj się
railway login

# Przejdź do katalogu web-simulator
cd web-simulator

# Zainicjalizuj projekt Railway
railway init

# Deploy aplikację
railway up

# Ustaw custom domain (opcjonalnie)
railway domain
```

## ⚙️ Konfiguracja Techniczna

### 🔧 Zmienne Środowiskowe (automatycznie ustawione)

| Zmienna | Wartość | Opis |
|---------|---------|------|
| `PORT` | `3000` | Port aplikacji (Railway ustawi automatycznie) |
| `PYTHONUNBUFFERED` | `1` | Wyłącza buforowanie Python output |
| `PYTHONPATH` | `/app/backend` | Ścieżka modułów Python |

### 🐳 Dockerfile - Multi-stage Build

```dockerfile
# Stage 1: Frontend (Node.js)
FROM node:18-alpine AS frontend-builder
# Przygotowanie statycznych plików frontend

# Stage 2: Backend (Python)
FROM python:3.11-slim
# Instalacja Python dependencies
# Skopiowanie backend + frontend
# Konfiguracja portów i health check
```

### 🚀 Automatyczne Features

- **Health Check**: `/api/health` endpoint
- **Auto-restart**: w przypadku błędów
- **Port Management**: automatyczne przypisanie przez Railway
- **CORS**: skonfigurowany dla production
- **Static Files**: frontend serwowany przez FastAPI
- **Logging**: kompletne logowanie startowe

## 🔍 Weryfikacja Deploymentu

### 1. Health Check
```bash
curl https://your-app-name.railway.app/api/health
```

Oczekiwana odpowiedź:
```json
{
  "status": "healthy",
  "timestamp": "2024-01-15T10:30:00",
  "service": "MELKENS HIL Simulator",
  "version": "1.0"
}
```

### 2. Frontend Dashboard
- Otwórz `https://your-app-name.railway.app`
- Powinien załadować się dashboard z interfejsem HIL simulatora

### 3. API Endpoints
```bash
# Status systemu
curl https://your-app-name.railway.app/api/status

# Lista scenariuszy
curl https://your-app-name.railway.app/api/scenarios

# WebSocket (test w przeglądarce)
ws://your-app-name.railway.app/ws
```

## 🎯 Kluczowe Zmiany w Kodzie

### 1. main.py - PORT z Environment Variable
```python
# Automatyczne wykrywanie portu Railway
port = int(os.environ.get("PORT", 8000))
uvicorn.run("main:app", host="0.0.0.0", port=port, reload=False)
```

### 2. Static Files - Flexible Path
```python
# Obsługa zarówno dev jak i production
frontend_path = Path(__file__).parent.parent / "frontend"
if not frontend_path.exists():
    frontend_path = Path("/app/frontend")  # Docker path
```

### 3. Health Check Endpoint
```python
@app.get("/api/health")
async def health_check():
    return {"status": "healthy", "service": "MELKENS HIL Simulator"}
```

## 🚨 Troubleshooting

### Problem: Build Fails
**Rozwiązanie**: 
- Sprawdź Railway logs w dashboard
- Upewnij się że `Root Directory` = `web-simulator`
- Zweryfikuj czy `Dockerfile` jest poprawny

### Problem: Port Error
**Rozwiązanie**:
- Railway automatycznie ustawi zmienną `PORT`
- Kod już jest przygotowany na dynamiczny port

### Problem: Frontend nie ładuje się
**Rozwiązanie**:
- Sprawdź czy pliki frontend są w `/app/frontend` w kontenerze
- Endpoint `/` powinien serwować `index.html`

### Problem: API nie odpowiada
**Rozwiązanie**:
- Sprawdź health check: `/api/health`
- Sprawdź logi Railway w dashboard
- Zweryfikuj czy Python dependencies są zainstalowane

## 📋 Checklist przed Deploymentem

- [✅] `Dockerfile` gotowy i przetestowany
- [✅] `railway.toml` skonfigurowany
- [✅] `start.sh` executable i gotowy
- [✅] `main.py` obsługuje zmienną `PORT`
- [✅] Frontend path jest flexible
- [✅] Health check endpoint gotowy
- [✅] Requirements.txt kompletny
- [✅] .dockerignore wyklucha niepotrzebne pliki
- [✅] CORS skonfigurowany dla production

## 🎉 Final Deployment Commands

```bash
# W katalogu głównego projektu (/workspace)
cd web-simulator

# Sprawdź czy wszystkie pliki są gotowe
ls -la Dockerfile railway.toml package.json start.sh

# Deploy przez Railway CLI
railway login
railway init
railway up

# LUB commit to GitHub i użyj GitHub integration w Railway
```

## 🔗 Po Deploymencie

1. **URL aplikacji**: Railway poda publiczny URL
2. **Custom domain**: Możesz ustawić własną domenę
3. **Monitoring**: Railway dashboard pokaże metryki i logi
4. **Scaling**: Automatyczne skalowanie w zależności od ruchu
5. **Updates**: Push do GitHub = automatyczny redeploy

---

## ✨ Status: GOTOWE DO PRODUKCJI

**🚀 Projekt jest w 100% przygotowany do bezproblemowego deploymentu na Railway!**

Wszystkie wymagania zostały spełnione:
- ✅ Multi-stage Dockerfile
- ✅ Dinamiczny PORT management
- ✅ Frontend i backend w jednej aplikacji
- ✅ Health checks i monitoring
- ✅ Przykładowe .env i dokumentacja
- ✅ Railway.toml konfiguracja
- ✅ Kompletna struktura projektu w web-simulator/

**Następny krok**: Push do GitHub i deploy przez Railway dashboard!