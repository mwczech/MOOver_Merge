# 🤖 MOOver MELKENS Web Simulator

A comprehensive web-based simulator for the MOOver MELKENS autonomous robot with Hardware-in-the-Loop (HIL) integration capabilities.

## 🚀 Quick Deploy to Railway.app

1. **Push this project to GitHub**
2. **Connect to Railway**: Go to [railway.app](https://railway.app) → New Project → Deploy from GitHub  
3. **Set Root Directory**: In Railway Settings → Root Directory → `web-simulator/`
4. **Deploy**: Railway will automatically build and deploy your application

Your app will be available at: `https://web-production-xxxx.up.railway.app`

## ✨ Features

- 🕹️ **Real-time Robot Control**: WebSocket joystick for instant movement
- 📊 **Live Monitoring**: Motor speeds, battery, current consumption, errors  
- 🗺️ **Route Management**: Autonomous navigation with routes A-K
- 🧲 **Magnetic Sensor**: Visual 16-bit magnetic field sensor array
- 🔧 **HIL Integration**: Connect real ESP32/PMB hardware for testing
- 📡 **OTA Firmware Updates**: Upload .bin/.hex files via drag-and-drop
- ⚙️ **Configuration Management**: WiFi/MQTT settings upload
- 📱 **Responsive Design**: Beautiful mobile-friendly interface

## 🐛 Troubleshooting

**❌ "Application failed to respond"**
- Check Root Directory = "web-simulator/"
- Verify health check /api/health returns 200  
- Ensure server listens on process.env.PORT

**❌ "Build failed"**  
- Ensure all package.json files exist
- Check Dockerfile syntax
- Verify frontend builds successfully

**❌ "WebSocket connection failed"**
- Check CORS allows Railway domain
- Verify Socket.IO production configuration
- Ensure PORT environment variable usage

---

**🏢 Melkens Sp. z o.o.**  
**📧 Contact**: Michał Czech  
**🔗 Project**: MOOver MELKENS Integration  
**⚡ Version**: 1.0.0
