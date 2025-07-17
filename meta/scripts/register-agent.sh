#!/bin/bash

# Agent Registration Script
# Registers an agent as active in the coordination system
# Usage: bash meta/scripts/register-agent.sh [agent-mode] [task-file]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
COORDINATION_DIR="$PROJECT_ROOT/meta/coordination"
ACTIVE_AGENTS_FILE="$COORDINATION_DIR/active-agents.json"
FILE_LOCKS_FILE="$COORDINATION_DIR/file-locks.json"

# Usage function
usage() {
    echo "Usage: $0 [agent-mode] [task-file]"
    echo "Agent modes: bevy, entt, flecs, console, meta"
    echo "Example: $0 bevy setup-project.md"
    exit 1
}

# Validate input
if [[ $# -ne 2 ]]; then
    usage
fi

AGENT_MODE="$1"
TASK_FILE="$2"

# Validate agent mode
case "$AGENT_MODE" in
    "bevy"|"entt"|"flecs"|"console"|"meta")
        ;;
    *)
        echo "Error: Invalid agent mode '$AGENT_MODE'"
        usage
        ;;
esac

# Ensure coordination directory exists
mkdir -p "$COORDINATION_DIR"

# Initialize active agents file if it doesn't exist
if [[ ! -f "$ACTIVE_AGENTS_FILE" ]]; then
    cat > "$ACTIVE_AGENTS_FILE" << EOF
{
  "agents": {},
  "last_updated": null,
  "schema_version": "1.0",
  "coordination_enabled": true,
  "notes": "Tracks currently active agents and their work status"
}
EOF
fi

# Register agent
TIMESTAMP=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
AGENT_ID="${AGENT_MODE}_$(date +%s)"

# Use jq to update the active agents file
jq --arg agent_id "$AGENT_ID" \
   --arg agent_mode "$AGENT_MODE" \
   --arg task_file "$TASK_FILE" \
   --arg timestamp "$TIMESTAMP" \
   '.agents[$agent_id] = {
     "mode": $agent_mode,
     "task_file": $task_file,
     "start_time": $timestamp,
     "status": "active"
   } | .last_updated = $timestamp' \
   "$ACTIVE_AGENTS_FILE" > "$ACTIVE_AGENTS_FILE.tmp" && mv "$ACTIVE_AGENTS_FILE.tmp" "$ACTIVE_AGENTS_FILE"

echo "Agent registered: $AGENT_ID"
echo "Mode: $AGENT_MODE"
echo "Task: $TASK_FILE"
echo "Time: $TIMESTAMP"