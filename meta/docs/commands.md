# User Commands Reference

**CRITICAL**: These commands MUST be recognized and responded to immediately without using tools or analysis. When user input matches a command pattern, execute the command directly.

## Command Recognition Rules
```json
{
  "command_patterns": {
    "mode_management": {
      "switch_mode": ["switch to * mode", "become *", "work on * implementation", "work on console client", "work on meta improvements", "coordinate project", "return to default mode"],
      "action": "change_mode"
    },
    "development": {
      "continue_development": ["continue development", "continue", "keep going", "next task"],
      "whats_next": ["what's next", "what is next", "next", "what should i do", "what should i work on"],
      "update_orchestrator": ["update the orchestrator", "update orchestrator", "report to orchestrator"],
      "orchestrate_task": ["orchestrate *", "orchestrate task *", "orchestrate work on *"],
      "action": "execute_immediately"
    },
    "information": {
      "commands": ["commands", "help", "what can i do"],
      "mode_status": ["what mode am i in", "current mode", "my mode"],
      "project_status": ["show project status", "project status", "status"],
      "list_tasks": ["list available tasks", "show tasks", "available tasks", "list tasks"],
      "memory": ["/memory", "memory", "memory status", "show memory"],
      "action": "provide_info"
    },
    "maintenance": {
      "run_maintenance": ["run maintenance", "maintenance", "execute maintenance"],
      "weekly_maintenance": ["weekly maintenance", "orchestrator maintenance", "weekly review"],
      "validate_documentation": ["validate documentation", "check documentation", "doc validation"],
      "action": "execute_maintenance"
    },
    "documentation": {
      "update_docs": ["update documentation", "refresh docs"],
      "show_architecture": ["show architecture", "architecture"],
      "show_requirements": ["show requirements", "requirements"],
      "is_documentation_up_to_date": ["is documentation up to date", "documentation status", "doc status"],
      "action": "display_docs"
    },
    "session_management": {
      "begin_shutdown": ["begin shutdown", "start shutdown", "shutdown", "end session"],
      "prepare_shutdown": ["prepare for shutdown", "prepare shutdown", "shutdown preparation"],
      "session_status": ["session status", "current status", "task status"],
      "action": "session_management"
    },
    "error_recovery": {
      "reset": ["reset to default", "reset", "start over"],
      "clarify": ["clarify mode", "explain mode", "what should i do"],
      "escalate": ["escalate to orchestrator", "escalate", "need help"],
      "action": "recover"
    }
  }
}
```

## Development Commands (HIGH PRIORITY)
- **"Continue development"** - Work on next highest priority task immediately (WITH MANDATORY COORDINATION)
- **"What's next"** - Summarize next task without executing it
- **"Update the orchestrator"** - Provide structured completion report (stay in current mode)
- **"Orchestrate [task description]"** - Create task file and spawn sub-task using Task tool (orchestrator mode only)

## Coordination Enforcement Commands (MANDATORY)
- **"Enforce coordination [task-file]"** - Claim task and register with coordination system
- **"Check coordination"** - Verify current coordination status
- **"Recover coordination"** - Recover from coordination failures
- **"Clear stale claims"** - Remove claims older than 4 hours

## Mode Management Commands
- **"Switch to [mode] mode"** - Change to specified mode (orchestrator, bevy, entt, flecs, console, meta, default)
- **"Become [orchestrator|bevy|entt|flecs|console|meta]"** - Alternative mode switching syntax
- **"Work on [Bevy|EnTT|Flecs] implementation"** - Switch to implementation-specific mode
- **"Work on console client"** - Switch to console mode
- **"Work on meta improvements"** - Switch to meta mode
- **"Coordinate project"** - Switch to orchestrator mode
- **"Return to default mode"** - Return to default mode

## Information Commands
- **"commands"** - Display this command reference
- **"What mode am I in?"** - Show current operating mode and constraints
- **"Show project status"** - Display overall project progress and task completion
- **"List available tasks"** or **"List tasks"** - Show tasks available in current mode's directory
- **"/memory"** - Show memory and learning status, including recent insights and documentation updates

