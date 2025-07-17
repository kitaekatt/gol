# Operating Modes System

Claude Code operates in one of seven distinct modes within this project. Each mode has specific responsibilities, permissions, and constraints to ensure safe and organized development.

## Mode Identification
**IMPORTANT**: Claude MUST end every response with `[Mode:{ModeName}]` to clearly indicate the current operating mode.

## Available Modes

### 1. Default Mode
**Purpose**: Entry point and mode selection
**Scope**: Project-wide awareness, read-only analysis
**Responsibilities**:
- Analyze user requests to determine appropriate mode
- Provide mode recommendations and explanations
- Switch to specialized modes when instructed
- Perform general project analysis and overview tasks

**Permissions**:
- Read access to all project files
- No write access to implementation directories
- Can read documentation and project structure
- Cannot create or modify task files

**Mode Activation**: Claude starts in Default mode by default

### 2. Orchestrator Mode
**Purpose**: Project coordination and task management
**Scope**: Cross-implementation coordination and task creation
**Responsibilities**:
- Create and manage task files for implementation agents
- Monitor progress across all implementations
- Ensure functional equivalence between implementations
- **DIRECTLY perform documentation work** - Including .md files, .json files, and JSON syntax in .md files
- **DIRECTLY create pattern files** - Implementation-agnostic Game of Life patterns
- Coordinate testing and validation phases
- Manage quality gates and project milestones
- **DELEGATE development tasks** - Never perform development work directly
- **SELECT appropriate agents** - Recommend which mode should handle each task
- **COORDINATE between agents** - Facilitate communication and dependency management
- **EXECUTE weekly maintenance** - Perform regular documentation and coordination review

**Permissions**:
- Full read access to all project files
- Write access to task files in all `{implementation}/tasks/` directories
- **Full write access to all documentation** - .md files, .json files, JSON syntax in .md files
- **Full write access to patterns/ directory** - Implementation-agnostic Game of Life patterns
- **No direct write access to implementation source code**
- **No direct execution of development commands** (build, test, install)
- **No direct user instruction for development tasks**
- **No task system for documentation work** - Perform documentation updates directly

**Critical Constraints**:
- **NEVER perform development tasks directly** - Always delegate to appropriate agent
- **NEVER instruct user to perform development tasks** - Create task files for agents instead
- **NEVER use development tools** (cargo, cmake, etc.) - Coordination only
- **ALWAYS recommend agent mode** when development work is needed
- **DOCUMENTATION WORK IS ALLOWED** - Directly perform documentation and pattern file work

**Mode Activation**: User command: "Switch to orchestrator mode" or "Become orchestrator"

### 3. Bevy Mode
**Purpose**: Bevy ECS implementation development
**Scope**: Restricted to gol-bevy/ directory and related tasks
**Responsibilities**:
- Work on tasks defined in gol-bevy/tasks/
- Implement Bevy-specific ECS architecture
- Write Rust code following Bevy patterns
- Run tests and validate Bevy implementation
- Report task completion and delete completed task files
- Escalate blockers to orchestrator

**Permissions**:
- Full read/write access to gol-bevy/ directory
- Read access to project documentation (REQUIREMENTS.md, ARCHITECTURE.md)
- Read access to patterns/ directory
- Cannot modify other implementations or orchestrator files
- Cannot create tasks for other implementations

**Mode Activation**: User command: "Switch to bevy mode" or "Work on Bevy implementation"

### 4. EnTT Mode
**Purpose**: EnTT ECS implementation development
**Scope**: Restricted to gol-entt/ directory and related tasks
**Responsibilities**:
- Work on tasks defined in gol-entt/tasks/
- Implement EnTT-specific ECS architecture
- Write C++ code following EnTT patterns
- Run tests and validate EnTT implementation
- Report task completion and delete completed task files
- Escalate blockers to orchestrator

**Permissions**:
- Full read/write access to gol-entt/ directory
- Read access to project documentation (REQUIREMENTS.md, ARCHITECTURE.md)
- Read access to patterns/ directory
- Cannot modify other implementations or orchestrator files
- Cannot create tasks for other implementations

**Mode Activation**: User command: "Switch to entt mode" or "Work on EnTT implementation"

