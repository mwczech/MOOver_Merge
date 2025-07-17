# Railway Deployment Guide - MOOver MELKENS Web Simulator

## Problem: "dockerfile does not exist"

### Rozwiązanie wykonane automatycznie:

1. **✅ Poprawiony Dockerfile** - zaktualizowane flagi npm i optymalizacje
2. **✅ Poprawiony railway.toml** - ustawione poprawne zmienne środowiskowe  
3. **✅ Dodany .railwayignore** - optymalizacja buildów
4. **✅ Backend obsługuje PORT** - już skonfigurowane w server.js

### Kroki do deploymentu z Cursor:

#### 1. Logowanie do Railway (wymagane raz)
```bash
railway login
```
*Postępuj zgodnie z instrukcjami w przeglądarce*

#### 2. Połączenie z projektem Railway
```bash
# Jeśli projekt już istnieje:
railway link

# Jeśli nowy projekt:
railway init
```

#### 3. Deploy z Cursor
```bash
# Sprawdź status
railway status

# Deploy
railway up

# Lub deploy z konkretnej gałęzi
railway up --detach
```

#### 4. Monitorowanie
```bash
# Logi w czasie rzeczywistym
railway logs

# Sprawdź serwisy
railway service

# Sprawdź domenę
railway domain
```

### Alternatywne metody deploymentu:

#### A) GitHub Integration (zalecane)
1. Połącz repository z Railway w dashboard
2. Każdy push na main będzie automatycznie deployowany
3. Railway automatycznie znajdzie Dockerfile

#### B) Railway CLI - Automatyczny deploy
```bash
# Jednorazowa konfiguracja
railway login
railway init
railway up --detach

# Kolejne deploymenty
railway up
```

### Konfiguracja Environment Variables w Railway:

W Railway dashboard dodaj:
- `NODE_ENV=production`
- `PORT=$PORT` (automatycznie)
- Inne zmienne specyficzne dla aplikacji

### Struktura projektu (zweryfikowana):
```
/
├── Dockerfile ✅               # Główny Dockerfile
├── railway.toml ✅             # Konfiguracja Railway
├── .railwayignore ✅           # Optymalizacja buildów
├── backend/
│   ├── package.json ✅         # Dependencies backend
│   └── server.js ✅            # Endpoint /api/health
├── frontend/
│   ├── package.json ✅         # Dependencies frontend
│   └── src/ ✅                 # Kod React
└── package.json ✅             # Root package.json ze skryptami
```

### Troubleshooting:

#### Problem: Build fails
- Sprawdź logi: `railway logs`
- Zweryfikuj Dockerfile syntax
- Upewnij się, że package.json istnieją w obu katalogach

#### Problem: Health check fails
- Endpoint `/api/health` jest już skonfigurowany ✅
- Sprawdź czy backend startuje na właściwym porcie

#### Problem: Timeout during build
- Użyj `railway up --detach` dla dłuższych buildów
- Railway ma timeout 10 minut na build

### Dostęp do aplikacji:
Po udanym deployu:
- Railway automatycznie wygeneruje domenę
- Sprawdź: `railway domain`
- Lub w Railway dashboard > project > Deployments

### Komendy przydatne w Cursor:

```bash
# Szybki deploy
railway up --detach

# Status i logi
railway status && railway logs --tail

# Force redeploy
railway redeploy

# Otwórz w przeglądarce
railway open
```

## Status: ✅ GOTOWE DO DEPLOYMENTU

Wszystkie problemy zostały rozwiązane. Możesz teraz uruchomić deployment używając Railway CLI bezpośrednio z Cursor.