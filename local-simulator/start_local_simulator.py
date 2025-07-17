#!/usr/bin/env python3
"""
MOOver MELKENS Local Simulator Launcher
Easy setup and configuration for local testing with IMU
"""

import argparse
import asyncio
import json
import logging
import os
import sys
from pathlib import Path
import subprocess
import serial.tools.list_ports

# Add local simulator to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from simulator import LocalSimulator
from sync_with_railway import RailwaySync

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def detect_imu_devices():
    """Detect available IMU devices"""
    print("\n🔍 Scanning for IMU devices...")
    ports = serial.tools.list_ports.comports()
    
    imu_candidates = []
    for port in ports:
        # Look for common IMU device patterns
        if any(keyword in port.description.lower() for keyword in 
               ['stm32', 'esp32', 'usb serial', 'arduino', 'uart']):
            imu_candidates.append({
                'device': port.device,
                'description': port.description,
                'manufacturer': getattr(port, 'manufacturer', 'Unknown')
            })
    
    if imu_candidates:
        print("📱 Found potential IMU devices:")
        for i, device in enumerate(imu_candidates):
            print(f"  {i+1}. {device['device']} - {device['description']}")
    else:
        print("⚠️  No IMU devices detected")
        print("   Make sure your IMU is connected via USB/Serial")
        
    return imu_candidates

def setup_environment():
    """Setup local simulator environment"""
    print("🛠️  Setting up local simulator environment...")
    
    # Create necessary directories
    directories = ['routes', 'backups', 'logs', 'templates']
    for directory in directories:
        Path(directory).mkdir(exist_ok=True)
        print(f"   ✅ Created directory: {directory}")
    
    # Check Python dependencies
    try:
        import flask, serial, numpy, requests, git
        print("   ✅ Python dependencies OK")
    except ImportError as e:
        print(f"   ❌ Missing dependency: {e}")
        print("   Run: pip install -r requirements.txt")
        return False
    
    return True

def create_default_routes():
    """Create default route files if they don't exist"""
    routes_dir = Path('routes')
    
    default_routes = {
        'route_A': {
            'name': 'Route A - Square Pattern',
            'waypoints': [
                {'x': 0, 'y': 0, 'speed': 50},
                {'x': 100, 'y': 0, 'speed': 50},
                {'x': 100, 'y': 100, 'speed': 50},
                {'x': 0, 'y': 100, 'speed': 50},
                {'x': 0, 'y': 0, 'speed': 30}
            ],
            'description': 'Basic square navigation pattern'
        },
        'route_B': {
            'name': 'Route B - Figure Eight',
            'waypoints': [
                {'x': 0, 'y': 50, 'speed': 40},
                {'x': 50, 'y': 0, 'speed': 40},
                {'x': 100, 'y': 50, 'speed': 40},
                {'x': 50, 'y': 100, 'speed': 40},
                {'x': 0, 'y': 50, 'speed': 30}
            ],
            'description': 'Figure-eight pattern for advanced testing'
        },
        'route_C': {
            'name': 'Route C - Linear Sweep',
            'waypoints': [
                {'x': 0, 'y': 0, 'speed': 60},
                {'x': 200, 'y': 0, 'speed': 60},
                {'x': 200, 'y': 20, 'speed': 30},
                {'x': 0, 'y': 20, 'speed': 60},
                {'x': 0, 'y': 40, 'speed': 30}
            ],
            'description': 'Linear sweeping pattern for area coverage'
        }
    }
    
    for route_name, route_data in default_routes.items():
        route_file = routes_dir / f"{route_name}.json"
        if not route_file.exists():
            with open(route_file, 'w') as f:
                json.dump(route_data, f, indent=2)
            print(f"   ✅ Created default route: {route_name}")

def configure_simulator():
    """Interactive configuration of simulator settings"""
    print("\n⚙️  Simulator Configuration")
    
    config = {
        'imu_port': None,
        'imu_baudrate': 115200,
        'web_port': 5000,
        'log_level': 'INFO',
        'auto_sync': True,
        'railway_url': ''
    }
    
    # IMU Configuration
    devices = detect_imu_devices()
    if devices:
        print("\nSelect IMU device:")
        for i, device in enumerate(devices):
            print(f"  {i+1}. {device['device']}")
        print(f"  {len(devices)+1}. Manual entry")
        print(f"  {len(devices)+2}. Skip (simulation mode)")
        
        try:
            choice = int(input("Choice: ")) - 1
            if 0 <= choice < len(devices):
                config['imu_port'] = devices[choice]['device']
            elif choice == len(devices):
                config['imu_port'] = input("Enter device path: ")
            else:
                config['imu_port'] = None
        except (ValueError, KeyboardInterrupt):
            config['imu_port'] = None
    
    # Railway Configuration
    railway_url = input("\nRailway app URL (optional): ").strip()
    if railway_url:
        config['railway_url'] = railway_url
    
    # Save configuration
    with open('simulator_config.json', 'w') as f:
        json.dump(config, f, indent=2)
    
    print("\n✅ Configuration saved to simulator_config.json")
    return config

