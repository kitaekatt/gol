#!/bin/bash

# Enhanced Agent Task Analysis with Coordination Enforcement
# Integrates mandatory coordination checks with task analysis
# Usage: bash meta/scripts/enhanced-analyze-agent-tasks.sh [agent-mode]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Usage function
usage() {
    echo "Usage: $0 [agent-mode]"
    echo "Agent modes: bevy, entt, flecs, console, meta"
    echo ""
    echo "Enhanced task analysis with mandatory coordination enforcement."
    exit 1
}

# Validate input
if [[ $# -ne 1 ]]; then
    usage
fi

AGENT_MODE="$1"

# Validate agent mode
case "$AGENT_MODE" in
    "bevy"|"entt"|"flecs"|"console"|"meta")
        ;;
    *)
        echo -e "${RED}ERROR: Invalid agent mode '$AGENT_MODE'${NC}"
        usage
        ;;
esac

echo "=== ENHANCED AGENT TASK ANALYSIS WITH COORDINATION ENFORCEMENT ==="
echo "Agent Mode: $AGENT_MODE"
echo ""

# STEP 1: Check current coordination status
echo "Checking coordination status..."
if ! bash "$SCRIPT_DIR/mandatory-coordination-check.sh" "$AGENT_MODE" "validate" 2>/dev/null; then
    echo -e "${YELLOW}⚠️  Agent is not currently coordinated${NC}"
    echo "Coordination enforcement will be required before task execution."
    echo ""
fi

# STEP 2: Run standard task analysis
echo "Running task analysis..."
echo ""
bash "$SCRIPT_DIR/analyze-agent-tasks.sh" "$AGENT_MODE"

# STEP 3: Add coordination enforcement instructions
echo ""
echo "=== COORDINATION ENFORCEMENT INSTRUCTIONS ==="
echo ""
echo "BEFORE BEGINNING ANY TASK WORK:"
echo "1. Use coordination enforcement: bash meta/scripts/enforce-coordination.sh $AGENT_MODE [task-file]"
echo "2. Verify enforcement success before proceeding"
echo "3. Begin task work only after successful coordination"
echo ""
echo "WHEN TASK COMPLETE:"
echo "1. Remove claim file: rm [task-directory]/[task-file].claimed"
echo "2. Unregister agent: bash meta/scripts/unregister-agent.sh $AGENT_MODE [task-file]"
echo ""
echo "CONTINUE DEVELOPMENT COMMAND:"
echo "• Use: bash meta/scripts/mandatory-coordination-check.sh $AGENT_MODE continue_development"
echo "• This will validate coordination and provide next steps"
echo ""
echo -e "${RED}⚠️  IMPORTANT: Coordination enforcement is now MANDATORY${NC}"
echo "Agents that bypass coordination may conflict with other agents."
echo "Always use coordination scripts before beginning task work."