#!/bin/bash

# Coordination Cleanup Script
# Removes stale agents and locks from coordination system
# Usage: bash meta/scripts/cleanup-coordination.sh [--force]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
COORDINATION_DIR="$PROJECT_ROOT/meta/coordination"
ACTIVE_AGENTS_FILE="$COORDINATION_DIR/active-agents.json"
FILE_LOCKS_FILE="$COORDINATION_DIR/file-locks.json"
STALE_THRESHOLD=$((60 * 60)) # 1 hour in seconds

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

# Parse command line arguments
FORCE_CLEANUP=false
if [[ $# -eq 1 ]] && [[ "$1" == "--force" ]]; then
    FORCE_CLEANUP=true
fi

echo "=== COORDINATION CLEANUP ==="
echo ""

# Check if coordination files exist
if [[ ! -f "$ACTIVE_AGENTS_FILE" ]] && [[ ! -f "$FILE_LOCKS_FILE" ]]; then
    echo -e "${GREEN}✅ No coordination files found - system is clean${NC}"
    exit 0
fi

# Cleanup stale agents
if [[ -f "$ACTIVE_AGENTS_FILE" ]]; then
    echo "=== CLEANING STALE AGENTS ==="
    
    current_time=$(date +%s)
    
    # Find stale agents
    stale_agents=$(jq -r --arg current_time "$current_time" --arg threshold "$STALE_THRESHOLD" '
        .agents | to_entries[] | select(
            ($current_time | tonumber) - (.value.start_time | fromdateiso8601) > ($threshold | tonumber)
        ) | .key' "$ACTIVE_AGENTS_FILE" 2>/dev/null || echo "")
    
    if [[ -n "$stale_agents" ]]; then
        echo -e "${YELLOW}⚠️  Found stale agents (older than 1 hour):${NC}"
        echo "$stale_agents" | while read -r agent_id; do
            if [[ -n "$agent_id" ]]; then
                echo "   • $agent_id"
            fi
        done
        
        if [[ "$FORCE_CLEANUP" == "true" ]]; then
            echo -e "${YELLOW}🧹 Removing stale agents...${NC}"
            
            # Remove stale agents
            jq --arg current_time "$current_time" --arg threshold "$STALE_THRESHOLD" '
                .agents = (.agents | to_entries | map(select(
                    ($current_time | tonumber) - (.value.start_time | fromdateiso8601) <= ($threshold | tonumber)
                )) | from_entries) | .last_updated = ($current_time | tonumber | todateiso8601)' \
                "$ACTIVE_AGENTS_FILE" > "$ACTIVE_AGENTS_FILE.tmp" && mv "$ACTIVE_AGENTS_FILE.tmp" "$ACTIVE_AGENTS_FILE"
            
            echo -e "${GREEN}✅ Stale agents removed${NC}"
        else
            echo -e "${YELLOW}⚠️  Run with --force to remove stale agents${NC}"
        fi
    else
        echo -e "${GREEN}✅ No stale agents found${NC}"
    fi
    
    # Show remaining active agents
    active_count=$(jq -r '.agents | length' "$ACTIVE_AGENTS_FILE" 2>/dev/null || echo "0")
    if [[ "$active_count" -gt 0 ]]; then
        echo -e "${YELLOW}⚠️  $active_count active agents remaining:${NC}"
        jq -r '.agents | to_entries[] | "   • \(.key): \(.value.mode) on \(.value.task_file)"' "$ACTIVE_AGENTS_FILE"
    else
        echo -e "${GREEN}✅ No active agents remaining${NC}"
    fi
fi

echo ""

# Cleanup stale file locks
if [[ -f "$FILE_LOCKS_FILE" ]]; then
    echo "=== CLEANING STALE FILE LOCKS ==="
    
    current_time=$(date +%s)
    
    # Find stale locks
    stale_locks=$(jq -r --arg current_time "$current_time" --arg threshold "$STALE_THRESHOLD" '
        .locks | to_entries[] | select(
            ($current_time | tonumber) - (.value.timestamp | fromdateiso8601) > ($threshold | tonumber)
        ) | .key' "$FILE_LOCKS_FILE" 2>/dev/null || echo "")
    
    if [[ -n "$stale_locks" ]]; then
        echo -e "${YELLOW}⚠️  Found stale file locks (older than 1 hour):${NC}"
        echo "$stale_locks" | while read -r lock_file; do
            if [[ -n "$lock_file" ]]; then
                echo "   • $lock_file"
            fi
        done
        
        if [[ "$FORCE_CLEANUP" == "true" ]]; then
            echo -e "${YELLOW}🧹 Removing stale file locks...${NC}"
            
            # Remove stale locks
            jq --arg current_time "$current_time" --arg threshold "$STALE_THRESHOLD" '
                .locks = (.locks | to_entries | map(select(
                    ($current_time | tonumber) - (.value.timestamp | fromdateiso8601) <= ($threshold | tonumber)
                )) | from_entries) | .last_updated = ($current_time | tonumber | todateiso8601)' \
                "$FILE_LOCKS_FILE" > "$FILE_LOCKS_FILE.tmp" && mv "$FILE_LOCKS_FILE.tmp" "$FILE_LOCKS_FILE"
            
            echo -e "${GREEN}✅ Stale file locks removed${NC}"
        else
            echo -e "${YELLOW}⚠️  Run with --force to remove stale file locks${NC}"
        fi
    else
        echo -e "${GREEN}✅ No stale file locks found${NC}"
    fi
    
    # Show remaining file locks
    lock_count=$(jq -r '.locks | length' "$FILE_LOCKS_FILE" 2>/dev/null || echo "0")
    if [[ "$lock_count" -gt 0 ]]; then
        echo -e "${YELLOW}⚠️  $lock_count file locks remaining:${NC}"
        jq -r '.locks | to_entries[] | "   • \(.key): locked by \(.value.agent_id)"' "$FILE_LOCKS_FILE"
    else
        echo -e "${GREEN}✅ No file locks remaining${NC}"
    fi
fi

echo ""

# Final coordination status
echo "=== FINAL COORDINATION STATUS ==="
total_agents=$(jq -r '.agents | length' "$ACTIVE_AGENTS_FILE" 2>/dev/null || echo "0")
total_locks=$(jq -r '.locks | length' "$FILE_LOCKS_FILE" 2>/dev/null || echo "0")

if [[ "$total_agents" -eq 0 ]] && [[ "$total_locks" -eq 0 ]]; then
    echo -e "${GREEN}✅ COORDINATION CLEAR - System is clean${NC}"
else
    echo -e "${YELLOW}⚠️  COORDINATION ACTIVE - $total_agents agents, $total_locks locks${NC}"
fi