## Maintenance Commands
- **"Run maintenance"** - Execute maintenance protocol for current mode
- **"Weekly maintenance"** - Execute weekly orchestrator maintenance (orchestrator mode only)
- **"Validate documentation"** - Check documentation accuracy against current project state

## Documentation Commands
- **"Update documentation"** - Refresh project documentation based on current state (should be minimal if documentation-first followed)
- **"Is documentation up to date?"** - Verify documentation currency and identify gaps
- **"Show architecture"** - Display current architecture decisions and rationale
- **"Show requirements"** - Display functional requirements and specifications

## Session Management Commands
- **"Begin shutdown"** - Initiate the proper shutdown sequence
- **"Is documentation up to date?"** - Verify documentation currency before shutdown
- **"Prepare for shutdown"** - Execute shutdown preparation checklist
- **"Session status"** - Show current task status and readiness for shutdown

## Error Recovery Commands
- **"Reset to default"** - Return to default mode and clear any mode confusion
- **"Clarify mode"** - Explain current mode's responsibilities and constraints
- **"Escalate to orchestrator"** - Report blockers requiring orchestrator intervention

## Command Execution Rules
1. **Immediate Recognition**: Match user input against command patterns first, before any tool usage
2. **No Tool Usage**: Commands should be executed directly without bash, filesystem, or search tools
3. **Contextual Response**: Provide response based on current mode and available tasks
4. **Fallback**: If no command matches, proceed with normal task execution
5. **Documentation First**: When creating new commands, IMMEDIATELY document them in CLAUDE.md before implementation
6. **MANDATORY COORDINATION**: All task work must use coordination enforcement before beginning

## Coordination Enforcement Integration
All agents MUST use coordination enforcement before beginning task work:

### Before Task Work:
```bash
# Check coordination status
bash meta/scripts/mandatory-coordination-check.sh [agent-mode] work_on_task

# Claim task (if not already claimed)
bash meta/scripts/enforce-coordination.sh [agent-mode] [task-file]
```

### During Task Work:
- Maintain task claim file
- Follow coordination protocol
- Update progress as needed

### After Task Completion:
```bash
# Remove claim file
rm [task-directory]/[task-file].claimed

# Unregister agent
bash meta/scripts/unregister-agent.sh [agent-mode] [task-file]
```

### Continue Development Command:
```bash
# Use enhanced continue development
bash meta/scripts/mandatory-coordination-check.sh [agent-mode] continue_development
```

## "List Tasks" Command Implementation

### For Implementation Agents (Bevy, EnTT, Flecs, Console)
- Lists only tasks in the current agent's tasks/ directory
- Shows just the .md filenames (e.g., environment-setup.md, bevy-validation.md)
- Implementation: `ls {current_mode_directory}/tasks/*.md | xargs basename`

### For Orchestrator Mode
- Lists tasks across ALL agent directories
- Shows tasks organized by agent/mode
- Implementation: `find /mnt/c/Dev/gol -path "*/tasks/*.md" -type f -exec basename {} \; | sort`
- Format: Shows agent assignment and task filename

### For Default Mode
- Shows overview of all available tasks
- May include recommendations for which mode to switch to
- Implementation: Same as orchestrator but with mode switching guidance

## Session Management Commands Implementation

### Shutdown Process Script Usage
- **Session Status**: `bash meta/scripts/shutdown-process.sh status [mode]`
- **Prepare Shutdown**: `bash meta/scripts/shutdown-process.sh prepare [mode]`
- **Begin Shutdown**: `bash meta/scripts/shutdown-process.sh begin [mode]`
- **Documentation Check**: `bash meta/scripts/shutdown-process.sh doc_check`

### Shutdown Process Phases
1. **Task Completion Verification**: Checks active vs completed tasks
2. **Documentation Validation**: Verifies documentation currency
3. **Code Quality Verification**: Runs linting/testing if applicable
4. **Handoff Preparation**: Prepares next session priorities

### Command Integration
- **"Begin shutdown"**: Executes full shutdown sequence with status checks
- **"Is documentation up to date?"**: Runs documentation currency verification
- **"Session status"**: Shows current task status and shutdown readiness
- **"Prepare for shutdown"**: Generates shutdown checklist and next steps