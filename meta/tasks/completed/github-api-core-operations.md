# GitHub API Core Operations Implementation

## Task Overview
**Priority**: High
**Assigned Mode**: Meta
**Phase**: 2 (Core Functionality)

## Objective
Implement and validate core GitHub API operations for task management, focusing on reading tasks, writing tasks, changing status, and querying task information. This establishes the fundamental operations that AI agents will use to understand and manage GitHub Issues.

## Context
We have completed Phase 1 (GitHub API foundation and project structure). Now we need to build the core operations that will allow AI agents to interact with GitHub Issues in the same way they currently interact with flat files.

## Requirements

### Core CRUD Operations
1. **Read Tasks**: Query GitHub Issues with filtering by labels, assignee, status
2. **Write Tasks**: Create new GitHub Issues with proper templates and labels
3. **Change Status**: Update issue state (open/closed) and labels (todo → in-progress → done)
4. **Query Information**: Get task details, assignee, status, comments, history

### AI Information Gathering
Create clear, structured responses that AI agents can parse:
- Task lists with priority and status information
- Individual task details with full context
- Assignment and coordination status
- Progress tracking and history

### Validation Requirements
- Test each operation with real GitHub Issues
- Verify AI agents can understand the information structure
- Confirm status transitions work correctly
- Validate filtering and querying capabilities

## Implementation Tasks

### 1. Core API Operations
Create `meta/scripts/github-core-ops.sh` with functions:
```bash
# Query operations
github_list_tasks()          # List issues with filters
github_get_task()           # Get single issue details
github_query_status()       # Check task status and assignee

# Write operations  
github_create_task()        # Create new issue with template
github_update_task()        # Update issue content
github_assign_task()        # Assign/unassign task

# Status operations
github_change_status()      # Update issue state and labels
github_add_comment()        # Add progress updates
github_close_task()         # Mark task complete
```

### 2. AI-Friendly Information Structure
Design output formats that AI agents can easily parse:
- Structured JSON responses for programmatic use
- Human-readable summaries for user interaction
- Clear status indicators and priority information
- Comprehensive task context including requirements and acceptance criteria

### 3. Status Management System
Implement GitHub Issues equivalent of our current status system:
- **Label Mapping**: Map our priority/status labels to GitHub Issues
- **State Transitions**: Handle todo → in-progress → done workflows
- **Assignment Tracking**: Use GitHub's assignee field for coordination
- **Progress Updates**: Use issue comments for status updates

### 4. Validation Testing
Test each operation thoroughly:
- Create test issues and verify AI can read them correctly
- Test status transitions and verify AI understands changes
- Validate querying and filtering works as expected
- Confirm AI agents can extract all necessary information

## Agent Constraints
- Work exclusively within meta/ directory scope
- Test operations using the existing GitHub Project setup
- Focus on information clarity and AI parseability
- Ensure operations match existing flat file capabilities

## Expected Deliverables
1. **Core Operations Script**: Complete `github-core-ops.sh` with all CRUD operations
2. **Information Structure Documentation**: Clear specification of how AI agents will interpret GitHub Issues data
3. **Status Management System**: Working implementation of status transitions and assignment tracking
4. **Validation Results**: Demonstrated functionality with test GitHub Issues
5. **API Response Formats**: Documented structure of information that AI agents will receive

## Acceptance Criteria
- [ ] AI agents can query available tasks and understand priority/status
- [ ] AI agents can read individual task details with full context
- [ ] Status transitions (todo → in-progress → done) work correctly
- [ ] Assignment and coordination information is clearly accessible
- [ ] All operations handle errors gracefully and provide clear feedback
- [ ] Information structure is consistent and easily parseable
- [ ] Test issues demonstrate full functionality

## Notes
This task focuses on the fundamental operations that will enable AI agents to understand and manage GitHub Issues. The key is ensuring information clarity and parseability - AI agents need to extract the same information they currently get from flat files, but from GitHub Issues instead.

Success means an AI agent can look at GitHub Issues and understand: what tasks are available, who is working on what, what the status is, and what needs to be done next.