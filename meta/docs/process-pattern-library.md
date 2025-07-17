# Process Pattern Library

## Overview

This document provides a comprehensive catalog of workflow patterns for intelligent process recognition in our agent system. Each pattern includes triggers, implementation details, confidence metrics, and usage guidelines.

## Pattern Library Structure

### Pattern Definition Format
```markdown
## Pattern Name
**Category**: [Work Discovery | Execution | Coordination | Information | Management]
**Confidence**: [High | Medium | Low] (0.0-1.0)
**Scope**: [Single Agent | Cross-Agent | System-Wide | Mode-Specific]
**Automation**: [Fully Automated | Semi-Automated | Manual | Adaptive]

### Triggers
- Primary: ["exact phrase", "alternative phrase"]
- Secondary: ["similar phrase", "related phrase"]
- Context: ["contextual trigger", "state-dependent trigger"]

### Implementation
```bash
# Command or script to execute
```

### Success Criteria
- Measurable outcome 1
- Measurable outcome 2

### Usage Notes
- Important considerations
- Common pitfalls
- Best practices
```

## Work Discovery Patterns

### Task Discovery Pattern
**Category**: Work Discovery
**Confidence**: High (0.9)
**Scope**: Single Agent
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["work on something", "continue development", "what's next", "keep going"]
- **Secondary**: ["start working", "begin work", "next task", "continue"]
- **Context**: ["after completing a task", "when entering a mode", "during idle state"]

#### Implementation
```bash
# Execute task analysis script for current agent mode
Bash(command="bash meta/scripts/analyze-agent-tasks.sh [mode]")
```

#### Success Criteria
- Task analysis completed successfully
- Available tasks identified and prioritized
- Highest priority task selected for execution
- Agent begins work on selected task

#### Usage Notes
- Most common pattern across all agent modes
- Should be first pattern checked for work-related requests
- Always use the standardized task analysis script
- Fallback to manual task selection if script fails

### Environment Assessment Pattern
**Category**: Work Discovery
**Confidence**: Medium (0.6)
**Scope**: Single Agent
**Automation**: Semi-Automated

#### Triggers
- **Primary**: ["set up", "prepare environment", "check setup", "validate setup"]
- **Secondary**: ["environment ready", "setup complete", "dependencies installed"]
- **Context**: ["before starting new implementation", "after repository clone", "during initialization"]

#### Implementation
```bash
# Mode-specific environment validation
# Bevy: Check Rust toolchain, cargo build
# EnTT: Check C++ compiler, CMake setup
# Flecs: Check C++ compiler, Flecs dependencies
# Console: Check gRPC tools, protoc compiler
# Meta: Check shell environment, script permissions
```

#### Success Criteria
- Environment requirements validated
- Missing dependencies identified
- Setup instructions provided if needed
- Environment ready for development

#### Usage Notes
- Varies significantly by implementation mode
- May require user intervention for dependency installation
- Should be run before major development tasks
- Results should be cached to avoid repeated checks

### Work Prioritization Pattern
**Category**: Work Discovery
**Confidence**: High (0.8)
**Scope**: Single Agent
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["what should I work on", "highest priority", "most important"]
- **Secondary**: ["prioritize tasks", "urgent tasks", "critical work"]
- **Context**: ["multiple tasks available", "after completing current task", "during planning phase"]

#### Implementation
```bash
# Execute task analysis with priority focus
Bash(command="bash meta/scripts/analyze-agent-tasks.sh [mode] | grep -E 'HIGH PRIORITY|RECOMMENDED'")
```

#### Success Criteria
- Tasks prioritized by importance and urgency
- Highest priority task identified
- Rationale provided for priority assignment
- Clear recommendation given

#### Usage Notes
- Builds on task discovery pattern
- Uses standardized priority system (High/Medium/Low)
- Should consider coordination conflicts
- Can be combined with other work discovery patterns

## Execution Patterns

### Implementation Pattern
**Category**: Execution
**Confidence**: Medium (0.7)
**Scope**: Single Agent
**Automation**: Semi-Automated

#### Triggers
- **Primary**: ["implement", "build", "create", "develop"]
- **Secondary**: ["code", "write", "construct", "make"]
- **Context**: ["after task selection", "with clear requirements", "during development phase"]

