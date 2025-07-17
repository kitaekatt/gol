#!/bin/bash

# Coordination Check Script
# Validates coordination system integrity and detects failures
# Usage: bash meta/scripts/check-coordination.sh

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
COORDINATION_DIR="$PROJECT_ROOT/meta/coordination"
ACTIVE_AGENTS_FILE="$COORDINATION_DIR/active-agents.json"
FILE_LOCKS_FILE="$COORDINATION_DIR/file-locks.json"

# Colors for output
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Check if running in terminal that supports colors
if [[ ! -t 1 ]]; then
    RED=""
    YELLOW=""
    GREEN=""
    NC=""
fi

echo "=== COORDINATION SYSTEM INTEGRITY CHECK ==="
echo ""

# Check coordination directory
if [[ ! -d "$COORDINATION_DIR" ]]; then
    echo -e "${RED}❌ CRITICAL: Coordination directory missing${NC}"
    echo "   Expected: $COORDINATION_DIR"
    echo "   Action: Run 'mkdir -p $COORDINATION_DIR'"
    exit 1
fi

# Check active agents file
if [[ ! -f "$ACTIVE_AGENTS_FILE" ]]; then
    echo -e "${YELLOW}⚠️  WARNING: Active agents file missing${NC}"
    echo "   Expected: $ACTIVE_AGENTS_FILE"
    echo "   Action: Will be created on first agent registration"
else
    echo -e "${GREEN}✅ Active agents file exists${NC}"
    
    # Validate JSON structure
    if ! jq empty "$ACTIVE_AGENTS_FILE" 2>/dev/null; then
        echo -e "${RED}❌ CRITICAL: Active agents file has invalid JSON${NC}"
        echo "   File: $ACTIVE_AGENTS_FILE"
        echo "   Action: Fix JSON syntax or regenerate file"
        exit 1
    fi
    
    # Check schema version
    schema_version=$(jq -r '.schema_version // "missing"' "$ACTIVE_AGENTS_FILE")
    if [[ "$schema_version" != "1.0" ]]; then
        echo -e "${YELLOW}⚠️  WARNING: Schema version mismatch${NC}"
        echo "   Expected: 1.0"
        echo "   Found: $schema_version"
    fi
    
    # Check active agents
    active_count=$(jq -r '.agents | length' "$ACTIVE_AGENTS_FILE")
    if [[ "$active_count" -gt 0 ]]; then
        echo -e "${YELLOW}⚠️  $active_count active agents found${NC}"
        jq -r '.agents | to_entries[] | "   • \(.key): \(.value.mode) on \(.value.task_file)"' "$ACTIVE_AGENTS_FILE"
    else
        echo -e "${GREEN}✅ No active agents (coordination clear)${NC}"
    fi
fi

# Check file locks file
if [[ ! -f "$FILE_LOCKS_FILE" ]]; then
    echo -e "${YELLOW}⚠️  WARNING: File locks file missing${NC}"
    echo "   Expected: $FILE_LOCKS_FILE"
    echo "   Action: Will be created on first file lock"
else
    echo -e "${GREEN}✅ File locks file exists${NC}"
    
    # Validate JSON structure
    if ! jq empty "$FILE_LOCKS_FILE" 2>/dev/null; then
        echo -e "${RED}❌ CRITICAL: File locks file has invalid JSON${NC}"
        echo "   File: $FILE_LOCKS_FILE"
        echo "   Action: Fix JSON syntax or regenerate file"
        exit 1
    fi
    
    # Check active locks
    lock_count=$(jq -r '.locks | length' "$FILE_LOCKS_FILE")
    if [[ "$lock_count" -gt 0 ]]; then
        echo -e "${YELLOW}⚠️  $lock_count file locks found${NC}"
        jq -r '.locks | to_entries[] | "   • \(.key): locked by \(.value.agent_id)"' "$FILE_LOCKS_FILE"
    else
        echo -e "${GREEN}✅ No file locks (coordination clear)${NC}"
    fi
fi

echo ""

# Check for stale agents (older than 1 hour)
if [[ -f "$ACTIVE_AGENTS_FILE" ]]; then
    echo "=== STALE AGENT DETECTION ==="
    current_time=$(date +%s)
    stale_threshold=$((60 * 60)) # 1 hour in seconds
    
    stale_agents=$(jq -r --arg current_time "$current_time" --arg threshold "$stale_threshold" '
        .agents | to_entries[] | select(
            ($current_time | tonumber) - (.value.start_time | fromdateiso8601) > ($threshold | tonumber)
        ) | .key' "$ACTIVE_AGENTS_FILE")
    
    if [[ -n "$stale_agents" ]]; then
        echo -e "${YELLOW}⚠️  Stale agents detected (older than 1 hour):${NC}"
        echo "$stale_agents" | while read -r agent_id; do
            echo "   • $agent_id"
        done
        echo "   Action: Consider cleaning up stale agents"
    else
        echo -e "${GREEN}✅ No stale agents detected${NC}"
    fi
fi

echo ""

# Check for coordination script dependencies
echo "=== COORDINATION SCRIPT DEPENDENCIES ==="

# Check for jq dependency
if command -v jq >/dev/null 2>&1; then
    echo -e "${GREEN}✅ jq command available${NC}"
else
    echo -e "${RED}❌ CRITICAL: jq command missing${NC}"
    echo "   Action: Install jq (apt install jq)"
    exit 1
fi

# Check script permissions
scripts_dir="$PROJECT_ROOT/meta/scripts"
coordination_scripts=(
    "register-agent.sh"
    "unregister-agent.sh"
    "check-coordination.sh"
)

for script in "${coordination_scripts[@]}"; do
    script_path="$scripts_dir/$script"
    if [[ -f "$script_path" ]]; then
        if [[ -x "$script_path" ]]; then
            echo -e "${GREEN}✅ $script is executable${NC}"
        else
            echo -e "${YELLOW}⚠️  $script is not executable${NC}"
            echo "   Action: chmod +x $script_path"
        fi
    else
        echo -e "${RED}❌ $script is missing${NC}"
        echo "   Expected: $script_path"
    fi
done

echo ""

# Overall coordination status
echo "=== OVERALL COORDINATION STATUS ==="
if [[ -f "$ACTIVE_AGENTS_FILE" ]] && [[ -f "$FILE_LOCKS_FILE" ]]; then
    total_active=$(jq -r '.agents | length' "$ACTIVE_AGENTS_FILE")
    total_locks=$(jq -r '.locks | length' "$FILE_LOCKS_FILE")
    
    if [[ "$total_active" -eq 0 ]] && [[ "$total_locks" -eq 0 ]]; then
        echo -e "${GREEN}✅ COORDINATION CLEAR - No active agents or file locks${NC}"
    else
        echo -e "${YELLOW}⚠️  COORDINATION ACTIVE - $total_active agents, $total_locks locks${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  COORDINATION UNKNOWN - Missing coordination files${NC}"
fi