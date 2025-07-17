# Coordination System Documentation

## Overview
The coordination system prevents conflicts between multiple agents working simultaneously. It tracks active agents and file locks to ensure safe parallel development.

## System Components

### 1. Coordination Files
- **active-agents.json**: Tracks currently active agents and their tasks
- **file-locks.json**: Manages file locking for shared resources

### 2. Coordination Scripts
- **register-agent.sh**: Registers an agent as active
- **unregister-agent.sh**: Removes an agent from active tracking
- **check-coordination.sh**: Validates system integrity and detects failures
- **cleanup-coordination.sh**: Removes stale agents and locks
- **analyze-agent-tasks.sh**: Enhanced with coordination awareness

## Agent Registration Protocol

### Starting Work
When an agent begins working on a task:
```bash
bash meta/scripts/register-agent.sh [agent-mode] [task-file]
```

Example:
```bash
bash meta/scripts/register-agent.sh bevy setup-project.md
```

### Completing Work
When an agent completes a task:
```bash
bash meta/scripts/unregister-agent.sh [agent-mode] [task-file]
```

Example:
```bash
bash meta/scripts/unregister-agent.sh bevy setup-project.md
```

## Coordination Status Detection

### Check System Status
```bash
bash meta/scripts/check-coordination.sh
```

This provides:
- Coordination file integrity validation
- Active agent detection
- File lock status
- Stale agent detection
- Dependency verification

### Cleanup Stale Entries
```bash
bash meta/scripts/cleanup-coordination.sh          # Check only
bash meta/scripts/cleanup-coordination.sh --force  # Remove stale entries
```

## Task Analysis Integration

The analyze-agent-tasks.sh script now includes:
- Coordination status awareness
- Agent registration commands
- Conflict detection
- Resource availability assessment

## Active Agent Tracking

### active-agents.json Structure
```json
{
  "agents": {
    "bevy_1642678943": {
      "mode": "bevy",
      "task_file": "setup-project.md",
      "start_time": "2024-01-20T10:15:43Z",
      "status": "active"
    }
  },
  "last_updated": "2024-01-20T10:15:43Z",
  "schema_version": "1.0",
  "coordination_enabled": true,
  "notes": "Tracks currently active agents and their work status"
}
```

### File Lock Tracking

### file-locks.json Structure
```json
{
  "locks": {
    "ARCHITECTURE.md": {
      "agent_id": "orchestrator_1642678943",
      "timestamp": "2024-01-20T10:15:43Z",
      "lock_type": "write"
    }
  },
  "last_updated": "2024-01-20T10:15:43Z",
  "schema_version": "1.0",
  "lock_timeout_minutes": 60,
  "notes": "Manages file locking for shared resources"
}
```

## Failure Detection

### Coordination Failures
The system detects:
- Missing coordination files
- Invalid JSON structure
- Stale agents (older than 1 hour)
- Stale file locks (older than 1 hour)
- Missing script dependencies

### Automatic Recovery
- Coordination files are created on first use
- Stale entries can be cleaned up automatically
- System validates integrity on each check

## Integration with Agent Workflows

### Mode-Specific Usage

**For All Implementation Agents (Bevy, EnTT, Flecs, Console):**
1. Run task analysis: `bash meta/scripts/analyze-agent-tasks.sh [mode]`
2. Register before starting work: `bash meta/scripts/register-agent.sh [mode] [task]`
3. Unregister after completing work: `bash meta/scripts/unregister-agent.sh [mode] [task]`

**For Orchestrator Mode:**
- Check coordination status before creating tasks
- Monitor active agents to prevent conflicts
- Clean up stale entries as needed

**For Meta Mode:**
- Maintain coordination system integrity
- Monitor and improve coordination processes
- Handle coordination system failures

## Troubleshooting

### Common Issues

**"No coordination files found"**
- Normal on first use
- Files created automatically on first agent registration

**"Invalid JSON structure"**
- Run `bash meta/scripts/check-coordination.sh` to identify issues
- Regenerate files if necessary

**"Stale agents detected"**
- Run `bash meta/scripts/cleanup-coordination.sh --force` to clean up
- Indicates agents didn't properly unregister

**"Coordination conflicts"**
- Check active agents with `bash meta/scripts/check-coordination.sh`
- Ensure only one agent works on conflicting tasks

### Recovery Procedures

1. **System Integrity Check**: Run check-coordination.sh
2. **Stale Entry Cleanup**: Run cleanup-coordination.sh --force
3. **Manual Reset**: Delete coordination files to start fresh
4. **Dependency Verification**: Ensure jq is installed

## System Maintenance

### Regular Maintenance
- Check coordination status before long work sessions
- Clean up stale entries weekly
- Validate system integrity monthly

### Performance Considerations
- Coordination files are small JSON documents
- Registration/unregistration is lightweight
- System scales well with multiple agents

## Future Enhancements

### Planned Features
- Automatic stale entry cleanup
- Web dashboard for coordination status
- Integration with development tools
- Performance metrics and monitoring

### Extension Points
- Custom lock types
- Agent priority systems
- Distributed coordination
- Integration with CI/CD pipelines

## Usage Examples

### Basic Agent Workflow
```bash
# Check available tasks
bash meta/scripts/analyze-agent-tasks.sh bevy

# Register as active agent
bash meta/scripts/register-agent.sh bevy setup-project.md

# Work on task (implementation code here)

# Unregister when complete
bash meta/scripts/unregister-agent.sh bevy setup-project.md
```

### Coordination Monitoring
```bash
# Check system status
bash meta/scripts/check-coordination.sh

# View active agents
jq '.agents' meta/coordination/active-agents.json

# Clean up stale entries
bash meta/scripts/cleanup-coordination.sh --force
```

### Emergency Reset
```bash
# Remove all coordination state
rm -f meta/coordination/active-agents.json meta/coordination/file-locks.json

# Verify system is clean
bash meta/scripts/check-coordination.sh
```

This coordination system ensures safe parallel development while maintaining simplicity and reliability.