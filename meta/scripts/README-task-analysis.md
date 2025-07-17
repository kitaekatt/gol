# Agent Task Analysis Script

## Overview
The `analyze-agent-tasks.sh` script provides comprehensive task analysis for any agent in a single tool call, replacing the previous multi-tool approach that required 5-10 separate calls.

## Performance Improvement
- **Before**: 5-10 tool calls (Glob + multiple Read calls + coordination checks)
- **After**: 1 tool call (single bash script execution)
- **Speed**: < 1 second vs. 5-10 seconds multi-tool approach

## Usage

### Basic Usage
```bash
bash meta/scripts/analyze-agent-tasks.sh [agent-mode]
```

### Supported Agent Modes
- `bevy` - Bevy ECS implementation tasks
- `entt` - EnTT ECS implementation tasks  
- `flecs` - Flecs ECS implementation tasks
- `console` - Console client tasks
- `meta` - Meta-development tasks

### Examples
```bash
# Analyze Bevy agent tasks
bash meta/scripts/analyze-agent-tasks.sh bevy

# Analyze Meta agent tasks
bash meta/scripts/analyze-agent-tasks.sh meta

# Analyze Console client tasks
bash meta/scripts/analyze-agent-tasks.sh console
```

## Output Format

The script provides structured output with the following sections:

### 1. Agent and Coordination Status
```
=== AGENT TASK ANALYSIS: [MODE] ===
Coordination Status: [CLEAR/CONFLICTS]
Active Agents: [count]
File Locks: [count]
```

### 2. Available Tasks by Priority
```
=== AVAILABLE TASKS ===
🔴 HIGH PRIORITY
  • task-name.md [duration] [Parallel: Yes/No] [Conflicts: none/list]
    Brief description from task file

🟡 MEDIUM PRIORITY
  • task-name.md [duration] [Parallel: Yes/No] [Conflicts: none/list]
    Brief description from task file

🟢 LOW PRIORITY
  • task-name.md [duration] [Parallel: Yes/No] [Conflicts: none/list]
    Brief description from task file
```

### 3. Blocked Tasks (if any)
```
=== BLOCKED TASKS ===
  • blocked-task.md - Blocked by: [reason]
```

### 4. Completed Tasks
```
=== COMPLETED TASKS ===
  • completed-task.md - Status: [completion status]
```

### 5. Task Recommendation
```
=== RECOMMENDED NEXT TASK ===
🎯 task-name.md
   Priority: [High/Medium/Low]
   Duration: [estimated time]
   Parallel Safe: [Yes/No]
   Conflicts: [none/list]
   Rationale: [reason for recommendation]
```

### 6. Decision Summary
```
=== AGENT DECISION SUMMARY ===
Total Available: [count]
Coordination Status: [CLEAR/CONFLICTS]
Resource Availability: [available/conflicts]
Recommendation: [action to take]
```

## Task Metadata Parsing

The script automatically parses the following metadata from task files:

### Priority Detection
- Searches for `Priority.*High`, `Priority.*Medium`, `Priority.*Low`
- Defaults to Medium if not specified

### Duration Extraction
- Looks for `estimated.*effort`, `duration`, `estimated effort`
- Extracts time estimates (e.g., "4-6 hours", "2 days")

### Parallel Safety
- Detects `Parallel Safe.*true/Yes` or `Parallel Safe.*false/No`
- Indicates if task can run alongside other tasks

### Conflicts and Dependencies
- Extracts `Conflicts With` and `Depends On` lists
- Used for coordination and task scheduling

### Completion Status
- Detects `TASK COMPLETED`, `COMPLETED`, `Status.*Complete`
- Automatically categorizes completed tasks

## Coordination Integration

The script integrates with the coordination system by:

### Active Agent Tracking
- Reads `meta/coordination/active-agents.json`
- Shows current agent activity and conflicts

### File Lock Checking
- Reads `meta/coordination/file-locks.json`
- Identifies resource conflicts and blocking

### Conflict Detection
- Applies coordination rules to filter available tasks
- Prevents conflicts during parallel execution

## Error Handling

The script includes robust error handling:

### Validation
- Validates agent mode parameter
- Checks for required directories and files
- Provides helpful error messages

### Graceful Degradation
- Works even if coordination files are missing
- Handles malformed task files without breaking
- Continues analysis despite individual task parsing errors

### Fallback Behavior
- Provides basic analysis if coordination system unavailable
- Uses default values for missing metadata
- Maintains functionality in edge cases

## Integration with Agent Workflows

### Agent Mode Integration
Replace multi-tool task analysis with single script call:

```bash
# Instead of:
# Glob(pattern="gol-bevy/tasks/*.md")
# Read(task1.md), Read(task2.md), Read(task3.md)...
# Read(active-agents.json), Read(file-locks.json)

# Use:
Bash(command="bash meta/scripts/analyze-agent-tasks.sh bevy")
```

### Consistent Analysis
- All agents use identical analysis format
- Uniform task prioritization across modes
- Standardized coordination integration

## Technical Implementation

### Dependencies
- Standard Unix utilities: `find`, `grep`, `sed`, `cut`, `jq`
- Bash shell (Linux/WSL compatible)
- JSON parsing via `jq` for coordination files

### Performance
- Execution time: < 1 second
- Memory usage: Minimal
- File I/O: Optimized single-pass processing

### Extensibility
- Easy to add new metadata parsing
- Modular functions for different analysis types
- Configurable output formatting

## Troubleshooting

### Common Issues

#### Script Not Executable
```bash
chmod +x meta/scripts/analyze-agent-tasks.sh
```

#### Missing Dependencies
```bash
# Install jq if not available
sudo apt-get install jq
```

#### Invalid Agent Mode
```
Error: Invalid agent mode 'invalid-mode'
Usage: ./analyze-agent-tasks.sh [agent-mode]
Agent modes: bevy, entt, flecs, console, meta
```

#### Tasks Directory Not Found
```
Error: Tasks directory not found: /path/to/tasks
```
- Verify project structure
- Check agent mode spelling
- Ensure tasks directory exists

### Debug Mode
For detailed debugging, modify the script to add:
```bash
set -x  # Enable debug tracing
```

## Future Enhancements

Potential improvements for future versions:

### Enhanced Metadata
- More sophisticated priority scoring
- Dependency graph analysis
- Resource requirement tracking

### Advanced Coordination
- Multi-agent scheduling optimization
- Predictive conflict detection
- Dynamic load balancing

### Output Formats
- JSON output for programmatic use
- CSV export for analysis
- Custom formatting options

### Integration Features
- Direct task execution from script
- Automatic coordination updates
- Integration with CI/CD pipelines

## Contributing

When modifying the script:

1. Maintain backward compatibility
2. Add comprehensive error handling
3. Update documentation
4. Test with all agent modes
5. Verify coordination integration works correctly

## Files in This Package

- `analyze-agent-tasks.sh` - Main analysis script
- `README-task-analysis.md` - This documentation
- `../coordination/` - Coordination system integration files