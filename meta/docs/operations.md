# Operations and Workflow Management

## Context Separation Protocol
Use `/clear` commands at strategic points:
- Between major mode changes
- Before entering specialized modes
- After complex mode sessions
- When starting new major tasks
- After error recovery

## Context Separation Benefits
- Reduced context confusion during sessions
- More focused responses for specific tasks
- Better task isolation between different work types
- Improved consistency in mode-specific behavior

## Claude Code Best Practices Integration

### Memory Management Insights
- **Hierarchical Memory System**: CLAUDE.md serves as L1 cache (fast access), with imported modules as L2 cache (structured context)
- **Import-Based Context Loading**: Use @file imports to load specific documentation modules only when needed
- **Context Separation**: Strategic `/clear` usage prevents context accumulation and maintains focus
- **Validation Checkpoints**: Regular documentation-reality validation prevents drift
- **Maintenance Protocols**: Scheduled documentation updates keep knowledge current

### Meta Mode Rationale
Created to address identified limitations in Claude Code's memory persistence:
1. **Context Separation Strategy**: Systematic `/clear` usage patterns for optimal focus
2. **Memory Maintenance Protocol**: Prevent documentation drift through regular maintenance
3. **CLAUDE.md Modularization**: Break monolithic instruction file into import-based modules
4. **Memory Validation Checkpoints**: Ensure documentation matches implementation reality

These meta-improvements directly address Claude Code's context management challenges and establish sustainable workflows for long-term project success.

## Tool Best Practices
- Maintain a list of tool best practices
- Whenever encountering a tool error, reflect on opportunities to update this list to avoid similar errors in the future
- Avoid simply appending endlessly; instead, consider the best way to integrate new knowledge

### Common Tool Error Patterns and Solutions
- **Script Path Resolution Issues**: Use absolute paths (`/mnt/c/dev/gol/meta/scripts/script.sh`) when relative paths fail
- **Coordination Cleanup Failures**: Always run cleanup protocols during task completion, especially stale agent cleanup
- **Documentation-Reality Gaps**: When encountering tool errors, immediately update documentation to reflect actual implementation
- **Missing Script Detection**: Verify script existence before referencing in documentation or workflows