#!/bin/bash

# Coordination Recovery Script
# Recovers from coordination protocol failures and conflicts
# Usage: bash meta/scripts/coordination-recovery.sh [action] [agent-mode] [task-file]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
COORDINATION_DIR="$PROJECT_ROOT/meta/coordination"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Usage function
usage() {
    echo "Usage: $0 [action] [agent-mode] [task-file]"
    echo ""
    echo "Actions:"
    echo "  clear_all_claims     - Remove all claim files (emergency reset)"
    echo "  clear_stale_claims   - Remove claims older than 4 hours"
    echo "  force_unclaim        - Force remove specific task claim"
    echo "  reset_coordination   - Reset entire coordination system"
    echo "  show_conflicts       - Show current coordination conflicts"
    echo "  cleanup_locks        - Clean up expired file locks"
    echo ""
    echo "Agent modes: bevy, entt, flecs, console, meta"
    echo ""
    echo "Examples:"
    echo "  $0 clear_stale_claims"
    echo "  $0 force_unclaim bevy setup-project.md"
    echo "  $0 reset_coordination"
    echo "  $0 show_conflicts"
    exit 1
}

# Validate input
if [[ $# -lt 1 ]]; then
    usage
fi

ACTION="$1"
AGENT_MODE="${2:-}"
TASK_FILE="${3:-}"

# Function to find all claim files
find_claim_files() {
    find "$PROJECT_ROOT" -name "*.claimed" -type f 2>/dev/null
}

# Function to check if claim is stale (older than 4 hours)
is_stale_claim() {
    local claim_file="$1"
    local claim_time=$(stat -c %Y "$claim_file" 2>/dev/null || echo "0")
    local current_time=$(date +%s)
    local age_seconds=$((current_time - claim_time))
    local four_hours_seconds=$((4 * 60 * 60))
    
    if [[ $age_seconds -gt $four_hours_seconds ]]; then
        return 0  # Stale
    else
        return 1  # Fresh
    fi
}

# Function to show claim details
show_claim_details() {
    local claim_file="$1"
    echo "Claim file: $claim_file"
    echo "Contents:"
    cat "$claim_file" 2>/dev/null || echo "  (unable to read)"
    echo "Age: $(date -r "$claim_file" 2>/dev/null || echo "unknown")"
    echo ""
}

case "$ACTION" in
    "clear_all_claims")
        echo "=== CLEARING ALL CLAIM FILES ==="
        echo -e "${RED}⚠️  WARNING: This will remove ALL task claims${NC}"
        echo "This may interrupt agents currently working on tasks."
        echo ""
        
        claim_files=($(find_claim_files))
        if [[ ${#claim_files[@]} -eq 0 ]]; then
            echo "No claim files found."
            exit 0
        fi
        
        echo "Found ${#claim_files[@]} claim files:"
        for claim_file in "${claim_files[@]}"; do
            echo "  $claim_file"
        done
        echo ""
        
        read -p "Are you sure you want to remove all claims? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            for claim_file in "${claim_files[@]}"; do
                rm -f "$claim_file"
                echo "Removed: $claim_file"
            done
            echo -e "${GREEN}✅ All claim files removed${NC}"
        else
            echo "Operation cancelled."
        fi
        ;;
        
    "clear_stale_claims")
        echo "=== CLEARING STALE CLAIM FILES ==="
        echo "Removing claims older than 4 hours..."
        echo ""
        
        claim_files=($(find_claim_files))
        stale_claims=()
        
        for claim_file in "${claim_files[@]}"; do
            if is_stale_claim "$claim_file"; then
                stale_claims+=("$claim_file")
            fi
        done
        
        if [[ ${#stale_claims[@]} -eq 0 ]]; then
            echo "No stale claims found."
            exit 0
        fi
        
        echo "Found ${#stale_claims[@]} stale claim files:"
        for claim_file in "${stale_claims[@]}"; do
            show_claim_details "$claim_file"
        done
        
        for claim_file in "${stale_claims[@]}"; do
            rm -f "$claim_file"
            echo "Removed stale claim: $claim_file"
        done
        
        echo -e "${GREEN}✅ Stale claims removed${NC}"
        ;;
        
    "force_unclaim")
        if [[ -z "$AGENT_MODE" ]] || [[ -z "$TASK_FILE" ]]; then
            echo -e "${RED}ERROR: force_unclaim requires agent-mode and task-file${NC}"
            usage
        fi
        
        # Validate agent mode
        case "$AGENT_MODE" in
            "bevy"|"entt"|"flecs"|"console"|"meta")
                ;;
            *)
                echo -e "${RED}ERROR: Invalid agent mode '$AGENT_MODE'${NC}"
                usage
                ;;
        esac
        
        # Determine task directory
        case "$AGENT_MODE" in
            "bevy")
                TASK_DIR="$PROJECT_ROOT/gol-bevy/tasks"
                ;;
            "entt")
                TASK_DIR="$PROJECT_ROOT/gol-entt/tasks"
                ;;
            "flecs")
                TASK_DIR="$PROJECT_ROOT/gol-flecs/tasks"
                ;;
            "console")
                TASK_DIR="$PROJECT_ROOT/gol-console-client/tasks"
                ;;
            "meta")
                TASK_DIR="$PROJECT_ROOT/meta/tasks"
                ;;
        esac
        
        CLAIM_FILE="$TASK_DIR/$TASK_FILE.claimed"
        
        echo "=== FORCE UNCLAIMING TASK ==="
        echo "Agent Mode: $AGENT_MODE"
        echo "Task File: $TASK_FILE"
        echo "Claim File: $CLAIM_FILE"
        echo ""
        
        if [[ -f "$CLAIM_FILE" ]]; then
            show_claim_details "$CLAIM_FILE"
            
            echo -e "${YELLOW}⚠️  WARNING: Forcing unclaim may interrupt active work${NC}"
            read -p "Are you sure you want to force unclaim this task? (y/N): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                rm -f "$CLAIM_FILE"
                echo -e "${GREEN}✅ Task unclaimed: $TASK_FILE${NC}"
                
                # Also try to unregister agent
                if bash "$SCRIPT_DIR/unregister-agent.sh" "$AGENT_MODE" "$TASK_FILE" 2>/dev/null; then
                    echo "Agent unregistered from coordination system."
                fi
            else
                echo "Operation cancelled."
            fi
        else
            echo "No claim file found for task: $TASK_FILE"
        fi
        ;;
        
    "reset_coordination")
        echo "=== RESETTING COORDINATION SYSTEM ==="
        echo -e "${RED}⚠️  WARNING: This will reset the entire coordination system${NC}"
        echo "This will:"
        echo "1. Remove all claim files"
        echo "2. Clear active agents registry"
        echo "3. Clear file locks"
        echo "4. Reset coordination system state"
        echo ""
        
        read -p "Are you sure you want to reset coordination? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            # Remove all claim files
            claim_files=($(find_claim_files))
            for claim_file in "${claim_files[@]}"; do
                rm -f "$claim_file"
                echo "Removed claim: $claim_file"
            done
            
            # Reset coordination files
            rm -f "$COORDINATION_DIR/active-agents.json"
            rm -f "$COORDINATION_DIR/file-locks.json"
            echo "Reset coordination system files"
            
            # Reinitialize coordination system
            python3 "$PROJECT_ROOT/meta/coordination/coordination-system.py" 2>/dev/null || echo "Coordination system reset (Python system not available)"
            
            echo -e "${GREEN}✅ Coordination system reset complete${NC}"
        else
            echo "Operation cancelled."
        fi
        ;;
        
    "show_conflicts")
        echo "=== COORDINATION CONFLICTS REPORT ==="
        echo ""
        
        # Show active claims
        claim_files=($(find_claim_files))
        if [[ ${#claim_files[@]} -gt 0 ]]; then
            echo "ACTIVE CLAIMS:"
            for claim_file in "${claim_files[@]}"; do
                show_claim_details "$claim_file"
            done
        else
            echo "No active claims found."
        fi
        
        # Show coordination system status
        echo "COORDINATION SYSTEM STATUS:"
        if [[ -f "$COORDINATION_DIR/active-agents.json" ]]; then
            echo "Active agents file exists"
            jq -r '.agents | keys[]' "$COORDINATION_DIR/active-agents.json" 2>/dev/null | while read -r agent_id; do
                echo "  Agent: $agent_id"
            done
        else
            echo "No active agents file found"
        fi
        
        if [[ -f "$COORDINATION_DIR/file-locks.json" ]]; then
            echo "File locks file exists"
            jq -r '.locks | keys[]' "$COORDINATION_DIR/file-locks.json" 2>/dev/null | while read -r file_path; do
                echo "  Locked file: $file_path"
            done
        else
            echo "No file locks file found"
        fi
        ;;
        
    "cleanup_locks")
        echo "=== CLEANING UP EXPIRED FILE LOCKS ==="
        echo ""
        
        if [[ -f "$PROJECT_ROOT/meta/coordination/coordination-system.py" ]]; then
            python3 "$PROJECT_ROOT/meta/coordination/coordination-system.py" cleanup 2>/dev/null || echo "Cleanup command not available"
        else
            echo "Coordination system not available for cleanup"
        fi
        
        echo -e "${GREEN}✅ Lock cleanup attempted${NC}"
        ;;
        
    *)
        echo -e "${RED}ERROR: Unknown action '$ACTION'${NC}"
        usage
        ;;
esac