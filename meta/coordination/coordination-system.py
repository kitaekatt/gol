#!/usr/bin/env python3
"""
Parallel Agent Coordination System

This module provides the core coordination logic for safe parallel agent execution.
It manages agent registration, file locking, conflict detection, and task coordination.
"""

import json
import os
import time
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple, Set
from dataclasses import dataclass
from pathlib import Path


@dataclass
class TaskMetadata:
    """Metadata for a task extracted from task file"""
    task_file: str
    mode: str
    priority: str
    parallel_safety: str
    classification: str
    estimated_duration: int
    conflicts_with: List[str]
    depends_on: List[str]
    modifies_files: List[str]
    reads_files: List[str]
    locked_resources: List[str]
    parallel_compatible: bool


@dataclass
class AgentStatus:
    """Status of an active agent"""
    agent_id: str
    mode: str
    current_task: str
    task_path: str
    status: str
    started_at: datetime
    estimated_completion: datetime
    locked_files: List[str]
    parallel_compatible: bool
    heartbeat: datetime


@dataclass
class FileLock:
    """File lock information"""
    file_path: str
    locked_by: str
    mode: str
    task: str
    lock_type: str
    acquired_at: datetime
    expires_at: datetime
    purpose: str


class CoordinationSystem:
    """Main coordination system class"""
    
    def __init__(self, base_path: str = "/mnt/c/dev/gol"):
        self.base_path = Path(base_path)
        self.coordination_path = self.base_path / "meta" / "coordination"
        self.active_agents_file = self.coordination_path / "active-agents.json"
        self.file_locks_file = self.coordination_path / "file-locks.json"
        
        # Ensure coordination directory exists
        self.coordination_path.mkdir(parents=True, exist_ok=True)
        
        # Initialize files if they don't exist
        self._init_coordination_files()
    
    def _init_coordination_files(self):
        """Initialize coordination files with default content"""
        if not self.active_agents_file.exists():
            default_agents = {
                "agents": {},
                "last_updated": datetime.now().isoformat(),
                "schema_version": "1.0",
                "coordination_enabled": True,
                "description": "Tracks active agents and their current work for parallel execution coordination"
            }
            self._save_json(self.active_agents_file, default_agents)
        
        if not self.file_locks_file.exists():
            default_locks = {
                "locks": {},
                "last_updated": datetime.now().isoformat(),
                "schema_version": "1.0",
                "lock_timeout_minutes": 60,
                "description": "Lightweight file locking system for parallel agent coordination"
            }
            self._save_json(self.file_locks_file, default_locks)
    
    def _load_json(self, file_path: Path) -> Dict:
        """Load JSON file with error handling"""
        try:
            with open(file_path, 'r') as f:
                return json.load(f)
        except (FileNotFoundError, json.JSONDecodeError) as e:
            print(f"Warning: Error loading {file_path}: {e}")
            return {}
    
    def _save_json(self, file_path: Path, data: Dict):
        """Save JSON file with error handling"""
        try:
            data["last_updated"] = datetime.now().isoformat()
            with open(file_path, 'w') as f:
                json.dump(data, f, indent=2)
        except Exception as e:
            print(f"Error saving {file_path}: {e}")
    
    def parse_task_metadata(self, task_file: str) -> Optional[TaskMetadata]:
        """Parse coordination metadata from task file"""
        task_path = Path(task_file)
        if not task_path.exists():
            return None
        
        try:
            with open(task_path, 'r') as f:
                content = f.read()
            
            # Extract metadata using simple parsing
            metadata = {}
            lines = content.split('\n')
            
            for line in lines:
                line = line.strip()
                if line.startswith('**Assigned Mode**:'):
                    metadata['mode'] = line.split(':', 1)[1].strip()
                elif line.startswith('**Priority**:'):
                    metadata['priority'] = line.split(':', 1)[1].strip()
                elif line.startswith('**Parallel Safety**:'):
                    metadata['parallel_safety'] = line.split(':', 1)[1].strip()
                elif line.startswith('**Classification**:'):
                    metadata['classification'] = line.split(':', 1)[1].strip()
                elif line.startswith('**Estimated Duration**:'):
                    duration_str = line.split(':', 1)[1].strip()
                    metadata['estimated_duration'] = int(duration_str.split()[0])
                elif line.startswith('**Conflicts With**:'):
                    conflicts = line.split(':', 1)[1].strip()
                    metadata['conflicts_with'] = [c.strip() for c in conflicts.split(',') if c.strip()]
                elif line.startswith('**Depends On**:'):
                    depends = line.split(':', 1)[1].strip()
                    metadata['depends_on'] = [d.strip() for d in depends.split(',') if d.strip()]
                elif line.startswith('**Modifies Files**:'):
                    modifies = line.split(':', 1)[1].strip()
                    metadata['modifies_files'] = [f.strip() for f in modifies.split(',') if f.strip()]
                elif line.startswith('**Reads Files**:'):
                    reads = line.split(':', 1)[1].strip()
                    metadata['reads_files'] = [f.strip() for f in reads.split(',') if f.strip()]
                elif line.startswith('**Locked Resources**:'):
                    locked = line.split(':', 1)[1].strip()
                    metadata['locked_resources'] = [r.strip() for r in locked.split(',') if r.strip()]
                elif line.startswith('**Parallel Compatible**:'):
                    compatible = line.split(':', 1)[1].strip().lower()
                    metadata['parallel_compatible'] = compatible in ['true', 'yes', '1']
            
            return TaskMetadata(
                task_file=task_file,
                mode=metadata.get('mode', 'unknown'),
                priority=metadata.get('priority', 'medium'),
                parallel_safety=metadata.get('parallel_safety', 'CONDITIONAL'),
                classification=metadata.get('classification', 'Unknown'),
                estimated_duration=metadata.get('estimated_duration', 60),
                conflicts_with=metadata.get('conflicts_with', []),
                depends_on=metadata.get('depends_on', []),
                modifies_files=metadata.get('modifies_files', []),
                reads_files=metadata.get('reads_files', []),
                locked_resources=metadata.get('locked_resources', []),
                parallel_compatible=metadata.get('parallel_compatible', False)
            )
        
        except Exception as e:
            print(f"Error parsing task metadata from {task_file}: {e}")
            return None
    
    def register_agent(self, agent_id: str, mode: str, task_path: str) -> bool:
        """Register an agent with the coordination system"""
        agents_data = self._load_json(self.active_agents_file)
        
        # Parse task metadata
        task_metadata = self.parse_task_metadata(task_path)
        if not task_metadata:
            print(f"Warning: Could not parse task metadata for {task_path}")
            return False
        
        # Check for conflicts before registration
        conflicts = self.check_task_conflicts(task_metadata)
        if conflicts:
            print(f"Cannot register agent {agent_id}: conflicts detected: {conflicts}")
            return False
        
        # Calculate estimated completion
        estimated_completion = datetime.now() + timedelta(minutes=task_metadata.estimated_duration)
        
        # Register agent
        agents_data['agents'][agent_id] = {
            'mode': mode,
            'current_task': os.path.basename(task_path),
            'task_path': task_path,
            'status': 'starting',
            'started_at': datetime.now().isoformat(),
            'estimated_completion': estimated_completion.isoformat(),
            'locked_files': [],
            'parallel_compatible': task_metadata.parallel_compatible,
            'heartbeat': datetime.now().isoformat()
        }
        
        self._save_json(self.active_agents_file, agents_data)
        
        # Acquire file locks
        for file_path in task_metadata.modifies_files:
            self.acquire_file_lock(agent_id, file_path, 'write')
        
        return True
    
    def unregister_agent(self, agent_id: str) -> bool:
        """Unregister an agent from the coordination system"""
        agents_data = self._load_json(self.active_agents_file)
        
        if agent_id not in agents_data.get('agents', {}):
            return False
        
        # Release all file locks
        agent_data = agents_data['agents'][agent_id]
        for file_path in agent_data.get('locked_files', []):
            self.release_file_lock(agent_id, file_path)
        
        # Remove agent
        del agents_data['agents'][agent_id]
        self._save_json(self.active_agents_file, agents_data)
        
        return True
    
    def acquire_file_lock(self, agent_id: str, file_path: str, lock_type: str = 'write') -> bool:
        """Acquire a file lock for an agent"""
        locks_data = self._load_json(self.file_locks_file)
        
        # Check for conflicting locks
        if file_path in locks_data.get('locks', {}):
            existing_lock = locks_data['locks'][file_path]
            
            # Check if lock is expired
            expires_at = datetime.fromisoformat(existing_lock['expires_at'])
            if datetime.now() > expires_at:
                # Remove expired lock
                del locks_data['locks'][file_path]
            else:
                # Check compatibility
                if existing_lock['lock_type'] == 'read' and lock_type == 'read':
                    # Read-read is compatible
                    pass
                else:
                    # Write locks are exclusive
                    return False
        
        # Acquire lock
        expires_at = datetime.now() + timedelta(minutes=60)
        locks_data['locks'][file_path] = {
            'locked_by': agent_id,
            'lock_type': lock_type,
            'acquired_at': datetime.now().isoformat(),
            'expires_at': expires_at.isoformat(),
            'purpose': 'task_execution'
        }
        
        self._save_json(self.file_locks_file, locks_data)
        
        # Update agent's locked files
        agents_data = self._load_json(self.active_agents_file)
        if agent_id in agents_data.get('agents', {}):
            if file_path not in agents_data['agents'][agent_id]['locked_files']:
                agents_data['agents'][agent_id]['locked_files'].append(file_path)
                self._save_json(self.active_agents_file, agents_data)
        
        return True
    
    def release_file_lock(self, agent_id: str, file_path: str) -> bool:
        """Release a file lock"""
        locks_data = self._load_json(self.file_locks_file)
        
        if file_path in locks_data.get('locks', {}):
            lock = locks_data['locks'][file_path]
            if lock['locked_by'] == agent_id:
                del locks_data['locks'][file_path]
                self._save_json(self.file_locks_file, locks_data)
                
                # Update agent's locked files
                agents_data = self._load_json(self.active_agents_file)
                if agent_id in agents_data.get('agents', {}):
                    locked_files = agents_data['agents'][agent_id]['locked_files']
                    if file_path in locked_files:
                        locked_files.remove(file_path)
                        self._save_json(self.active_agents_file, agents_data)
                
                return True
        
        return False
    
    def check_task_conflicts(self, task_metadata: TaskMetadata) -> List[str]:
        """Check for conflicts with a proposed task"""
        conflicts = []
        
        # Check file conflicts
        locks_data = self._load_json(self.file_locks_file)
        for file_path in task_metadata.modifies_files:
            if file_path in locks_data.get('locks', {}):
                lock = locks_data['locks'][file_path]
                expires_at = datetime.fromisoformat(lock['expires_at'])
                if datetime.now() <= expires_at:
                    conflicts.append(f"File locked: {file_path} by {lock['locked_by']}")
        
        # Check task dependencies
        for dependency in task_metadata.depends_on:
            if not self.is_task_complete(dependency):
                conflicts.append(f"Dependency not complete: {dependency}")
        
        # Check explicit conflicts
        agents_data = self._load_json(self.active_agents_file)
        for agent_id, agent_data in agents_data.get('agents', {}).items():
            current_task = agent_data['current_task']
            if current_task in task_metadata.conflicts_with:
                conflicts.append(f"Conflicting task active: {current_task}")
        
        return conflicts
    
    def is_task_complete(self, task_file: str) -> bool:
        """Check if a task is complete"""
        # For now, assume task is complete if the file doesn't exist
        # This is a simplified implementation
        task_path = Path(task_file)
        if not task_path.is_absolute():
            # Try to find the task file in the project
            for impl in ['gol-bevy', 'gol-entt', 'gol-flecs', 'gol-console-client', 'meta']:
                potential_path = self.base_path / impl / "tasks" / task_file
                if potential_path.exists():
                    return False
        else:
            return not task_path.exists()
        
        return True
    
    def get_compatible_tasks(self, agent_mode: str) -> List[str]:
        """Get tasks that are compatible with parallel execution"""
        compatible_tasks = []
        
        # Find all tasks for the agent mode
        mode_path = self.base_path / f"gol-{agent_mode}" / "tasks"
        if agent_mode == "meta":
            mode_path = self.base_path / "meta" / "tasks"
        elif agent_mode == "console":
            mode_path = self.base_path / "gol-console-client" / "tasks"
        
        if not mode_path.exists():
            return compatible_tasks
        
        for task_file in mode_path.glob("*.md"):
            task_metadata = self.parse_task_metadata(str(task_file))
            if task_metadata and task_metadata.parallel_compatible:
                conflicts = self.check_task_conflicts(task_metadata)
                if not conflicts:
                    compatible_tasks.append(str(task_file))
        
        return compatible_tasks
    
    def update_agent_status(self, agent_id: str, status: str) -> bool:
        """Update agent status"""
        agents_data = self._load_json(self.active_agents_file)
        
        if agent_id in agents_data.get('agents', {}):
            agents_data['agents'][agent_id]['status'] = status
            agents_data['agents'][agent_id]['heartbeat'] = datetime.now().isoformat()
            self._save_json(self.active_agents_file, agents_data)
            return True
        
        return False
    
    def update_heartbeat(self, agent_id: str) -> bool:
        """Update agent heartbeat"""
        agents_data = self._load_json(self.active_agents_file)
        
        if agent_id in agents_data.get('agents', {}):
            agents_data['agents'][agent_id]['heartbeat'] = datetime.now().isoformat()
            self._save_json(self.active_agents_file, agents_data)
            return True
        
        return False
    
    def cleanup_expired_locks(self):
        """Clean up expired file locks"""
        locks_data = self._load_json(self.file_locks_file)
        expired_locks = []
        
        for file_path, lock in locks_data.get('locks', {}).items():
            expires_at = datetime.fromisoformat(lock['expires_at'])
            if datetime.now() > expires_at:
                expired_locks.append(file_path)
        
        for file_path in expired_locks:
            del locks_data['locks'][file_path]
            print(f"Cleaned up expired lock: {file_path}")
        
        if expired_locks:
            self._save_json(self.file_locks_file, locks_data)
    
    def cleanup_stale_agents(self):
        """Clean up agents with stale heartbeats"""
        agents_data = self._load_json(self.active_agents_file)
        stale_agents = []
        
        for agent_id, agent_data in agents_data.get('agents', {}).items():
            heartbeat = datetime.fromisoformat(agent_data['heartbeat'])
            if datetime.now() - heartbeat > timedelta(minutes=15):
                stale_agents.append(agent_id)
        
        for agent_id in stale_agents:
            print(f"Cleaning up stale agent: {agent_id}")
            self.unregister_agent(agent_id)
    
    def get_status_report(self) -> Dict:
        """Get comprehensive status report"""
        self.cleanup_expired_locks()
        self.cleanup_stale_agents()
        
        agents_data = self._load_json(self.active_agents_file)
        locks_data = self._load_json(self.file_locks_file)
        
        return {
            'active_agents': len(agents_data.get('agents', {})),
            'active_locks': len(locks_data.get('locks', {})),
            'agents': agents_data.get('agents', {}),
            'locks': locks_data.get('locks', {}),
            'coordination_enabled': agents_data.get('coordination_enabled', True)
        }


def main():
    """Main function for testing coordination system"""
    coord = CoordinationSystem()
    
    # Example usage
    print("Coordination System Status:")
    status = coord.get_status_report()
    print(f"Active agents: {status['active_agents']}")
    print(f"Active locks: {status['active_locks']}")
    print(f"Coordination enabled: {status['coordination_enabled']}")


if __name__ == "__main__":
    main()