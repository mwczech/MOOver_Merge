# MELKENS Development Tools

This directory contains standalone development tools for the MELKENS robot system that operate independently from the web simulator.

## 🛠 Available Tools

### 📍 Route Editor (`/route-editor`)
Standalone offline route creation and editing tool.
- **Purpose**: Design routes without running the full simulator
- **Features**: Visual route editor, parameter validation, export/import capabilities
- **Export**: Compatible with both simulator and physical robot firmware
- **Usage**: `cd route-editor && npm start`

### ⚠️ Fault Injector (`/fault-injector`)
Tool for generating and testing system failures and edge cases.
- **Purpose**: Create controlled fault scenarios for testing
- **Features**: Hardware fault simulation, sensor failure patterns, timing issues
- **Integration**: Works with both simulator and hardware-in-the-loop setups
- **Usage**: `cd fault-injector && npm start`

### 🔗 Hardware Wiring (`/hardware-wiring`)
Documentation and tools for hardware-in-the-loop setup.
- **Purpose**: Complete wiring diagrams and connection guides
- **Features**: Interactive diagrams, pin mappings, protocol documentation
- **Formats**: PDF guides, interactive web viewer, configuration generators

## 🚀 Quick Start

Each tool is self-contained with its own dependencies and README:

```bash
# Install all tools
npm run install-tools

# Start route editor
npm run route-editor

# Start fault injector  
npm run fault-injector

# Generate hardware docs
npm run hardware-docs
```

## 📁 Directory Structure

```
tools/
├── route-editor/           # Standalone route creation tool
│   ├── src/               # React application
│   ├── public/            # Static assets
│   ├── package.json       # Dependencies
│   └── README.md          # Detailed usage guide
├── fault-injector/        # Fault injection and testing tool
│   ├── src/               # Node.js CLI and web interface
│   ├── scenarios/         # Pre-defined fault scenarios
│   ├── package.json       # Dependencies
│   └── README.md          # Fault injection guide
├── hardware-wiring/       # HIL setup documentation
│   ├── diagrams/          # Wiring diagrams (SVG, PDF)
│   ├── configs/           # Hardware configuration files
│   ├── protocols/         # Communication protocol docs
│   └── README.md          # Connection setup guide
└── shared/                # Shared utilities between tools
    ├── types/             # Common TypeScript types
    ├── validators/        # Route and parameter validation
    └── exporters/         # File format converters
```

## 🔧 Tool Integration

All tools are designed to work seamlessly together:

1. **Route Editor** → Creates routes → **Simulator/Hardware**
2. **Fault Injector** → Generates scenarios → **Simulator/Hardware**  
3. **Hardware Wiring** → Guides setup → **Hardware-in-the-loop**

## 📋 Development Workflow

1. **Design Phase**: Use Route Editor to create and validate routes
2. **Testing Phase**: Use Fault Injector to test edge cases and failures
3. **Integration Phase**: Use Hardware Wiring guides for physical setup
4. **Validation Phase**: Run routes on simulator and hardware

## 🎯 Key Features

- **Offline Operation**: Tools work without simulator running
- **Cross-Platform**: Windows, macOS, Linux support
- **Export Compatibility**: Routes work in simulator and firmware
- **Modular Design**: Use tools individually or combined
- **Documentation**: Complete setup and usage guides

## 📖 Documentation

Each tool includes comprehensive documentation:
- Setup and installation instructions
- Usage examples and tutorials
- Integration guides with simulator/hardware
- Troubleshooting and FAQ sections

---

For specific tool documentation, see the README.md file in each tool's directory.