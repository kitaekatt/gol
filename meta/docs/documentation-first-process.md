# Documentation-First Process Guidelines

## Core Principle
**Documentation-first means updating documentation DURING task completion, not after.**

When operating correctly:
- **"Is documentation up to date?"** → **"Yes"**
- **"Update documentation"** → **Minimal or no work needed**

## Immediate Documentation Update Triggers

### Task Completion Triggers
- **Task File Creation**: Update PROJECTPLAN.md immediately
- **Task File Deletion**: Update PROJECTPLAN.md immediately  
- **Code Implementation**: Update README.md and architecture docs during implementation
- **Bug Fixes**: Update troubleshooting sections immediately
- **API Changes**: Update API documentation immediately
- **Dependency Changes**: Update dependency documentation immediately

### Mode-Specific Triggers
- **Orchestrator**: Update PROJECTPLAN.md when creating/managing tasks
- **Implementation Agents**: Update implementation-specific READMEs during development
- **Meta Mode**: Update CLAUDE.md and meta/docs/ during process improvements
- **Console Mode**: Update integration docs during gRPC client development

## Task Completion Documentation Checklist

### Before Marking Task Complete
- [ ] Implementation-specific documentation updated
- [ ] PROJECTPLAN.md reflects current task status
- [ ] Architecture decisions documented if applicable
- [ ] README.md updated with new functionality
- [ ] API documentation updated if endpoints changed
- [ ] Troubleshooting section updated if issues encountered

### During Task Execution
- [ ] Document design decisions as they're made
- [ ] Update architecture docs when patterns emerge
- [ ] Record insights that will help future implementations
- [ ] Note framework-specific gotchas and solutions
- [ ] Update dependency information for new libraries

### Task Completion Validation
- [ ] All task-related documentation is current
- [ ] No documentation gaps exist for implemented functionality
- [ ] Future developers can understand the implementation
- [ ] Documentation matches actual implementation

## Documentation Currency Verification

### "Is Documentation Up to Date?" Response Framework
**If following documentation-first correctly:**
```
Yes, documentation is current:
- PROJECTPLAN.md reflects latest task status
- Implementation READMEs document current functionality
- Architecture docs include recent design decisions
- API documentation matches current endpoints
- All task-related documentation updated during completion
```

**If documentation gaps exist:**
```
Documentation needs updates:
- [Specific gap 1]: [Required update]
- [Specific gap 2]: [Required update]
- [Specific gap 3]: [Required update]

This indicates documentation-first process breakdown.
```

### Documentation Update Request Analysis
**When asked "update documentation":**

**Correct Response (Minimal Updates):**
```
Documentation is largely current due to documentation-first approach.
Minor updates needed:
- [Small update 1]
- [Small update 2]
```

**Problem Response (Significant Work):**
```
Significant documentation updates needed:
- [Major update 1]
- [Major update 2]
- [Major update 3]

This indicates documentation-first process was not followed properly.
```

## Process Failure Detection

### Warning Signs
- **Major documentation gaps** after task completion
- **Significant work needed** when asked to "update documentation"
- **Inability to answer** "Is documentation up to date?" with confidence
- **Repeated requests** for documentation updates
- **Disconnect between implementation and documentation**

### Recovery Procedures
1. **Immediate Assessment**: Identify specific documentation gaps
2. **Gap Analysis**: Determine why documentation-first failed
3. **Rapid Update**: Bring documentation current with implementation
4. **Process Correction**: Implement preventive measures for future tasks
5. **Validation**: Verify documentation now matches implementation

### Prevention Measures
- **Real-time Documentation**: Update docs during, not after, implementation
- **Completion Checklist**: Use documentation checklist before marking tasks complete
- **Regular Verification**: Periodically check documentation currency
- **Process Reinforcement**: Remind agents of documentation-first principles
- **Failure Learning**: Document process failures to prevent repetition

## Operational Guidelines

### For All Modes
- **Default Behavior**: Update documentation immediately upon task completion
- **Verification Question**: Ask "Is documentation current?" before completing tasks
- **Gap Prevention**: Document decisions and changes as they happen
- **Quality Check**: Ensure documentation matches implementation reality

### For Orchestrator Mode
- **Task Creation**: Update PROJECTPLAN.md immediately when creating tasks
- **Task Completion**: Update PROJECTPLAN.md when agents report completion
- **Status Tracking**: Maintain current project documentation
- **Coordination**: Ensure all agents follow documentation-first principles

### For Implementation Agents
- **Development Documentation**: Update READMEs during implementation
- **Architecture Updates**: Document design decisions immediately
- **API Changes**: Update API documentation as endpoints are modified
- **Insight Capture**: Record framework-specific insights during development

### For Meta Mode
- **Process Documentation**: Update CLAUDE.md and meta/docs/ during improvements
- **Workflow Updates**: Document process changes immediately
- **Learning Integration**: Update documentation based on session insights
- **Validation**: Ensure documentation matches current process reality

## Documentation-First Success Examples

### Good Example: Task Completion
```
Task: Implement user authentication
During implementation:
- Updated API documentation with new endpoints
- Added authentication section to README
- Documented security considerations in ARCHITECTURE.md
- Updated PROJECTPLAN.md with task completion

Result: When asked "Is documentation up to date?" → "Yes"
```

### Bad Example: Task Completion
```
Task: Implement user authentication
During implementation:
- Focused only on code implementation
- No documentation updates

Result: When asked "Is documentation up to date?" → "Major gaps exist"
```

## Integration with Existing Workflow

### Command Integration
- **"Continue development"**: Include documentation updates in task execution
- **"Update documentation"**: Should be minimal if process followed correctly
- **"What's next"**: Should include documentation verification as task step

### Mode Integration
- **Mode Switching**: Verify documentation currency before switching modes
- **Task Delegation**: Include documentation requirements in task creation
- **Completion Reporting**: Include documentation status in progress reports

### Quality Integration
- **Code Quality**: Documentation quality is part of overall code quality
- **Testing**: Documentation accuracy is part of testing process
- **Validation**: Documentation currency is part of task validation

## Process Improvement Metrics

### Success Indicators
- **"Is documentation up to date?"** consistently answered "Yes"
- **"Update documentation"** requests are rare and minimal
- **Documentation gaps** are caught during task execution, not after
- **Process failure** incidents decrease over time
- **Documentation quality** improves through real-time updates

### Failure Indicators
- **Major documentation work** needed when asked to update
- **Repeated documentation requests** for the same areas
- **Disconnect** between implementation and documentation
- **Process confusion** about when to update documentation
- **Quality degradation** due to delayed documentation updates

## Next Steps for Implementation

1. **Integrate with CLAUDE.md**: Update main instruction file with documentation-first principles
2. **Update Mode Templates**: Include documentation requirements in mode activation templates
3. **Create Validation Tools**: Develop methods to verify documentation currency
4. **Establish Metrics**: Track documentation-first process success over time
5. **Continuous Improvement**: Regular review and refinement of documentation-first approach