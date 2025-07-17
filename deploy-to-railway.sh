#!/bin/bash

echo "🚂 MOOver MELKENS Web Simulator - Railway Deployment Script"
echo "========================================================="

# Sprawdź czy Railway CLI jest zainstalowane
if ! command -v railway &> /dev/null; then
    echo "❌ Railway CLI nie jest zainstalowane!"
    echo "📦 Instaluję Railway CLI..."
    npm install -g @railway/cli
    if [ $? -ne 0 ]; then
        echo "❌ Nie udało się zainstalować Railway CLI"
        exit 1
    fi
fi

echo "✅ Railway CLI wersja: $(railway --version)"

# Sprawdź czy jesteś zalogowany
if ! railway status &> /dev/null; then
    echo "🔐 Musisz się zalogować do Railway..."
    echo "📋 Uruchom: railway login"
    echo "   Następnie wróć i uruchom ten skrypt ponownie"
    exit 1
fi

echo "✅ Zalogowany do Railway"

# Sprawdź czy projekt jest połączony
echo "🔗 Sprawdzam połączenie z projektem..."
if ! railway status | grep -q "Project:"; then
    echo "❓ Projekt nie jest połączony z Railway"
    echo "🆕 Chcesz utworzyć nowy projekt czy połączyć istniejący?"
    echo "   Dla nowego: railway init"
    echo "   Dla istniejącego: railway link"
    exit 1
fi

echo "✅ Projekt połączony z Railway"

# Sprawdź czy Dockerfile istnieje
if [ ! -f "Dockerfile" ]; then
    echo "❌ Dockerfile nie został znaleziony!"
    exit 1
fi

echo "✅ Dockerfile znaleziony"

# Sprawdź czy railway.toml istnieje
if [ ! -f "railway.toml" ]; then
    echo "❌ railway.toml nie został znaleziony!"
    exit 1
fi

echo "✅ railway.toml znaleziony"

# Sprawdź strukturę katalogów
if [ ! -d "backend" ] || [ ! -d "frontend" ]; then
    echo "❌ Nie znaleziono katalogów backend i/lub frontend!"
    exit 1
fi

echo "✅ Struktura katalogów poprawna"

# Deploy
echo "🚀 Rozpoczynam deployment..."
echo "⏳ To może potrwać kilka minut..."

railway up --detach

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 Deployment zakończony pomyślnie!"
    echo "🌐 Sprawdź domenę: railway domain"
    echo "📊 Logi: railway logs"
    echo "📱 Otwórz aplikację: railway open"
else
    echo ""
    echo "❌ Deployment nie powiódł się!"
    echo "📋 Sprawdź logi: railway logs"
    echo "🔧 Debug: railway status"
fi