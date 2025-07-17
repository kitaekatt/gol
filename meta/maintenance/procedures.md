# Memory Maintenance Procedures

## Overview
Detailed procedures and checklists for executing memory maintenance protocols across all modes.

## Maintenance Execution Framework

### Command Recognition
When user says maintenance commands, execute immediately:
- **"Run maintenance"** - Execute current mode's maintenance protocol
- **"Weekly maintenance"** - Execute weekly orchestrator maintenance (orchestrator mode only)
- **"Validate documentation"** - Check documentation accuracy against current project state

### Maintenance Type Selection
Based on current mode and context:
- **Daily**: After task completion (implementation agents)
- **Session**: During mode switches (all modes)
- **Weekly**: Orchestrator coordination review (orchestrator mode)
- **Monthly**: Deep review and optimization (meta mode)

## Daily Maintenance Procedures

### Implementation Agent Maintenance (Bevy, EnTT, Flecs, Console)
**Trigger**: After task completion
**Duration**: 2-3 minutes

#### Execution Checklist
1. **Task Documentation Validation**:
   - [ ] Verify completed task file has been deleted
   - [ ] Check if task completion revealed documentation gaps
   - [ ] Note any new patterns or insights discovered
   - [ ] Update implementation README if new patterns emerged

2. **Implementation Notes Review**:
   - [ ] Document any framework-specific gotchas encountered
   - [ ] Record optimal sequences or patterns discovered
   - [ ] Note any deviations from documented architecture
   - [ ] Identify any missing documentation for current task type

3. **Error Pattern Documentation**:
   - [ ] Document any recurring build/test errors and solutions
   - [ ] Note workflow friction points encountered
   - [ ] Record any tool usage patterns that worked well
   - [ ] Update troubleshooting sections if applicable

4. **Workflow Insight Capture**:
   - [ ] Note any command sequences that could be optimized
   - [ ] Identify any repetitive patterns worth documenting
   - [ ] Record any dependencies or prerequisites discovered
   - [ ] Update task templates if improvements identified

#### Quick Validation Commands
Execute these to verify documentation accuracy:
```bash
# Verify task completion
ls {mode}/tasks/ | wc -l

# Check for undocumented patterns
grep -r "TODO\|FIXME\|NOTE" {mode}/src/ || echo "No pending documentation found"

# Validate build/test status
{mode build command} && echo "Build validation passed"
```

### Meta Mode Daily Maintenance
**Trigger**: After meta task completion
**Duration**: 3-5 minutes

#### Execution Checklist
1. **Meta Task Impact Assessment**:
   - [ ] Verify meta changes didn't break existing workflows
   - [ ] Check that documentation imports still work correctly
   - [ ] Validate command recognition patterns still function
   - [ ] Test mode switching after meta changes

2. **Cross-Mode Coordination Check**:
   - [ ] Verify meta changes are compatible with all modes
   - [ ] Check that no mode permissions were inadvertently changed
   - [ ] Validate that task workflows remain intact
   - [ ] Test orchestrator communication patterns

## Session Maintenance Procedures

### Mode Switch Maintenance
**Trigger**: During mode transitions
**Duration**: 1-2 minutes

#### Execution Checklist
1. **Pre-Switch Validation**:
   - [ ] Review target mode's current task availability
   - [ ] Verify target mode has appropriate permissions for intended work
   - [ ] Check for any blockers or dependencies in target mode
   - [ ] Confirm mode switch rationale aligns with project needs

2. **Context Separation Protocol**:
   - [ ] Execute `/clear` if switching between major mode types
   - [ ] Document current mode's status before switching
   - [ ] Prepare context for target mode requirements
   - [ ] Note any cross-mode dependencies or handoffs needed

3. **Post-Switch Confirmation**:
   - [ ] Verify mode constraints are properly loaded
   - [ ] Check task directory accessibility
   - [ ] Validate permissions match mode requirements
   - [ ] Confirm command patterns are active for new mode

## Weekly Maintenance Procedures

### Orchestrator Weekly Review
**Trigger**: Weekly orchestrator maintenance command
**Duration**: 15-20 minutes

#### Execution Checklist
1. **Mode Instructions Accuracy Review**:
   - [ ] Test mode switching for all modes
   - [ ] Verify mode constraints match actual behavior
   - [ ] Check that mode permissions are correctly enforced
   - [ ] Validate mode-specific task execution patterns

2. **Command Pattern Validation**:
   - [ ] Test high-priority command recognition patterns
   - [ ] Verify "continue development" works across modes
   - [ ] Check "update orchestrator" communication flows
   - [ ] Validate maintenance command execution

