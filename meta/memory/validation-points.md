# Validation Points Framework

## Overview
This document defines specific validation checkpoints integrated into existing workflows to prevent documentation-reality mismatches and ensure system reliability.

## Core Validation Points

### 1. Mode Switch Validation Points

#### Pre-Mode Switch Validation
**Trigger**: Before any mode switch command execution
**Process**:
```bash
# Pre-mode switch validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/modes.md
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/CLAUDE.md
```

**Validation Checks**:
- Current mode documentation matches actual behavior
- Mode constraints are being followed correctly
- Mode-specific files have not drifted from baselines
- Command patterns are up-to-date and functional

**Failure Actions**:
- Log validation failure with mode context
- Provide mode-specific guidance for resolution
- Escalate to meta mode if systematic issue detected

#### Post-Mode Switch Validation
**Trigger**: After successful mode switch completion
**Process**:
```bash
# Post-mode switch validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh post-check /mnt/c/dev/gol/meta/docs/modes.md "mode-switch" "Mode switch validation"
```

**Validation Checks**:
- New mode activated correctly
- Mode-specific constraints are active
- Mode identification string is correct
- Access permissions match mode specifications

### 2. Task Initiation Validation Points

#### Pre-Task Validation
**Trigger**: Before starting any task (via "continue development" or explicit task start)
**Process**:
1. **Task Directory Validation**:
   ```bash
   # Validate task directory structure
   ls -la {current_mode}/tasks/
   /mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/tasks.md
   ```

2. **Requirements Validation**:
   ```bash
   # Validate requirements and architecture docs
   /mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/REQUIREMENTS.md
   /mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/ARCHITECTURE.md
   ```

3. **Mode-Specific Documentation Validation**:
   ```bash
   # Validate mode-specific constraints
   /mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/modes.md
   ```

**Validation Checks**:
- Task file exists and is accessible
- Requirements documentation is current
- Architecture documentation reflects current state
- Mode permissions allow task execution
- No conflicting tasks in progress

#### Post-Task Validation
**Trigger**: After task completion or significant progress
**Process**:
```bash
# Post-task validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh post-check {modified_files} "task-completion" "Task completion validation"
```

**Validation Checks**:
- Task completion criteria met
- Documentation updates reflect changes
- No unintended side effects in other files
- Mode boundaries respected during task execution

### 3. Orchestrator Communication Validation Points

#### Pre-"Update Orchestrator" Validation
**Trigger**: Before providing orchestrator reports
**Process**:
```bash
# Pre-orchestrator update validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/commands.md
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/modes.md
```

**Validation Checks**:
- Current mode is correct for communication
- Mode boundaries are being respected
- Communication patterns match documentation
- Agent is staying in assigned mode (not switching)

#### Post-"Update Orchestrator" Validation
**Trigger**: After providing orchestrator report
**Process**:
```bash
# Post-orchestrator update validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh post-check /mnt/c/dev/gol/meta/docs/modes.md "orchestrator-communication" "Orchestrator communication validation"
```

**Validation Checks**:
- Agent remained in original mode
- Communication followed proper protocol
- No mode confusion occurred
- Report format matches expected patterns

### 4. Development Command Validation Points

#### Pre-"Continue Development" Validation
**Trigger**: Before executing "continue development" command
**Process**:
```bash
# Pre-development validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/development.md
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/tasks.md
```

**Validation Checks**:
- Development guidelines are current
- Task management protocols are up-to-date
- Permission protocols are documented correctly
- Quality assurance requirements are current

#### Post-Development Validation
**Trigger**: After development work completion
**Process**:
```bash
# Post-development validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh post-check {modified_files} "development-completion" "Development completion validation"
```

**Validation Checks**:
- Development followed documented protocols
- Quality checks were performed
- Permission protocols were followed
- Documentation updates reflect reality

### 5. Maintenance Command Validation Points

#### Pre-"Weekly Maintenance" Validation
**Trigger**: Before executing weekly maintenance (orchestrator mode only)
**Process**:
```bash
# Pre-maintenance validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh weekly
```

**Validation Checks**:
- All core documentation files validated
- Baseline integrity confirmed
- System-wide validation status confirmed
- Maintenance protocols are current

#### Post-Maintenance Validation
**Trigger**: After maintenance completion
**Process**:
```bash
# Post-maintenance validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh post-check {updated_files} "maintenance-completion" "Maintenance completion validation"
```

**Validation Checks**:
- Maintenance tasks completed successfully
- Documentation updates are accurate
- Baseline updates are consistent
- System stability maintained

### 6. Error Recovery Validation Points

#### Pre-Error Recovery Validation
**Trigger**: Before executing error recovery commands
**Process**:
```bash
# Pre-error recovery validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/commands.md
/mnt/c/dev/gol/meta/memory/validate-memory.sh validate /mnt/c/dev/gol/meta/docs/modes.md
```

**Validation Checks**:
- Error recovery protocols are current
- Mode recovery procedures are documented
- Command patterns are functional
- Context separation protocols are clear

#### Post-Error Recovery Validation
**Trigger**: After error recovery completion
**Process**:
```bash
# Post-error recovery validation
/mnt/c/dev/gol/meta/memory/validate-memory.sh post-check {recovered_files} "error-recovery" "Error recovery validation"
```

**Validation Checks**:
- System returned to stable state
- Error patterns documented for prevention
- Documentation reflects recovery actions
- No lingering issues remain

## Implementation Integration

### Command Pattern Integration
Each command pattern should include validation checkpoints:

