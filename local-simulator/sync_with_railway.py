#!/usr/bin/env python3
"""
Synchronization script for Local Simulator <-> Railway Web App
Handles route sync, configuration updates, and deployment coordination
"""

import json
import os
import requests
import git
import logging
from datetime import datetime
from typing import Dict, List, Optional
import argparse
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class RailwaySync:
    """Handles synchronization between local simulator and Railway deployment"""
    
    def __init__(self, config_file: str = "sync_config.json"):
        self.config = self.load_config(config_file)
        self.git_repo = None
        self.setup_git()
        
    def load_config(self, config_file: str) -> Dict:
        """Load synchronization configuration"""
        default_config = {
            "railway_url": "https://web-production-xxxx.up.railway.app",
            "git_remote": "origin",
            "git_branch": "main",
            "routes_path": "routes/",
            "backup_path": "backups/",
            "auto_commit": True,
            "sync_interval": 300,  # 5 minutes
            "api_endpoints": {
                "routes": "/api/routes",
                "upload_route": "/api/upload_route",
                "status": "/api/status"
            }
        }
        
        try:
            with open(config_file, 'r') as f:
                user_config = json.load(f)
                default_config.update(user_config)
        except FileNotFoundError:
            logger.info(f"Config file {config_file} not found, creating default")
            self.save_config(default_config, config_file)
            
        return default_config
    
    def save_config(self, config: Dict, config_file: str):
        """Save configuration to file"""
        with open(config_file, 'w') as f:
            json.dump(config, f, indent=2)
    
    def setup_git(self):
        """Initialize Git repository connection"""
        try:
            self.git_repo = git.Repo('.')
            logger.info(f"Connected to Git repository: {self.git_repo.git_dir}")
        except git.InvalidGitRepositoryError:
            logger.error("Not in a Git repository. Please run from project root.")
            raise
    
    def pull_latest_changes(self) -> bool:
        """Pull latest changes from remote repository"""
        try:
            origin = self.git_repo.remotes[self.config["git_remote"]]
            origin.pull(self.config["git_branch"])
            logger.info("Successfully pulled latest changes")
            return True
        except Exception as e:
            logger.error(f"Failed to pull changes: {e}")
            return False
    
    def push_changes(self, message: str = None) -> bool:
        """Push local changes to remote repository"""
        try:
            if not message:
                message = f"Auto-sync local simulator data - {datetime.now().isoformat()}"
                
            # Add all changed files
            self.git_repo.git.add('.')
            
            # Check if there are changes to commit
            if self.git_repo.is_dirty():
                self.git_repo.index.commit(message)
                origin = self.git_repo.remotes[self.config["git_remote"]]
                origin.push(self.config["git_branch"])
                logger.info(f"Successfully pushed changes: {message}")
                return True
            else:
                logger.info("No changes to push")
                return True
                
        except Exception as e:
            logger.error(f"Failed to push changes: {e}")
            return False
    
    def download_routes_from_railway(self) -> Dict:
        """Download current routes from Railway deployment"""
        try:
            url = self.config["railway_url"] + self.config["api_endpoints"]["routes"]
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            
            routes = response.json()
            logger.info(f"Downloaded {len(routes)} routes from Railway")
            return routes
            
        except requests.RequestException as e:
            logger.error(f"Failed to download routes from Railway: {e}")
            return {}
    
    def upload_routes_to_railway(self, routes: Dict) -> bool:
        """Upload routes to Railway deployment"""
        try:
            url = self.config["railway_url"] + self.config["api_endpoints"]["upload_route"]
            
            for route_name, route_data in routes.items():
                payload = {
                    "name": route_name,
                    "data": route_data
                }
                
                response = requests.post(url, json=payload, timeout=10)
                response.raise_for_status()
                logger.info(f"Uploaded route {route_name} to Railway")
                
            return True
            
        except requests.RequestException as e:
            logger.error(f"Failed to upload routes to Railway: {e}")
            return False
    
    def load_local_routes(self) -> Dict:
        """Load routes from local files"""
        routes = {}
        routes_dir = Path(self.config["routes_path"])
        
        if not routes_dir.exists():
            routes_dir.mkdir(parents=True, exist_ok=True)
            return routes
            
        for route_file in routes_dir.glob("*.json"):
            try:
                with open(route_file, 'r') as f:
                    route_data = json.load(f)
                    route_name = route_file.stem
                    routes[route_name] = route_data
                    logger.debug(f"Loaded local route: {route_name}")
            except Exception as e:
                logger.error(f"Failed to load route {route_file}: {e}")
                
        return routes
    
    def save_local_routes(self, routes: Dict):
        """Save routes to local files"""
        routes_dir = Path(self.config["routes_path"])
        routes_dir.mkdir(parents=True, exist_ok=True)
        
        for route_name, route_data in routes.items():
            route_file = routes_dir / f"{route_name}.json"
            try:
                with open(route_file, 'w') as f:
                    json.dump(route_data, f, indent=2)
                logger.debug(f"Saved local route: {route_name}")
            except Exception as e:
                logger.error(f"Failed to save route {route_name}: {e}")
    
    def backup_routes(self, routes: Dict, suffix: str = None):
        """Create backup of routes"""
        if not suffix:
            suffix = datetime.now().strftime("%Y%m%d_%H%M%S")
            
        backup_dir = Path(self.config["backup_path"])
        backup_dir.mkdir(parents=True, exist_ok=True)
        
        backup_file = backup_dir / f"routes_backup_{suffix}.json"
        
        try:
            with open(backup_file, 'w') as f:
                json.dump(routes, f, indent=2)
            logger.info(f"Created backup: {backup_file}")
        except Exception as e:
            logger.error(f"Failed to create backup: {e}")
    
    def sync_routes_to_railway(self) -> bool:
        """Sync local routes to Railway deployment"""
        logger.info("Starting route sync to Railway...")
        
        # Load local routes
        local_routes = self.load_local_routes()
        if not local_routes:
            logger.warning("No local routes found")
            return False
            
        # Create backup
        self.backup_routes(local_routes, "before_railway_sync")
        
        # Upload to Railway
        success = self.upload_routes_to_railway(local_routes)
        
        if success and self.config["auto_commit"]:
            commit_msg = f"Sync routes to Railway: {list(local_routes.keys())}"
            self.push_changes(commit_msg)
            
        return success
    
    def sync_routes_from_railway(self) -> bool:
        """Sync routes from Railway to local"""
        logger.info("Starting route sync from Railway...")
        
        # Backup current local routes
        local_routes = self.load_local_routes()
        if local_routes:
            self.backup_routes(local_routes, "before_railway_download")
        
        # Download from Railway
        railway_routes = self.download_routes_from_railway()
        if not railway_routes:
            logger.warning("No routes downloaded from Railway")
            return False
            
        # Save locally
        self.save_local_routes(railway_routes)
        
        if self.config["auto_commit"]:
            commit_msg = f"Sync routes from Railway: {list(railway_routes.keys())}"
            self.push_changes(commit_msg)
            
        return True
    
    def bidirectional_sync(self) -> bool:
        """Perform bidirectional synchronization"""
        logger.info("Starting bidirectional sync...")
        
        # Pull latest Git changes first
        if not self.pull_latest_changes():
            logger.error("Failed to pull latest changes")
            return False
        
        # Load both local and Railway routes
        local_routes = self.load_local_routes()
        railway_routes = self.download_routes_from_railway()
        
        # Create comprehensive backup
        all_routes = {**local_routes, **railway_routes}
        self.backup_routes(all_routes, "bidirectional_sync")
        
        # Merge routes (Railway takes precedence for conflicts)
        merged_routes = {**local_routes, **railway_routes}
        
        # Update both sides
        self.save_local_routes(merged_routes)
        success = self.upload_routes_to_railway(merged_routes)
        
        if success and self.config["auto_commit"]:
            commit_msg = f"Bidirectional sync complete: {len(merged_routes)} routes"
            self.push_changes(commit_msg)
            
        return success
    
    def status_check(self) -> Dict:
        """Check status of both local and Railway systems"""
        status = {
            "local": {"routes": 0, "git_status": "unknown"},
            "railway": {"status": "unknown", "routes": 0},
            "sync_needed": False
        }
        
        # Check local status
        local_routes = self.load_local_routes()
        status["local"]["routes"] = len(local_routes)
        
        try:
            if self.git_repo.is_dirty():
                status["local"]["git_status"] = "dirty"
            else:
                status["local"]["git_status"] = "clean"
        except:
            status["local"]["git_status"] = "error"
        
        # Check Railway status
        try:
            url = self.config["railway_url"] + self.config["api_endpoints"]["status"]
            response = requests.get(url, timeout=5)
            if response.status_code == 200:
                status["railway"]["status"] = "online"
                
                # Get Railway routes
                railway_routes = self.download_routes_from_railway()
                status["railway"]["routes"] = len(railway_routes)
                
                # Check if sync needed
                if set(local_routes.keys()) != set(railway_routes.keys()):
                    status["sync_needed"] = True
                    
        except:
            status["railway"]["status"] = "offline"
        
        return status

