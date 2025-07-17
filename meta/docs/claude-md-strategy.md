# CLAUDE.md Modularization Strategy

## Current State Analysis

### File Statistics
- **Current size**: 45,070 bytes (45KB)
- **Current lines**: 1,106 lines
- **Target size**: <500 lines for main file
- **Reduction goal**: ~75% size reduction

### Current Structure Analysis
The current CLAUDE.md contains these main sections:

1. **User Commands Reference** (Lines 5-102) - 98 lines
   - Command recognition rules (JSON patterns)
   - Development commands
   - Mode management commands
   - Information commands
   - Documentation commands
   - Error recovery commands
   - "List Tasks" command implementation

2. **Operating Modes System** (Lines 103-506) - 404 lines
   - Mode identification
   - 7 different modes (Default, Orchestrator, Bevy, EnTT, Flecs, Console, Meta)
   - Mode switching protocol
   - Context separation protocol
   - Mode-specific instruction templates

3. **Legacy and Project Overview** (Lines 507-563) - 57 lines
   - Legacy role-based instructions
   - Project overview
   - Implementation structure

4. **Task Management System** (Lines 564-785) - 222 lines
   - Task file structure and format
   - Agent workflow
   - Completion tracking system
   - Development continuation process

5. **Development Commands** (Lines 786-877) - 92 lines
   - Package management
   - Implementation-specific build commands
   - Testing and validation commands

6. **Quality Assurance & Operations** (Lines 878-1039) - 162 lines
   - Testing requirements
   - Validation criteria
   - Cross-implementation validation
   - Documentation standards
   - Performance considerations
   - Deployment and operations
   - Troubleshooting

7. **Success Criteria & Reminders** (Lines 1040-1106) - 67 lines
   - Success criteria
   - Important reminders
   - Next steps

## Modularization Strategy

### Core Principles
1. **Frequently Accessed Content**: Keep in main file
2. **Reference Material**: Move to modular imports  
3. **Logical Grouping**: Group related functionality
4. **Import Hierarchy**: Use clear file structure
5. **Minimal Disruption**: Preserve all existing functionality

### Module Structure Design

#### Main CLAUDE.md (Target: <500 lines)
Keep only the most frequently accessed content:
- Project overview and quick start
- User commands reference (condensed)
- Current mode identification
- Import statements for all modules
- Critical workflow instructions

#### Module Breakdown

**@meta/docs/commands.md** (~150 lines)
- Complete user commands reference
- Command recognition rules (JSON patterns)
- Command execution rules
- All command implementations

**@meta/docs/modes.md** (~400 lines)
- Complete operating modes system
- All 7 mode definitions
- Mode switching protocol
- Context separation protocol
- Mode-specific instruction templates

**@meta/docs/tasks.md** (~250 lines)
- Complete task management system
- Task file structure and format
- Agent workflow processes
- Completion tracking system
- Development continuation process

**@meta/docs/development.md** (~200 lines)
- Development commands for all implementations
- Package management guidelines
- Build and test procedures
- Quality assurance requirements
- Testing and validation criteria

**@meta/docs/architecture.md** (~150 lines)
- Implementation structure
- Project overview details
- Cross-implementation validation
- Performance considerations
- Documentation standards

**@meta/docs/operations.md** (~100 lines)
- Deployment and operations
- Troubleshooting guidelines
- Error handling and escalation
- Success criteria
- Important reminders

### Implementation Plan

#### Phase 1: Setup and Validation (Low Risk)
1. Create all module files in meta/docs/
2. Copy content to modules (no changes to main file yet)
3. Test import functionality with simple examples
4. Validate module structure and organization

#### Phase 2: Main File Refactoring (Medium Risk)
1. Create new streamlined main CLAUDE.md
2. Add import statements for all modules
3. Keep only essential content in main file
4. Backup original file as CLAUDE.md.backup

#### Phase 3: Integration Testing (High Risk)
1. Test all command recognition patterns
2. Validate mode switching functionality
3. Test workflow processes
4. Verify no behavioral changes

#### Phase 4: Optimization and Cleanup (Low Risk)
1. Remove redundant content
2. Optimize import statements
3. Update cross-references
4. Final validation

### Success Metrics
- **Size Reduction**: Main file under 500 lines (currently 1,106)
- **Functionality**: All commands and modes work identically
- **Maintainability**: Easier to navigate and modify
- **Context Efficiency**: Reduced memory usage while preserving functionality

### Testing Procedures

#### Command Recognition Testing
- Test each command pattern category
- Verify immediate recognition without tools
- Validate contextual responses
- Check fallback behavior

#### Mode Switching Testing
- Test all mode activation commands
- Verify mode-specific constraints
- Check context separation protocol
- Validate mode instruction templates

#### Workflow Testing
- Test "continue development" workflow
- Verify "what's next" functionality
- Check "update orchestrator" process
- Validate task management system

### Rollback Plan
1. **Backup Strategy**: Keep CLAUDE.md.backup with original content
2. **Quick Restore**: Simple file rename if critical issues arise
3. **Incremental Rollback**: Restore individual modules if specific issues found
4. **Validation Points**: Test after each phase with ability to revert

### Risk Mitigation
- **Incremental Implementation**: Phase-based approach with validation
- **Functionality Preservation**: No behavioral changes during modularization
- **Comprehensive Testing**: All features tested before final deployment
- **Clear Documentation**: All changes documented with rationale

## Implementation Details

### Import Statement Strategy
Use Claude Code's @path/to/file.md syntax:
```markdown
# Core functionality imports
@meta/docs/commands.md
@meta/docs/modes.md

# Process and workflow imports  
@meta/docs/tasks.md
@meta/docs/development.md

# Reference material imports
@meta/docs/architecture.md
@meta/docs/operations.md
```

### Content Organization Principles
- **Commands**: All user-facing command patterns and implementations
- **Modes**: Complete mode system with switching and templates
- **Tasks**: Task management, workflows, and completion tracking
- **Development**: Build commands, testing, and quality assurance
- **Architecture**: Project structure, validation, and standards
- **Operations**: Deployment, troubleshooting, and success criteria

### Cross-Reference Management
- Update all internal references to work with modular structure
- Maintain consistent section numbering across modules
- Ensure all links and references remain functional
- Document any reference changes for future maintenance

## Next Steps
1. Execute Phase 1: Create module structure and copy content
2. Execute Phase 2: Refactor main file with imports
3. Execute Phase 3: Comprehensive testing and validation
4. Execute Phase 4: Final optimization and cleanup

This strategy will achieve the goal of reducing CLAUDE.md from 1,106 lines to under 500 lines while maintaining all functionality and improving maintainability.