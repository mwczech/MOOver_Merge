#!/bin/bash
# MOOver MELKENS Local Simulator - Quick Install Script
# Supports Ubuntu/Debian, CentOS/RHEL, Arch Linux

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect OS
detect_os() {
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        OS=$NAME
        DISTRO=$ID
    else
        print_error "Cannot detect OS. Please install manually."
        exit 1
    fi
    
    print_status "Detected OS: $OS"
}

# Install Python and dependencies
install_python() {
    print_status "Installing Python and dependencies..."
    
    case $DISTRO in
        "ubuntu"|"debian")
            sudo apt update
            sudo apt install -y python3 python3-pip python3-venv git
            ;;
        "centos"|"rhel"|"fedora")
            sudo yum install -y python3 python3-pip git || sudo dnf install -y python3 python3-pip git
            ;;
        "arch"|"manjaro")
            sudo pacman -S --noconfirm python python-pip git
            ;;
        *)
            print_warning "Unsupported distribution. Please install Python 3.8+ manually."
            ;;
    esac
    
    print_success "Python installed"
}

# Setup USB permissions for IMU
setup_usb_permissions() {
    print_status "Setting up USB permissions for IMU access..."
    
    # Add user to dialout group
    sudo usermod -a -G dialout $USER
    
    # Create udev rule for STM32 devices
    sudo tee /etc/udev/rules.d/99-stm32.rules > /dev/null <<EOF
# STM32 devices
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="5740", MODE="0666", GROUP="dialout"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", MODE="0666", GROUP="dialout"
# Generic USB-Serial converters
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", MODE="0666", GROUP="dialout"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", MODE="0666", GROUP="dialout"
EOF

    # Reload udev rules
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    
    print_success "USB permissions configured"
    print_warning "Please log out and log back in for group changes to take effect"
}

# Install Python packages
install_python_packages() {
    print_status "Installing Python packages..."
    
    # Create virtual environment
    python3 -m venv venv
    source venv/bin/activate
    
    # Upgrade pip
    pip install --upgrade pip
    
    # Install requirements
    pip install -r requirements.txt
    
    print_success "Python packages installed"
}

# Create desktop shortcut
create_desktop_shortcut() {
    print_status "Creating desktop shortcut..."
    
    DESKTOP_FILE="$HOME/Desktop/MOOver-Local-Simulator.desktop"
    CURRENT_DIR=$(pwd)
    
    cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=MOOver MELKENS Local Simulator
Comment=Local simulator with real IMU support
Exec=gnome-terminal --working-directory="$CURRENT_DIR" -- bash -c "source venv/bin/activate && python start_local_simulator.py; exec bash"
Icon=$CURRENT_DIR/icon.png
Terminal=true
Categories=Development;Engineering;
EOF

    chmod +x "$DESKTOP_FILE"
    
    print_success "Desktop shortcut created"
}

# Test installation
test_installation() {
    print_status "Testing installation..."
    
    source venv/bin/activate
    
    # Test Python imports
    if python -c "import flask, serial, numpy, requests, git; print('All imports successful')"; then
        print_success "Python dependencies OK"
    else
        print_error "Python dependency test failed"
        return 1
    fi
    
    # Test USB device detection
    if python start_local_simulator.py devices &>/dev/null; then
        print_success "Device detection working"
    else
        print_warning "Device detection test failed (this may be normal if no IMU is connected)"
    fi
    
    print_success "Installation test completed"
}

# Main installation process
main() {
    echo "🤖 MOOver MELKENS Local Simulator - Installation Script"
    echo "=" * 60
    
    # Check if running as root
    if [[ $EUID -eq 0 ]]; then
        print_error "Please don't run this script as root"
        exit 1
    fi
    
    # Check if we're in the right directory
    if [[ ! -f "requirements.txt" ]]; then
        print_error "Please run this script from the local-simulator directory"
        exit 1
    fi
    
    detect_os
    
    # Ask for confirmation
    echo
    print_status "This script will:"
    echo "  - Install Python 3 and required packages"
    echo "  - Set up USB permissions for IMU devices"
    echo "  - Create a Python virtual environment"
    echo "  - Install all dependencies"
    echo "  - Create a desktop shortcut"
    echo
    read -p "Continue? [y/N] " -n 1 -r
    echo
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_status "Installation cancelled"
        exit 0
    fi
    
    # Run installation steps
    install_python
    setup_usb_permissions
    install_python_packages
    create_desktop_shortcut
    test_installation
    
    echo
    print_success "🎉 Installation completed successfully!"
    echo
    print_status "Next steps:"
    echo "  1. Log out and log back in (for USB permissions)"
    echo "  2. Connect your IMU device via USB"
    echo "  3. Run: source venv/bin/activate"
    echo "  4. Run: python start_local_simulator.py setup"
    echo "  5. Run: python start_local_simulator.py run"
    echo
    print_status "Or use the desktop shortcut: MOOver MELKENS Local Simulator"
    echo
}

# Handle script interruption
trap 'print_error "Installation interrupted"; exit 1' INT TERM

# Run main function
main "$@"