#### Implementation
```bash
# Follow documentation-first development process
# 1. Read requirements and architecture documentation
# 2. Design implementation approach
# 3. Write code following project conventions
# 4. Update documentation during implementation
# 5. Run tests and quality checks
```

#### Success Criteria
- Requirements analyzed and understood
- Implementation plan created
- Code written following project conventions
- Documentation updated in real-time
- Tests passing and quality gates met

#### Usage Notes
- Requires clear requirements and specifications
- Must follow documentation-first process
- Should include real-time documentation updates
- Requires quality validation before completion

### Testing Pattern
**Category**: Execution
**Confidence**: High (0.8)
**Scope**: Single Agent
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["test", "validate", "verify", "check"]
- **Secondary**: ["run tests", "quality check", "validation"]
- **Context**: ["after implementation", "before task completion", "during CI/CD"]

#### Implementation
```bash
# Run mode-specific testing commands
# Bevy: cargo test
# EnTT: make test or ctest
# Flecs: make test or ctest
# Console: go test or npm test
# Meta: bash test scripts
```

#### Success Criteria
- All tests executed successfully
- Test coverage maintained or improved
- No regressions introduced
- Quality gates passed

#### Usage Notes
- Should be run before marking tasks complete
- Test failures should block task completion
- New functionality should include new tests
- Test results should be documented

### Documentation Pattern
**Category**: Execution
**Confidence**: High (0.9)
**Scope**: Single Agent or Cross-Agent
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["document", "update docs", "create documentation"]
- **Secondary**: ["write docs", "documentation", "readme"]
- **Context**: ["during implementation", "after completing feature", "before task completion"]

#### Implementation
```bash
# Follow documentation-first process
# 1. Update implementation-specific README
# 2. Update architecture documentation if needed
# 3. Update API documentation for changes
# 4. Verify documentation matches implementation
```

#### Success Criteria
- Documentation updated in real-time
- All implementation changes documented
- Documentation matches actual implementation
- No documentation gaps exist

#### Usage Notes
- Should be done during implementation, not after
- Follow documentation-first process strictly
- Update multiple documentation types as needed
- Verify documentation currency before completion

## Coordination Patterns

### Mode Switching Pattern
**Category**: Coordination
**Confidence**: High (0.9)
**Scope**: System-Wide
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["switch to * mode", "work on * implementation", "become *"]
- **Secondary**: ["as *", "change to *", "enter * mode"]
- **Context**: ["switching between tasks", "different work types", "coordination needs"]

#### Implementation
```bash
# Mode switching protocol
# 1. Validate requested mode and permissions
# 2. Check current task status and completion
# 3. Prepare context for new mode
# 4. Activate mode-specific constraints
# 5. Execute mode-specific initialization
```

#### Success Criteria
- Mode successfully switched
- Mode-specific constraints activated
- Context prepared for new mode
- Previous mode cleanly exited

#### Usage Notes
- Distinguish from orchestrator communication
- Use /clear for complex mode switches
- Validate permissions before switching
- Prepare context for optimal mode entry

### Agent Communication Pattern
**Category**: Coordination
**Confidence**: High (0.8)
**Scope**: Cross-Agent
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["update orchestrator", "report status", "escalate"]
- **Secondary**: ["communicate", "report", "inform"]
- **Context**: ["task completion", "blocker encountered", "status request"]

#### Implementation
```bash
# Structured communication protocol
# 1. Gather current status and context
# 2. Format structured report
# 3. Communicate through appropriate channel
# 4. Remain in current mode (no mode switch)
```

#### Success Criteria
- Status accurately reported
- Communication clear and structured
- Receiver acknowledges receipt
- Agent remains in current mode

#### Usage Notes
- Do NOT switch modes during communication
- Use structured report format
- Include context and next steps
- Distinguish from mode switching requests

### Sub-Task Orchestration Pattern
**Category**: Coordination
**Confidence**: Medium (0.7)
**Scope**: Cross-Agent
**Automation**: Semi-Automated

#### Triggers
- **Primary**: ["orchestrate *", "orchestrate task *", "orchestrate work on *"]
- **Secondary**: ["delegate *", "assign *", "coordinate *"]
- **Context**: ["orchestrator mode only", "complex task delegation", "cross-agent work"]