3. **Task Workflow Verification**:
   - [ ] Review task completion patterns across implementations
   - [ ] Check task creation and delegation workflows
   - [ ] Verify cross-implementation coordination effectiveness
   - [ ] Validate task priority and dependency management

4. **Project Structure Synchronization**:
   - [ ] Compare documented structure with actual directories
   - [ ] Verify all task directories are accessible
   - [ ] Check that documentation references are current
   - [ ] Validate file paths and import statements

5. **Recent Changes Integration**:
   - [ ] Review completed tasks for documentation updates needed
   - [ ] Check for workflow improvements discovered during implementation
   - [ ] Identify any new patterns worth documenting
   - [ ] Update project documentation based on recent insights

#### Weekly Review Commands
Execute these during weekly maintenance:
```bash
# Check all task directories
find /mnt/c/dev/gol -name "tasks" -type d -exec ls -la {} \;

# Verify documentation imports
grep -r "@.*\.md" /mnt/c/dev/gol/CLAUDE.md

# Check for mode-specific issues
grep -r "Mode:" /mnt/c/dev/gol/meta/docs/

# Validate project structure
tree /mnt/c/dev/gol -I "*build*|*target*|*node_modules*"
```

## Monthly Deep Review Procedures

### Meta Mode Monthly Review
**Trigger**: Monthly deep review in meta mode
**Duration**: 30-45 minutes

#### Execution Checklist
1. **Documentation Effectiveness Analysis**:
   - [ ] Review documentation usage patterns from recent sessions
   - [ ] Identify sections that caused confusion or errors
   - [ ] Analyze command usage frequency and effectiveness
   - [ ] Check for documentation that's become outdated

2. **Workflow Optimization Assessment**:
   - [ ] Identify frequently used but undocumented patterns
   - [ ] Note workflow friction points encountered repeatedly
   - [ ] Review maintenance protocol effectiveness
   - [ ] Assess time spent on different maintenance types

3. **Knowledge Gap Identification**:
   - [ ] Find areas where documentation is missing or unclear
   - [ ] Identify recurring questions or confusion points
   - [ ] Note implementation-specific knowledge that could be generalized
   - [ ] Check for cross-mode coordination gaps

4. **Process Improvement Planning**:
   - [ ] Evaluate current maintenance protocol effectiveness
   - [ ] Identify opportunities for automation or streamlining
   - [ ] Plan documentation structure improvements
   - [ ] Design next iteration of maintenance processes

#### Monthly Review Deliverables
- **Maintenance Effectiveness Report**: Analysis of current protocols
- **Documentation Gap Analysis**: Identified areas needing improvement
- **Process Optimization Plan**: Next iteration improvements
- **Knowledge Base Update**: Consolidated insights and patterns

## Maintenance Trigger Procedures

### Immediate Maintenance Triggers
When these conditions are detected, execute immediate maintenance:

1. **Documentation-Reality Mismatch**:
   - Document the mismatch details
   - Update documentation to match current reality
   - Verify fix across affected modes
   - Add validation checkpoint to prevent recurrence

2. **New Command Pattern Addition**:
   - Update command recognition patterns
   - Test command across all relevant modes
   - Document command usage and examples
   - Add to weekly validation checklist

3. **Mode Responsibility Changes**:
   - Update mode documentation immediately
   - Test mode switching with new responsibilities
   - Verify permission changes are correctly implemented
   - Communicate changes to all affected modes

4. **Project Structure Modifications**:
   - Update all references to changed paths
   - Verify imports and file references
   - Test task directory accessibility
   - Update project structure documentation

## Success Measurement

### Maintenance Effectiveness Metrics
Track these indicators to measure maintenance success:

1. **Reduced Confusion Incidents**:
   - Fewer mode switching errors
   - Reduced command recognition failures
   - Less documentation-reality mismatches

2. **Improved Task Execution Speed**:
   - Faster task completion times
   - Reduced time spent on documentation lookups
   - More consistent workflow execution

3. **Documentation Quality Indicators**:
   - Fewer outdated documentation references
   - More accurate command patterns
   - Better alignment between documentation and behavior

4. **Workflow Efficiency Measures**:
   - Reduced maintenance overhead
   - More effective cross-mode coordination
   - Better task delegation and completion patterns

### Maintenance Protocol Optimization
Continuously improve maintenance based on:
- Time spent on different maintenance types
- Effectiveness of different validation approaches
- User feedback on documentation clarity
- Frequency of different maintenance triggers