def main():
    """Main CLI interface"""
    parser = argparse.ArgumentParser(description="MOOver MELKENS Railway Sync Tool")
    parser.add_argument("action", choices=[
        "to-railway", "from-railway", "bidirectional", 
        "status", "pull", "push"
    ], help="Synchronization action")
    parser.add_argument("--config", default="sync_config.json", 
                       help="Configuration file path")
    parser.add_argument("--message", help="Commit message for push action")
    
    args = parser.parse_args()
    
    try:
        sync_tool = RailwaySync(args.config)
        
        if args.action == "to-railway":
            success = sync_tool.sync_routes_to_railway()
            print("✅ Routes synced to Railway" if success else "❌ Sync to Railway failed")
            
        elif args.action == "from-railway":
            success = sync_tool.sync_routes_from_railway()
            print("✅ Routes synced from Railway" if success else "❌ Sync from Railway failed")
            
        elif args.action == "bidirectional":
            success = sync_tool.bidirectional_sync()
            print("✅ Bidirectional sync complete" if success else "❌ Bidirectional sync failed")
            
        elif args.action == "status":
            status = sync_tool.status_check()
            print(f"📊 Local: {status['local']['routes']} routes, Git: {status['local']['git_status']}")
            print(f"🌐 Railway: {status['railway']['status']}, {status['railway']['routes']} routes")
            print(f"🔄 Sync needed: {'Yes' if status['sync_needed'] else 'No'}")
            
        elif args.action == "pull":
            success = sync_tool.pull_latest_changes()
            print("✅ Git pull successful" if success else "❌ Git pull failed")
            
        elif args.action == "push":
            success = sync_tool.push_changes(args.message)
            print("✅ Git push successful" if success else "❌ Git push failed")
            
    except Exception as e:
        logger.error(f"Error: {e}")
        print(f"❌ Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())