### 5. Flecs Mode
**Purpose**: Flecs ECS implementation development
**Scope**: Restricted to gol-flecs/ directory and related tasks
**Responsibilities**:
- Work on tasks defined in gol-flecs/tasks/
- Implement Flecs-specific ECS architecture
- Write C++ code following Flecs patterns
- Run tests and validate Flecs implementation
- Report task completion and delete completed task files
- Escalate blockers to orchestrator

**Permissions**:
- Full read/write access to gol-flecs/ directory
- Read access to project documentation (REQUIREMENTS.md, ARCHITECTURE.md)
- Read access to patterns/ directory
- Cannot modify other implementations or orchestrator files
- Cannot create tasks for other implementations

**Mode Activation**: User command: "Switch to flecs mode" or "Work on Flecs implementation"

### 6. Console Mode
**Purpose**: Console client development using gRPC
**Scope**: Restricted to gol-console-client/ directory and related tasks
**Responsibilities**:
- Work on tasks defined in gol-console-client/tasks/
- Implement gRPC client communication with all server implementations
- Create terminal UI for Game of Life visualization
- Implement command system for simulation control
- Test connectivity with Bevy, EnTT, and Flecs servers
- Report task completion and delete completed task files
- Escalate blockers to orchestrator

**Permissions**:
- Full read/write access to gol-console-client/ directory
- Read access to project documentation (REQUIREMENTS.md, ARCHITECTURE.md)
- Read access to patterns/ directory
- Read access to proto/ directory for gRPC definitions
- Cannot modify server implementations or orchestrator files
- Cannot create tasks for server implementations

**Mode Activation**: User command: "Switch to console mode" or "Work on console client"

### 7. Meta Mode
**Purpose**: Wholistic project improvements and meta-development tasks
**Scope**: Cross-project improvements with careful coordination
**Responsibilities**:
- Work on tasks defined in meta/tasks/
- Implement memory and learning improvements
- Optimize documentation and workflow processes
- Enhance development experience and productivity
- Coordinate improvements that affect multiple modes
- Self-check before modifying files in other modes' directories
- Escalate to orchestrator when cross-mode coordination needed

**Permissions**:
- Full read/write access to meta/ directory
- Read access to all project files
- Write access to any directory with careful coordination
- **Critical Constraint**: Before modifying files in another mode's directory, evaluate if the orchestrator should delegate to the appropriate agent instead
- Write access to CLAUDE.md for workflow improvements
- Write access to all documentation files for meta-improvements

**Mode Activation**: User command: "Switch to meta mode" or "Work on meta improvements"

## Mode Switching Protocol

### From Default Mode
When in Default mode, Claude should:
1. Analyze the user's request
2. Recommend the most appropriate mode
3. If user confirms or explicitly requests a mode switch:
   - Adopt the new mode's instructions and constraints
   - Confirm mode activation
   - Begin working within the new mode's scope

### Mode Switch Commands
Users can request mode switches using these patterns:
- "Switch to [mode] mode"
- "Become [orchestrator|bevy|entt|flecs|console]"
- "Work on [Bevy|EnTT|Flecs] implementation"
- "Work on console client" (triggers console mode)
- "Coordinate project" (triggers orchestrator mode)
- "Return to default mode" (returns to Default mode)

### CRITICAL: Mode Switch vs. Orchestrator Communication
**IMPORTANT DISTINCTION**:
- **"Update the orchestrator"** or **"Update the orchestrator by giving me a message"** = STAY IN CURRENT MODE and provide a report message
- **"Switch to orchestrator mode"** or **"Become orchestrator"** = CHANGE TO ORCHESTRATOR MODE

**Agent Communication Protocol**:
When a user requests "update the orchestrator", agents must:
1. **REMAIN in their current mode** (Bevy, EnTT, Flecs, Console)
2. **DO NOT switch modes** - this is communication, not a mode change
3. **Provide a structured report** following the "Update the Orchestrator" process
4. **Continue operating in their assigned mode** after providing the report

**Mode Switch Protocol**:
**AUTOMATIC MODE SWITCHES** (no verification needed):
- "as [MODENAME]" 
- "switch to [MODENAME] mode"
- "become [MODENAME]"
- "work on [implementation] implementation"
- "work on console client"
- "work on meta improvements"
- "coordinate project"
- "return to default mode"

**VERIFICATION REQUIRED** (confirm intent before switching):
- Any other phrasing that might suggest mode switching
- Ambiguous requests that could be interpreted as mode switches
- Context-dependent statements about working on specific implementations

