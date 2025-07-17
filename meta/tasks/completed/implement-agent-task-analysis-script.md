# Implement Agent Task Analysis Script

## Task Overview
**Priority**: High
**Assigned Mode**: Meta
**Created By**: Orchestrator based on efficiency analysis
**Implementation Scope**: Cross-agent workflow optimization

## Objective
Create a unified bash script that performs comprehensive task analysis for any agent in a single tool call, replacing the current multi-tool approach that requires separate calls to Glob, Read, and coordination file checks.

## Problem Statement
Current task analysis requires multiple tool calls:
1. `Glob` to find available tasks
2. Multiple `Read` calls for each task file
3. `Read` calls for coordination status files
4. Manual analysis and priority sorting

This is inefficient and inconsistent across agents. A single bash script could:
- Reduce 5-10 tool calls to 1 tool call
- Provide consistent analysis format across all agents
- Include coordination status automatically
- Enable faster agent decision-making

## Requirements

### Core Script Functionality
1. **Universal Agent Support**
   - Accept agent mode as parameter (bevy, entt, flecs, console, meta)
   - Automatically determine correct tasks directory
   - Handle mode-specific task analysis requirements

2. **Comprehensive Task Discovery**
   - Find all .md files in agent's tasks/ directory
   - Parse task metadata (priority, dependencies, conflicts, etc.)
   - Identify completed vs. available tasks
   - Extract estimated duration and parallel safety information

3. **Coordination Integration**
   - Check active-agents.json for current agent activity
   - Verify file-locks.json for resource conflicts
   - Apply coordination rules to filter available tasks
   - Highlight parallel execution opportunities

4. **Priority Analysis**
   - Sort tasks by priority (High > Medium > Low)
   - Apply coordination-based scoring adjustments
   - Identify blocked tasks due to conflicts
   - Recommend next task based on agent mode and availability

### Script Output Format
```bash
=== AGENT TASK ANALYSIS: [MODE] ===
Coordination Status: [CLEAR/CONFLICTS]
Active Agents: [count] ([agent1, agent2, ...])
File Locks: [count] ([file1, file2, ...])

=== AVAILABLE TASKS ===
🔴 HIGH PRIORITY
  • task-name.md [2-3hrs] [Parallel: Yes/No] [Conflicts: none/list]
    Brief description from task file

🟡 MEDIUM PRIORITY
  • task-name.md [4-6hrs] [Parallel: Yes/No] [Conflicts: none/list]
    Brief description from task file

🟢 LOW PRIORITY
  • task-name.md [1-2hrs] [Parallel: Yes/No] [Conflicts: none/list]
    Brief description from task file

=== BLOCKED TASKS ===
  • blocked-task.md - Blocked by: [reason]

=== COMPLETED TASKS ===
  • completed-task.md - Completed: [date]

=== RECOMMENDED NEXT TASK ===
🎯 task-name.md
   Priority: High
   Duration: 2-3 hours
   Parallel Safe: Yes
   Conflicts: None
   Rationale: [reason for recommendation]

=== AGENT DECISION SUMMARY ===
Total Available: [count]
Coordination Status: [status]
Resource Availability: [available/conflicts]
Recommendation: [action]
```

### Script Location and Usage
- **Location**: `/mnt/c/dev/gol/meta/scripts/analyze-agent-tasks.sh`
- **Usage**: `bash meta/scripts/analyze-agent-tasks.sh [agent-mode]`
- **Examples**:
  - `bash meta/scripts/analyze-agent-tasks.sh bevy`
  - `bash meta/scripts/analyze-agent-tasks.sh meta`
  - `bash meta/scripts/analyze-agent-tasks.sh console`

## Implementation Details

### Script Architecture
1. **Parameter Validation**
   - Validate agent mode parameter
   - Verify tasks directory exists
   - Check coordination system availability

2. **Task Discovery Engine**
   - Use `find` to locate all .md files in tasks directory
   - Parse each file for metadata using `grep` and `awk`
   - Extract: priority, duration, parallel safety, conflicts, dependencies

3. **Coordination Integration**
   - Parse active-agents.json for current agent status
   - Parse file-locks.json for resource conflicts
   - Apply coordination rules to filter available tasks

4. **Priority Scoring Algorithm**
   - Base priority score from task metadata
   - Coordination bonus for parallel-safe tasks
   - Penalty for tasks with conflicts
   - Availability bonus for unblocked tasks

5. **Output Formatting**
   - Structured output with clear sections
   - Color coding for priority levels (if terminal supports)
   - Consistent formatting across all agent modes

### Error Handling
- Graceful degradation if coordination files missing
- Handle malformed task files without breaking analysis
- Provide helpful error messages for invalid parameters
- Fallback to basic analysis if coordination system unavailable

## Expected Deliverables
1. **`analyze-agent-tasks.sh`** - Main analysis script
2. **`task-parser-utils.sh`** - Utility functions for task metadata parsing
3. **`coordination-checker.sh`** - Coordination status checking functions
4. **`README-task-analysis.md`** - Documentation and usage guide
5. **Agent integration examples** - Sample usage in each agent mode

## Acceptance Criteria
- [ ] Script works for all agent modes (bevy, entt, flecs, console, meta)
- [ ] Single bash call replaces 5-10 tool calls
- [ ] Consistent output format across all agents
- [ ] Coordination status integrated automatically
- [ ] Priority-based task recommendations
- [ ] Error handling for edge cases
- [ ] Performance improvement over multi-tool approach
- [ ] Documentation and usage examples provided

## Technical Considerations
- **Performance**: < 1 second execution time
- **Portability**: Works on Linux/WSL environments
- **Maintainability**: Clear, documented bash code
- **Extensibility**: Easy to add new analysis features
- **Reliability**: Handles missing files and edge cases gracefully

## Integration Points
- **Agent Mode Templates**: Update all modes to use script
- **CLAUDE.md**: Add script usage documentation
- **Coordination System**: Integrate with existing coordination infrastructure
- **Task Templates**: Ensure metadata is parseable by script

## Success Metrics
- **Tool Call Reduction**: 5-10 calls → 1 call per task analysis
- **Consistency**: Identical analysis format across all agents
- **Speed**: < 1 second analysis time vs. 5-10 seconds multi-tool
- **Accuracy**: 100% task discovery and coordination status
- **Usability**: Clear, actionable output for agent decision-making

## Usage Examples

### Bevy Agent Task Analysis
```bash
# Current approach (5+ tool calls)
Glob(pattern="gol-bevy/tasks/*.md")
Read(file1), Read(file2), Read(file3)...
Read(active-agents.json), Read(file-locks.json)
# Manual analysis and priority sorting

# New approach (1 tool call)
Bash(command="bash meta/scripts/analyze-agent-tasks.sh bevy")
```

### Meta Agent Task Analysis
```bash
# Current approach (6+ tool calls)
Glob(pattern="meta/tasks/*.md") 
Read(task1), Read(task2), Read(task3)...
Read(coordination files)...
# Manual analysis

# New approach (1 tool call)
Bash(command="bash meta/scripts/analyze-agent-tasks.sh meta")
```

## Notes
This improvement provides immediate efficiency gains for all agents while maintaining consistency and improving decision-making speed. The script approach is more maintainable and extensible than the current multi-tool pattern.

## Dependencies
- Existing coordination system infrastructure
- Task metadata format consistency
- Bash environment (Linux/WSL)
- Standard Unix utilities (find, grep, awk, cat)

## Estimated Effort
**4-6 hours** - Script development, testing, and documentation