```json
{
  "command_patterns": {
    "mode_management": {
      "pre_validation": ["validate_modes", "validate_claude_md"],
      "post_validation": ["update_baseline", "confirm_mode_switch"]
    },
    "development": {
      "pre_validation": ["validate_development_docs", "validate_tasks"],
      "post_validation": ["update_modified_files", "confirm_completion"]
    },
    "maintenance": {
      "pre_validation": ["weekly_validation_sweep"],
      "post_validation": ["update_maintenance_baseline"]
    }
  }
}
```

### Mode-Specific Integration
Each mode should implement validation points appropriate to its scope:

#### Orchestrator Mode Validation
- Full documentation validation before coordination tasks
- Cross-implementation validation during task creation
- Pattern file validation before updates
- Weekly maintenance validation sweep

#### Implementation Mode Validation (Bevy, EnTT, Flecs, Console)
- Task-specific validation before implementation
- Mode boundary validation during execution
- Completion validation before task deletion
- Escalation validation before orchestrator communication

#### Meta Mode Validation
- System-wide validation before meta improvements
- Cross-mode impact validation before modifications
- Permission protocol validation before script creation
- Documentation consistency validation

### Lightweight Implementation Principles

#### Validation Efficiency
- Use cached validation results when possible
- Implement incremental validation for frequent operations
- Batch validation checks to reduce overhead
- Skip validation for read-only operations

#### Validation Granularity
- **File-level validation**: For documentation changes
- **Section-level validation**: For specific documentation updates
- **Command-level validation**: For command pattern changes
- **Mode-level validation**: For mode-specific operations

#### Validation Timing
- **Pre-execution**: Validate before potentially destructive operations
- **Post-execution**: Update baselines after successful operations
- **Periodic**: Regular validation sweeps during maintenance
- **On-demand**: Manual validation when issues suspected

## Validation Categories

### 1. Mode Instructions Validation
**Purpose**: Ensure mode constraints match actual behavior
**Scope**: Mode-specific instruction accuracy
**Triggers**: Mode switches, task initiation, orchestrator communication
**Criteria**:
- Mode permissions correctly implemented
- Mode constraints actively enforced
- Mode activation templates current
- Mode switching protocols functional

### 2. Command Pattern Validation
**Purpose**: Ensure command recognition patterns work as documented
**Scope**: Command execution and recognition
**Triggers**: Command execution, pattern updates, error recovery
**Criteria**:
- Command patterns recognize intended inputs
- Command execution follows documented behavior
- Command fallback mechanisms work correctly
- Command documentation matches implementation

### 3. Task Workflow Validation
**Purpose**: Ensure documented processes match actual usage
**Scope**: Task management and execution
**Triggers**: Task initiation, completion, workflow updates
**Criteria**:
- Task files exist and are accessible
- Task completion criteria are clear
- Task dependencies are resolved
- Task escalation procedures work

### 4. Project Structure Validation
**Purpose**: Ensure documentation reflects actual directory structure
**Scope**: File system organization and access
**Triggers**: Directory operations, file moves, structure changes
**Criteria**:
- Directory structure matches documentation
- File paths are correct and accessible
- Permissions match documented constraints
- Import statements work correctly

### 5. Documentation Consistency Validation
**Purpose**: Ensure cross-references between documents are accurate
**Scope**: Inter-document consistency
**Triggers**: Documentation updates, weekly maintenance
**Criteria**:
- Cross-references point to correct locations
- Imported sections are current
- Documentation hierarchy is logical
- Version consistency across documents

## Success Metrics

### Validation Success Rate
- **Target**: >95% validation success rate
- **Measurement**: Successful validations / Total validations
- **Monitoring**: Daily validation log analysis

### Documentation Drift Detection
- **Target**: Zero undetected drift incidents
- **Measurement**: Manual verification vs. automated detection
- **Monitoring**: Weekly comparison audits

### Workflow Disruption Minimization
- **Target**: <2% additional overhead from validation
- **Measurement**: Execution time with/without validation
- **Monitoring**: Performance benchmarking

### Error Prevention Effectiveness
- **Target**: 50% reduction in documentation-reality mismatches
- **Measurement**: Error incidents before/after validation
- **Monitoring**: Monthly error trend analysis

## Rollback Procedures

### Validation Overhead Issues
1. **Identify bottlenecks**: Profile validation execution times
2. **Optimize validation**: Reduce frequency or scope
3. **Implement caching**: Cache validation results
4. **Fallback option**: Disable validation for specific operations

### Validation False Positives
1. **Analyze patterns**: Identify common false positive triggers
2. **Refine criteria**: Adjust validation thresholds
3. **Update baselines**: Correct baseline inconsistencies
4. **Improve detection**: Enhance validation logic

### System Instability
1. **Immediate rollback**: Disable validation system
2. **Diagnostic mode**: Enable detailed validation logging
3. **Incremental re-enable**: Gradually restore validation
4. **Monitoring increase**: Enhanced system monitoring

## Maintenance Schedule

### Daily Validation
- Automatic validation during normal operations
- Log review for validation failures
- Quick fixes for minor validation issues

### Weekly Validation
- Comprehensive validation sweep
- Baseline update review
- Performance impact assessment
- Documentation accuracy verification

### Monthly Validation
- Validation effectiveness analysis
- Success metrics review
- System optimization opportunities
- Rollback procedure testing

---

**Implementation Status**: Framework defined
**Next Steps**: Create lightweight validation procedures
**Integration Target**: Seamless workflow integration
**Monitoring**: Continuous validation effectiveness tracking