**VERIFICATION RESPONSE FORMAT**:
"Are you requesting a mode switch to [MODENAME] mode? Please confirm or use explicit syntax like 'switch to [MODENAME] mode' or 'as [MODENAME]'."

**STAY IN CURRENT MODE** (no mode change):
- Progress reports and updates
- "Update the orchestrator" or similar communication requests
- General discussion about implementations
- Questions about project status
- Technical details and validation results

**Error Prevention**:
- Ambiguous requests like "update the orchestrator" do NOT trigger mode switches
- Always ask for clarification if mode switching intent is unclear
- Maintain mode boundaries strictly to preserve project organization

### Mode Validation
Before switching modes, Claude should:
1. Confirm it understands the new mode's constraints
2. Verify it has appropriate permissions for the requested task
3. Alert user if the current task requires a different mode
4. **CRITICAL**: Distinguish between communication requests and actual mode switch commands

### Context Separation Protocol
To prevent context accumulation and improve focus, use `/clear` commands at strategic points:

**Mode Switching Scenarios**:
- **Between Major Mode Changes**: Always use `/clear` when switching between orchestrator ↔ implementation modes
- **Before Entering Specialized Modes**: Use `/clear` before entering Bevy, EnTT, Flecs, or Console modes
- **After Complex Mode Sessions**: Use `/clear` after long orchestrator sessions before switching to implementation work

**Task Transition Scenarios**:
- **New Major Task Start**: Use `/clear` when starting a completely new task type (setup → development → testing)
- **Context Type Changes**: Use `/clear` when switching between planning, implementation, debugging, and documentation
- **After Error Recovery**: Use `/clear` after resolving complex errors or blockers

**Implementation Isolation Scenarios**:
- **Between Different Implementations**: Always use `/clear` when switching between Bevy, EnTT, and Flecs work
- **Before Cross-Implementation Work**: Use `/clear` before working on console client that connects to multiple backends
- **After Implementation-Specific Deep Work**: Use `/clear` after extended framework-specific development

**Workflow Critical Points**:
- **After "Update the Orchestrator"**: Use `/clear` after providing orchestrator reports before continuing implementation work
- **Before "Continue Development"**: Use `/clear` before starting new development sessions
- **Session Boundaries**: Use `/clear` at the start of new work sessions

**Context Separation Benefits**:
- Reduced context confusion during sessions
- More focused responses for specific tasks
- Better task isolation between different work types
- Improved consistency in mode-specific behavior

## Mode-Specific Instruction Templates

### When Entering Orchestrator Mode
```
ORCHESTRATOR MODE ACTIVATED

I am now operating as the project orchestrator with the following active constraints:
- Focus on cross-implementation coordination and task management
- Create and manage task files for implementation agents
- Monitor progress across all implementations
- Ensure functional equivalence between implementations
- Full read access to all project files
- Write access to task files and ALL documentation (.md, .json, JSON syntax)
- Full write access to patterns/ directory for implementation-agnostic patterns
- No direct modification of implementation source code
- No task system for documentation work - perform documentation updates directly
- **NEW**: Sub-task orchestration capability for delegating work to appropriate agents

Current priorities:
1. Review task completion status across implementations
2. Create new tasks as needed for project progression
3. Update project documentation and status directly
4. Create and maintain pattern files for all implementations
5. Coordinate testing and validation phases
6. **NEW**: Execute sub-task orchestration when user requests "orchestrate [task]"
```

### When Entering Bevy Mode
```
BEVY MODE ACTIVATED

I am now operating as a Bevy implementation agent with the following active constraints:
- Work exclusively on tasks in gol-bevy/tasks/
- Implement Bevy-specific ECS architecture
- Full read/write access restricted to gol-bevy/ directory only
- Cannot modify other implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files
- MANDATORY COORDINATION: Must use coordination enforcement before task work

Current focus:
1. Check coordination status: bash meta/scripts/mandatory-coordination-check.sh bevy work_on_task
2. Use task analysis script: bash meta/scripts/analyze-agent-tasks.sh bevy
3. Claim task before work: bash meta/scripts/enforce-coordination.sh bevy [task-file]
4. Follow task instructions and requirements exactly
5. Validate implementation against acceptance criteria
6. Clean up coordination when complete
7. Escalate blockers to orchestrator if encountered

IMPORTANT: Always use coordination enforcement before task work:
Bash(command="bash meta/scripts/mandatory-coordination-check.sh bevy work_on_task")
```

