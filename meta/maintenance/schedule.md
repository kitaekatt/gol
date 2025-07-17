# Memory Maintenance Schedule

## Overview
Regular maintenance schedule to prevent documentation drift and keep project documentation current, concise, and accurate.

## Weekly Maintenance (Orchestrator)
**Frequency**: Every 7 days or after major project milestones
**Duration**: 15-20 minutes
**Responsibility**: Orchestrator mode

### Review Checklist
- [ ] **Mode Instructions Accuracy**: Do mode constraints match actual behavior?
- [ ] **Command Pattern Validation**: Test key command recognition patterns
- [ ] **Task Workflow Verification**: Ensure documented processes match actual usage
- [ ] **Project Structure Sync**: Verify documentation reflects actual directory structure
- [ ] **Recent Changes Integration**: Review completed tasks for documentation updates needed

### Key Review Areas
1. **CLAUDE.md Core Sections**:
   - Mode definitions and constraints
   - Command recognition patterns
   - Task management workflows
   - Mode switching protocols

2. **Implementation Status**:
   - Task completion accuracy across all modes
   - Cross-implementation coordination effectiveness
   - Documentation gaps identified during recent work

3. **Workflow Efficiency**:
   - Identify frequently used but undocumented patterns
   - Note workflow friction points for improvement
   - Validate that shortcuts and optimizations are documented

## Daily Maintenance (Task Completion)
**Frequency**: After each task completion
**Duration**: 2-3 minutes
**Responsibility**: Implementation agents (Bevy, EnTT, Flecs, Console)

### Quick Validation Steps
- [ ] **Task Documentation**: Verify task completion updates are accurate
- [ ] **Implementation Notes**: Check that new patterns are documented
- [ ] **Error Patterns**: Document any recurring issues or solutions
- [ ] **Workflow Insights**: Note any improvements discovered during task execution

## Session Maintenance (Mode Switching)
**Frequency**: During mode switches
**Duration**: 1-2 minutes
**Responsibility**: All modes

### Mode Switch Validation
- [ ] **Mode Instructions**: Review mode constraints before activation
- [ ] **Context Separation**: Use `/clear` at strategic transition points
- [ ] **Task Availability**: Verify tasks exist in target mode directory
- [ ] **Permission Verification**: Confirm mode has appropriate access for intended work

## Monthly Deep Review (Meta Mode)
**Frequency**: Every 30 days
**Duration**: 30-45 minutes
**Responsibility**: Meta mode

### Comprehensive Review Areas
- [ ] **Documentation Effectiveness**: Analyze documentation usage patterns
- [ ] **Workflow Optimization**: Identify process improvements
- [ ] **Knowledge Gaps**: Find areas where documentation is missing or unclear
- [ ] **Maintenance Protocol Review**: Evaluate and improve maintenance processes themselves

### Deep Review Process
1. **Usage Analysis**: Review recent sessions for documentation accuracy
2. **Gap Identification**: Find knowledge gaps that caused confusion or errors
3. **Process Optimization**: Streamline frequently used workflows
4. **Future Planning**: Identify documentation improvements for next iteration

## Maintenance Triggers
**Immediate maintenance required when**:
- Documentation-reality mismatch discovered
- New command patterns added
- Mode responsibilities change
- Project structure modifications
- Workflow process changes

## Success Metrics
- **Reduced Confusion**: Fewer sessions with mode or command confusion
- **Faster Task Execution**: Improved task completion times
- **Fewer Documentation Gaps**: Reduced instances of missing or incorrect documentation
- **Improved Workflow Efficiency**: More consistent and predictable behavior across sessions

## Integration Points
- **Task Completion**: Documentation validation as part of task completion
- **Orchestrator Updates**: Include maintenance items in orchestrator communication
- **Mode Switching**: Quick validation during mode transitions
- **Weekly Coordination**: Dedicated maintenance time in orchestrator schedule