#### Implementation
```bash
# Orchestration workflow
# 1. Create formal task file in appropriate agent directory
# 2. Use Task tool to delegate work to appropriate agent
# 3. Display full communication flow
# 4. Process agent response and results
# 5. Move completed task to completed/ directory
```

#### Success Criteria
- Task file created successfully
- Work delegated to appropriate agent
- Full communication flow visible
- Results processed and documented

#### Usage Notes
- Only available in orchestrator mode
- Always create corresponding task file
- Use Task tool for delegation
- Maintain full communication visibility

## Information Patterns

### Status Reporting Pattern
**Category**: Information
**Confidence**: High (0.8)
**Scope**: Single Agent or Cross-Agent
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["status", "progress", "where are we", "current state"]
- **Secondary**: ["update", "report", "situation", "progress report"]
- **Context**: ["project check-in", "before planning", "during coordination"]

#### Implementation
```bash
# Context-aware status gathering
# 1. Gather current task status
# 2. Analyze recent progress
# 3. Identify blockers and next steps
# 4. Format structured status report
```

#### Success Criteria
- Current status accurately reported
- Progress clearly summarized
- Blockers identified and documented
- Next steps clearly defined

#### Usage Notes
- Should be context-aware and mode-specific
- Include quantitative progress metrics
- Identify blockers and dependencies
- Provide clear next steps

### Task Listing Pattern
**Category**: Information
**Confidence**: High (0.9)
**Scope**: Single Agent
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["list tasks", "show tasks", "available tasks"]
- **Secondary**: ["tasks", "what tasks", "task list"]
- **Context**: ["planning phase", "work discovery", "status inquiry"]

#### Implementation
```bash
# Task enumeration by agent mode
# Implementation agents: List only current mode tasks
# Orchestrator: List all tasks across agents
# Default: Show overview with mode recommendations
```

#### Success Criteria
- All relevant tasks listed
- Tasks organized by priority
- Task status clearly indicated
- Mode-appropriate scope applied

#### Usage Notes
- Scope depends on current agent mode
- Use task analysis script for consistency
- Include priority and status information
- Provide mode switching guidance when appropriate

### Command Reference Pattern
**Category**: Information
**Confidence**: High (0.9)
**Scope**: System-Wide
**Automation**: Fully Automated

#### Triggers
- **Primary**: ["commands", "help", "what can I do"]
- **Secondary**: ["available commands", "command list", "options"]
- **Context**: ["new user", "forgotten command", "capability inquiry"]

#### Implementation
```bash
# Display command reference from documentation
# Read meta/docs/commands.md and present formatted command list
```

#### Success Criteria
- Complete command reference displayed
- Commands organized by category
- Usage examples provided
- Mode-specific commands highlighted

#### Usage Notes
- Should be comprehensive and up-to-date
- Include usage examples and syntax
- Organize by functional category
- Highlight mode-specific commands

## Management Patterns

### Session Management Pattern
**Category**: Management
**Confidence**: High (0.8)
**Scope**: System-Wide
**Automation**: Semi-Automated

#### Triggers
- **Primary**: ["shutdown", "end session", "prepare shutdown"]
- **Secondary**: ["finish", "wrap up", "session end"]
- **Context**: ["end of work session", "before major change", "clean exit"]

#### Implementation
```bash
# Shutdown process execution
# 1. Verify task completion status
# 2. Validate documentation currency
# 3. Run quality checks if applicable
# 4. Prepare handoff documentation
# 5. Execute clean shutdown sequence
```

#### Success Criteria
- All active tasks at logical stopping points
- Documentation is current and accurate
- Quality gates passed
- Handoff documentation prepared

#### Usage Notes
- Should follow proper shutdown sequence
- Include documentation currency check
- Run quality validation before shutdown
- Prepare clear handoff for next session

### Memory Maintenance Pattern
**Category**: Management
**Confidence**: Medium (0.7)
**Scope**: System-Wide
**Automation**: Semi-Automated

#### Triggers
- **Primary**: ["maintenance", "memory", "validate documentation"]
- **Secondary**: ["check memory", "update memory", "maintenance protocol"]
- **Context**: ["scheduled maintenance", "documentation drift", "system optimization"]

#### Implementation
```bash
# Memory maintenance workflow
# 1. Validate documentation currency
# 2. Check for documentation drift
# 3. Update memory based on recent sessions
# 4. Optimize documentation organization
# 5. Validate system consistency
```