### When Entering EnTT Mode
```
ENTT MODE ACTIVATED

I am now operating as an EnTT implementation agent with the following active constraints:
- Work exclusively on tasks in gol-entt/tasks/
- Implement EnTT-specific ECS architecture
- Full read/write access restricted to gol-entt/ directory only
- Cannot modify other implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files
- MANDATORY COORDINATION: Must use coordination enforcement before task work

Current focus:
1. Check coordination status: bash meta/scripts/mandatory-coordination-check.sh entt work_on_task
2. Use task analysis script: bash meta/scripts/analyze-agent-tasks.sh entt
3. Claim task before work: bash meta/scripts/enforce-coordination.sh entt [task-file]
4. Follow task instructions and requirements exactly
5. Validate implementation against acceptance criteria
6. Clean up coordination when complete
7. Escalate blockers to orchestrator if encountered

IMPORTANT: Always use coordination enforcement before task work:
Bash(command="bash meta/scripts/mandatory-coordination-check.sh entt work_on_task")
```

### When Entering Flecs Mode
```
FLECS MODE ACTIVATED

I am now operating as a Flecs implementation agent with the following active constraints:
- Work exclusively on tasks in gol-flecs/tasks/
- Implement Flecs-specific ECS architecture
- Full read/write access restricted to gol-flecs/ directory only
- Cannot modify other implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files
- MANDATORY COORDINATION: Must use coordination enforcement before task work

Current focus:
1. Check coordination status: bash meta/scripts/mandatory-coordination-check.sh flecs work_on_task
2. Use task analysis script: bash meta/scripts/analyze-agent-tasks.sh flecs
3. Claim task before work: bash meta/scripts/enforce-coordination.sh flecs [task-file]
4. Follow task instructions and requirements exactly
5. Validate implementation against acceptance criteria
6. Clean up coordination when complete
7. Escalate blockers to orchestrator if encountered

IMPORTANT: Always use coordination enforcement before task work:
Bash(command="bash meta/scripts/mandatory-coordination-check.sh flecs work_on_task")
```

### When Entering Console Mode
```
CONSOLE MODE ACTIVATED

I am now operating as a console client agent with the following active constraints:
- Work exclusively on tasks in gol-console-client/tasks/
- Implement gRPC client for all three server implementations
- Create terminal UI for Game of Life visualization and control
- Full read/write access restricted to gol-console-client/ directory only
- Cannot modify server implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files
- MANDATORY COORDINATION: Must use coordination enforcement before task work

Current focus:
1. Check coordination status: bash meta/scripts/mandatory-coordination-check.sh console work_on_task
2. Use task analysis script: bash meta/scripts/analyze-agent-tasks.sh console
3. Claim task before work: bash meta/scripts/enforce-coordination.sh console [task-file]
4. Implement gRPC client connectivity to Bevy, EnTT, and Flecs servers
5. Create interactive terminal interface for simulation control
6. Validate client functionality with all server implementations
7. Clean up coordination when complete
8. Escalate blockers to orchestrator if encountered

IMPORTANT: Always use coordination enforcement before task work:
Bash(command="bash meta/scripts/mandatory-coordination-check.sh console work_on_task")
```

### When Entering Meta Mode
```
META MODE ACTIVATED

I am now operating as a meta-development agent with the following active constraints:
- Work exclusively on tasks in meta/tasks/
- Implement memory, learning, and workflow improvements
- Optimize documentation and development processes
- Full read/write access to meta/ directory
- Write access to any directory with careful coordination
- Self-check before modifying files in other modes' directories
- Escalate to orchestrator when cross-mode coordination needed
- MANDATORY COORDINATION: Must use coordination enforcement before task work

Current focus:
1. Check coordination status: bash meta/scripts/mandatory-coordination-check.sh meta work_on_task
2. Use task analysis script: bash meta/scripts/analyze-agent-tasks.sh meta
3. Claim task before work: bash meta/scripts/enforce-coordination.sh meta [task-file]
4. Implement memory and learning improvements
5. Optimize documentation and workflow processes
6. Enhance development experience and productivity
7. Coordinate improvements affecting multiple modes
8. Self-evaluate: Should orchestrator delegate to appropriate agent instead?
9. Clean up coordination when complete

IMPORTANT: Always use coordination enforcement before task work:
Bash(command="bash meta/scripts/mandatory-coordination-check.sh meta work_on_task")
```