def load_config():
    """Load simulator configuration"""
    try:
        with open('simulator_config.json', 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        return None

async def run_simulator(config):
    """Run the local simulator with given configuration"""
    print(f"\n🚀 Starting MOOver MELKENS Local Simulator...")
    print(f"   📱 IMU Port: {config.get('imu_port', 'Simulation Mode')}")
    print(f"   🌐 Web Interface: http://localhost:{config.get('web_port', 5000)}")
    print(f"   📡 Railway Sync: {'Enabled' if config.get('railway_url') else 'Disabled'}")
    
    simulator = LocalSimulator()
    
    try:
        await simulator.start_simulator(config.get('imu_port'))
    except KeyboardInterrupt:
        print("\n🛑 Simulator stopped by user")
    except Exception as e:
        print(f"\n❌ Simulator error: {e}")
        logger.error(f"Simulator error: {e}")

def sync_with_railway(action, config):
    """Perform Railway synchronization"""
    if not config.get('railway_url'):
        print("❌ Railway URL not configured. Run setup first.")
        return False
    
    try:
        # Update sync config with Railway URL
        sync_config = {
            'railway_url': config['railway_url'],
            'auto_commit': config.get('auto_sync', True)
        }
        
        with open('sync_config.json', 'w') as f:
            json.dump(sync_config, f, indent=2)
        
        sync_tool = RailwaySync()
        
        if action == 'status':
            status = sync_tool.status_check()
            print(f"📊 Local: {status['local']['routes']} routes")
            print(f"🌐 Railway: {status['railway']['status']}, {status['railway']['routes']} routes")
            print(f"🔄 Sync needed: {'Yes' if status['sync_needed'] else 'No'}")
            
        elif action == 'to-railway':
            success = sync_tool.sync_routes_to_railway()
            print("✅ Routes synced to Railway" if success else "❌ Sync failed")
            
        elif action == 'from-railway':
            success = sync_tool.sync_routes_from_railway()
            print("✅ Routes synced from Railway" if success else "❌ Sync failed")
            
        elif action == 'bidirectional':
            success = sync_tool.bidirectional_sync()
            print("✅ Bidirectional sync complete" if success else "❌ Sync failed")
            
        return True
        
    except Exception as e:
        print(f"❌ Sync error: {e}")
        return False

def main():
    """Main CLI interface"""
    parser = argparse.ArgumentParser(
        description="MOOver MELKENS Local Simulator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python start_local_simulator.py run              # Start simulator
  python start_local_simulator.py setup            # Configure simulator
  python start_local_simulator.py sync status      # Check sync status
  python start_local_simulator.py sync to-railway  # Upload routes to Railway
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Setup command
    setup_parser = subparsers.add_parser('setup', help='Configure simulator')
    
    # Run command
    run_parser = subparsers.add_parser('run', help='Start simulator')
    run_parser.add_argument('--config', help='Configuration file')
    run_parser.add_argument('--port', help='IMU device port')
    
    # Sync command
    sync_parser = subparsers.add_parser('sync', help='Railway synchronization')
    sync_parser.add_argument('action', choices=['status', 'to-railway', 'from-railway', 'bidirectional'])
    
    # Devices command
    devices_parser = subparsers.add_parser('devices', help='List available IMU devices')
    
    args = parser.parse_args()
    
    # Handle commands
    if args.command == 'setup':
        print("🤖 MOOver MELKENS Local Simulator Setup")
        print("=" * 50)
        
        if not setup_environment():
            return 1
            
        create_default_routes()
        config = configure_simulator()
        print("\n✅ Setup complete!")
        print("   Run 'python start_local_simulator.py run' to start the simulator")
        
    elif args.command == 'run':
        config = load_config()
        if not config:
            print("❌ No configuration found. Run 'setup' first.")
            return 1
        
        # Override with command line arguments
        if args.port:
            config['imu_port'] = args.port
            
        asyncio.run(run_simulator(config))
        
    elif args.command == 'sync':
        config = load_config()
        if not config:
            print("❌ No configuration found. Run 'setup' first.")
            return 1
            
        sync_with_railway(args.action, config)
        
    elif args.command == 'devices':
        detect_imu_devices()
        
    else:
        # No command specified - show interactive menu
        print("🤖 MOOver MELKENS Local Simulator")
        print("=" * 40)
        print("1. Setup simulator")
        print("2. Run simulator")
        print("3. Check Railway sync status")
        print("4. Sync routes to Railway")
        print("5. Sync routes from Railway")
        print("6. Detect IMU devices")
        print("0. Exit")
        
        try:
            choice = input("\nSelect option: ").strip()
            
            if choice == '1':
                setup_environment()
                create_default_routes()
                configure_simulator()
                
            elif choice == '2':
                config = load_config()
                if config:
                    asyncio.run(run_simulator(config))
                else:
                    print("❌ Run setup first")
                    
            elif choice in ['3', '4', '5']:
                config = load_config()
                if config:
                    actions = {'3': 'status', '4': 'to-railway', '5': 'from-railway'}
                    sync_with_railway(actions[choice], config)
                else:
                    print("❌ Run setup first")
                    
            elif choice == '6':
                detect_imu_devices()
                
            elif choice == '0':
                print("👋 Goodbye!")
                
        except KeyboardInterrupt:
            print("\n👋 Goodbye!")
    
    return 0

if __name__ == "__main__":
    exit(main())