#### Success Criteria
- Documentation validated against reality
- Memory updated with recent insights
- Documentation organization optimized
- System consistency verified

#### Usage Notes
- Should be run regularly to prevent drift
- Focus on documentation-reality alignment
- Capture insights from recent sessions
- Optimize for future session efficiency

### Error Recovery Pattern
**Category**: Management
**Confidence**: Medium (0.6)
**Scope**: System-Wide
**Automation**: Semi-Automated

#### Triggers
- **Primary**: ["reset", "recover", "fix error", "start over"]
- **Secondary**: ["error", "problem", "issue", "troubleshoot"]
- **Context**: ["system error", "workflow failure", "unexpected state"]

#### Implementation
```bash
# Error recovery workflow
# 1. Analyze error context and symptoms
# 2. Identify root cause if possible
# 3. Determine appropriate recovery strategy
# 4. Execute recovery procedure
# 5. Validate system state after recovery
```

#### Success Criteria
- Error analyzed and understood
- Root cause identified if possible
- Recovery strategy successfully executed
- System returned to stable state

#### Usage Notes
- Requires careful analysis of error context
- May need user intervention for complex issues
- Should include prevention recommendations
- Validate system state after recovery

## Pattern Combinations and Workflows

### Common Pattern Sequences

#### Development Workflow
```
1. Task Discovery Pattern → identify available work
2. Work Prioritization Pattern → select highest priority
3. Implementation Pattern → execute development work
4. Testing Pattern → validate implementation
5. Documentation Pattern → update documentation
6. Status Reporting Pattern → communicate completion
```

#### Coordination Workflow
```
1. Status Reporting Pattern → gather current state
2. Agent Communication Pattern → report to orchestrator
3. Sub-Task Orchestration Pattern → delegate new work
4. Mode Switching Pattern → switch to appropriate mode
5. Task Discovery Pattern → begin new work
```

#### Session Management Workflow
```
1. Status Reporting Pattern → assess current state
2. Documentation Pattern → ensure documentation currency
3. Memory Maintenance Pattern → update system memory
4. Session Management Pattern → execute proper shutdown
```

### Pattern Optimization Rules

#### High-Confidence Patterns
- Execute immediately without confirmation
- Log usage for performance optimization
- Provide predictive suggestions based on context
- Enable full automation with quality gates

#### Medium-Confidence Patterns
- Request confirmation for ambiguous cases
- Provide preview of intended actions
- Enable semi-automated execution
- Learn from user feedback

#### Low-Confidence Patterns
- Always request explicit confirmation
- Provide detailed explanation of actions
- Enable manual override at all steps
- Focus on learning and improvement

## Pattern Performance Metrics

### Success Rate Tracking
```json
{
  "pattern_name": {
    "total_executions": 100,
    "successful_executions": 95,
    "success_rate": 0.95,
    "average_confidence": 0.8,
    "user_satisfaction": 0.9
  }
}
```

### Performance Optimization
- **Response Time**: Time from trigger to execution
- **Accuracy**: Percentage of correct pattern matches
- **Efficiency**: Reduction in manual intervention
- **Learning Rate**: Improvement over time

### Quality Assurance
- **Pattern Validation**: Regular testing of pattern execution
- **User Feedback**: Continuous improvement based on usage
- **System Integration**: Compatibility with existing workflows
- **Documentation Currency**: Keeping patterns up-to-date

## Usage Guidelines

### For Implementation Agents
- Focus on work discovery and execution patterns
- Use task analysis script for all work discovery
- Follow documentation-first process strictly
- Report status and escalate blockers appropriately

### For Orchestrator Mode
- Emphasize coordination and management patterns
- Use sub-task orchestration for complex delegation
- Maintain visibility into all agent activities
- Focus on system-wide optimization

### For Meta Mode
- Specialize in system optimization patterns
- Focus on memory maintenance and learning
- Optimize cross-mode pattern effectiveness
- Enhance pattern recognition capabilities

### Universal Guidelines
- Always verify pattern preconditions
- Use appropriate confidence levels
- Log pattern usage for optimization
- Provide fallback to manual processing
- Focus on user experience and efficiency

This pattern library provides the foundation for intelligent process recognition that transforms agents from reactive command executors to proactive process recognizers, enabling true workflow optimization through standardized, measurable, and continuously improving patterns.