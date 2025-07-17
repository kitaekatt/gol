#!/bin/bash

# Agent Unregistration Script
# Removes an agent from the coordination system
# Usage: bash meta/scripts/unregister-agent.sh [agent-mode] [task-file]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
COORDINATION_DIR="$PROJECT_ROOT/meta/coordination"
ACTIVE_AGENTS_FILE="$COORDINATION_DIR/active-agents.json"

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

# Check if active agents file exists
if [[ ! -f "$ACTIVE_AGENTS_FILE" ]]; then
    echo "Warning: No active agents file found"
    exit 0
fi

# Find and remove matching agent
TIMESTAMP=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

# Use jq to remove agents matching the mode and task file
jq --arg agent_mode "$AGENT_MODE" \
   --arg task_file "$TASK_FILE" \
   --arg timestamp "$TIMESTAMP" \
   '.agents = (.agents | to_entries | map(select(.value.mode != $agent_mode or .value.task_file != $task_file)) | from_entries) | .last_updated = $timestamp' \
   "$ACTIVE_AGENTS_FILE" > "$ACTIVE_AGENTS_FILE.tmp" && mv "$ACTIVE_AGENTS_FILE.tmp" "$ACTIVE_AGENTS_FILE"

echo "Agent unregistered: $AGENT_MODE working on $TASK_FILE"
echo "Time: $TIMESTAMP"