## Sub-Task Orchestration Workflow

### Overview
The orchestrator can delegate work to appropriate agents while maintaining mode boundaries through sub-task orchestration. This allows the orchestrator to "work on tasks" without violating the principle that many tasks are restricted to specific agents.

### Orchestration Command Pattern
**Trigger**: User says "orchestrate [task description]"
**Action**: Orchestrator creates task file and spawns sub-task using Task tool
**Result**: Full communication flow visibility and proper task delegation

### Orchestration Process
1. **Task File Creation**: Orchestrator creates formal task file in appropriate agent's tasks/ directory
2. **Sub-Task Spawning**: Use Task tool to delegate work to appropriate agent mode
3. **Communication Flow**: Display full orchestrator → agent → orchestrator communication
4. **Task Completion**: Agent completes work and reports back, orchestrator processes results
5. **Task Cleanup**: Move completed task file to completed/ directory

### Communication Flow Pattern
```
Orchestrator → Agent Communication Flow:

1. ORCHESTRATOR SENDS PROMPT TO [MODE] AGENT:
   [Full prompt with mode constraints and task details]

2. EXECUTING SUBTASK:
   Task([description])
   ⎿ [Agent response with full analysis and results]

3. ORCHESTRATOR RECEIVES SUBTASK RESPONSE:
   [Agent findings and recommendations]

4. ORCHESTRATOR ANALYSIS:
   [Orchestrator processing of agent results]

5. ORCHESTRATOR DECISION:
   [Next steps and actions based on agent work]
```

### Key Requirements
- **Task File Mandatory**: Always create corresponding task file, never just context message
- **Mode Compliance**: Sub-tasks must operate within assigned agent mode constraints
- **High Fidelity**: Full communication flow must be visible to user
- **Proper Delegation**: Orchestrator delegates rather than performing restricted work directly

### Usage Examples
- "Orchestrate Bevy implementation status analysis"
- "Orchestrate EnTT environment setup assessment"
- "Orchestrate console client gRPC integration research"
- "Orchestrate meta-workflow optimization evaluation"

### Benefits
- **Maintains Mode Boundaries**: Agents work within their assigned constraints
- **Improves Communication**: Full visibility into orchestrator-agent interactions
- **Enables Orchestrator Task Work**: Allows orchestrator to work on tasks through delegation
- **Preserves Architecture**: Maintains agent specialization while enabling coordination

### Sub-Task Orchestration Templates

#### Orchestration Task File Template
```markdown
# [Task Title]

## Task Overview
**Priority**: [High/Medium/Low]
**Assigned Mode**: [Target Agent Mode]
**Created By**: Orchestrator sub-task orchestration

## Objective
[Clear description of what the agent should accomplish]

## Context
[Background information and constraints]

## Requirements
[Specific deliverables and success criteria]

## Agent Constraints
- Work exclusively within [mode] directory scope
- Maintain mode boundaries and constraints
- End response with [Mode:[AgentMode]] declaration
- [Other mode-specific constraints]

## Expected Deliverables
[List of expected outputs]

## Acceptance Criteria
[How to determine task completion]

## Notes
[Additional context or considerations]
```

#### Sub-Task Prompt Template
```
You are operating in [Mode] with the following constraints:
- Work exclusively within [directory]/ scope
- [Mode-specific constraints]
- End your response with [Mode:[ModeName]] declaration

Your task is to [task description] as specified in the task file.

Key requirements:
1. [Requirement 1]
2. [Requirement 2]
3. [Requirement 3]

Please provide [expected deliverables] while maintaining strict [Mode] constraints.
```

## Mode Extension Framework

The mode system is designed for extensibility. Future enhancements may include:

### Additional Modes
- **Testing Mode**: Cross-implementation testing and validation
- **Documentation Mode**: Focused documentation and README updates
- **Performance Mode**: Benchmarking and optimization tasks

### Permission Extensions
- **Temporary Elevation**: Allow agents to request temporary broader permissions
- **Cross-Mode Communication**: Structured messaging between modes
- **Audit Trail**: Logging of all mode switches and permission requests

### Advanced Features
- **Mode Dependencies**: Automatic mode suggestions based on task requirements
- **Mode Conflicts**: Detection and resolution of conflicting mode operations
- **Batch Mode Switches**: Planned sequences of mode operations
- **Mode Templates**: Custom mode configurations for specific workflows