# Fix Task Analysis Script Completed Task Detection

## Task Overview
**Priority**: High
**Assigned Mode**: Meta
**Created By**: Orchestrator (critical bug fix)

## Objective
Fix the task analysis script to properly exclude completed tasks from available task listings, preventing agents from attempting to work on already completed tasks.

## Context
The `meta/scripts/analyze-agent-tasks.sh` script is incorrectly reporting tasks in the `completed/` directory as available for work. This creates confusion and inefficiency as agents may attempt to work on tasks that are already done.

## Problem Analysis
Current script behavior:
- Reports tasks in `completed/` folders as available
- Shows incorrect counts of available tasks
- Provides misleading task recommendations
- Wastes agent time investigating completed work

## Requirements
1. **Script Logic Fix**: Update the script to exclude tasks in `completed/` directories
2. **Accurate Reporting**: Ensure only genuinely available tasks are listed
3. **Completed Task Tracking**: Optionally show completed tasks separately for reference
4. **Status Validation**: Verify the fix works across all agent directories

## Agent Constraints
- Work exclusively within meta/ directory scope
- Focus on script functionality and accuracy
- Test with all agent directories (bevy, entt, flecs, console, meta)
- End response with [Mode:Meta] declaration

## Expected Deliverables
1. Fixed `meta/scripts/analyze-agent-tasks.sh` script
2. Accurate task availability reporting
3. Proper exclusion of completed tasks
4. Testing validation across all agent directories

## Acceptance Criteria
- Script excludes tasks in `*/completed/` directories from available listings
- Task counts are accurate and reflect only available work
- Completed tasks are properly identified and excluded
- Script works correctly for all agent types (bevy, entt, flecs, console, meta)
- No false positives in task availability reporting

## Technical Requirements
- Modify the script's task discovery logic
- Add proper path filtering for completed directories
- Maintain existing functionality for genuinely available tasks
- Ensure backward compatibility with existing usage patterns

## Testing Protocol
1. Run script for each agent type (bevy, entt, flecs, console, meta)
2. Verify completed tasks are excluded from available listings
3. Confirm task counts are accurate
4. Test with mixed scenarios (some completed, some available)

## Notes
This is a critical bug that affects all agent task selection. Fix immediately to prevent continued confusion and inefficiency.