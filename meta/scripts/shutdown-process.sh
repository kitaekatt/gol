#!/bin/bash

# Shutdown Process Script for Game of Life Multi-Implementation Project
# Usage: bash meta/scripts/shutdown-process.sh [command] [mode]
# Commands: status, prepare, begin, doc_check
# Mode: bevy, entt, flecs, console, meta, orchestrator, default

COMMAND=${1:-"status"}
MODE=${2:-"default"}

# Project root directory
PROJECT_ROOT="/mnt/c/dev/gol"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "=== SHUTDOWN PROCESS: ${MODE^^} MODE ==="
echo "Command: $COMMAND"
echo ""

# Function to check task status
check_task_status() {
    local mode_dir="$1"
    local task_count=0
    local completed_count=0
    
    if [ -d "$mode_dir/tasks" ]; then
        task_count=$(find "$mode_dir/tasks" -name "*.md" -type f | wc -l)
        if [ -d "$mode_dir/tasks/completed" ]; then
            completed_count=$(find "$mode_dir/tasks/completed" -name "*.md" -type f | wc -l)
        fi
    fi
    
    echo "  Active tasks: $task_count"
    echo "  Completed tasks: $completed_count"
    
    if [ $task_count -eq 0 ]; then
        echo -e "  ${GREEN}✓ All tasks completed${NC}"
        return 0
    else
        echo -e "  ${YELLOW}⚠ Tasks still in progress${NC}"
        return 1
    fi
}

# Function to check documentation currency
check_documentation_currency() {
    local mode_dir="$1"
    local doc_status=0
    
    echo "Documentation Currency Check:"
    
    # Check for README in mode directory
    if [ -f "$mode_dir/README.md" ]; then
        echo -e "  ${GREEN}✓ README.md exists${NC}"
    else
        echo -e "  ${RED}✗ README.md missing${NC}"
        doc_status=1
    fi
    
    # Check if project-level docs exist
    local project_docs=("README.md" "ARCHITECTURE.md" "REQUIREMENTS.md" "PROJECTPLAN.md")
    for doc in "${project_docs[@]}"; do
        if [ -f "$PROJECT_ROOT/$doc" ]; then
            echo -e "  ${GREEN}✓ $doc exists${NC}"
        else
            echo -e "  ${RED}✗ $doc missing${NC}"
            doc_status=1
        fi
    done
    
    return $doc_status
}

# Function to check code quality
check_code_quality() {
    local mode_dir="$1"
    local quality_status=0
    
    echo "Code Quality Check:"
    
    # Check for common build files
    if [ -f "$mode_dir/Cargo.toml" ]; then
        echo -e "  ${BLUE}ℹ Rust project detected${NC}"
        if command -v cargo &> /dev/null; then
            echo "  Running cargo check..."
            if (cd "$mode_dir" && cargo check --quiet); then
                echo -e "  ${GREEN}✓ Cargo check passed${NC}"
            else
                echo -e "  ${RED}✗ Cargo check failed${NC}"
                quality_status=1
            fi
        fi
    elif [ -f "$mode_dir/CMakeLists.txt" ]; then
        echo -e "  ${BLUE}ℹ CMake project detected${NC}"
        echo -e "  ${YELLOW}⚠ Manual build verification required${NC}"
    elif [ -f "$mode_dir/package.json" ]; then
        echo -e "  ${BLUE}ℹ Node.js project detected${NC}"
        if command -v npm &> /dev/null; then
            echo "  Running npm test..."
            if (cd "$mode_dir" && npm test --silent); then
                echo -e "  ${GREEN}✓ npm test passed${NC}"
            else
                echo -e "  ${RED}✗ npm test failed${NC}"
                quality_status=1
            fi
        fi
    else
        echo -e "  ${YELLOW}⚠ No build system detected${NC}"
    fi
    
    return $quality_status
}

# Function to show shutdown status
show_shutdown_status() {
    local mode_dir="$1"
    local overall_status=0
    
    echo "=== SHUTDOWN STATUS ASSESSMENT ==="
    echo ""
    
    echo "Phase 1: Task Completion Verification"
    if ! check_task_status "$mode_dir"; then
        overall_status=1
    fi
    echo ""
    
    echo "Phase 2: Documentation Validation"
    if ! check_documentation_currency "$mode_dir"; then
        overall_status=1
    fi
    echo ""
    
    echo "Phase 3: Code Quality Verification"
    if ! check_code_quality "$mode_dir"; then
        overall_status=1
    fi
    echo ""
    
    echo "=== OVERALL SHUTDOWN READINESS ==="
    if [ $overall_status -eq 0 ]; then
        echo -e "${GREEN}✓ READY FOR SHUTDOWN${NC}"
        echo "All quality gates satisfied"
    else
        echo -e "${RED}✗ NOT READY FOR SHUTDOWN${NC}"
        echo "Please address the issues above before shutdown"
    fi
    
    return $overall_status
}

# Function to prepare for shutdown
prepare_shutdown() {
    local mode_dir="$1"
    
    echo "=== SHUTDOWN PREPARATION ==="
    echo ""
    
    echo "Preparing shutdown checklist for $MODE mode..."
    
    # Create shutdown checklist
    echo "Shutdown Checklist:"
    echo "- [ ] All active tasks at logical stopping points"
    echo "- [ ] Documentation updated during task completion"
    echo "- [ ] Code quality requirements met"
    echo "- [ ] Next session priorities identified"
    echo "- [ ] Blockers documented"
    echo ""
    
    # Show next steps
    echo "Next Steps:"
    if [ -d "$mode_dir/tasks" ]; then
        echo "Immediate priorities for next session:"
        find "$mode_dir/tasks" -name "*.md" -type f | head -3 | while read -r task; do
            echo "  - $(basename "$task" .md)"
        done
    fi
    
    echo ""
    echo "Recommended next mode: $MODE"
    echo ""
    echo "Use 'bash meta/scripts/shutdown-process.sh begin $MODE' to execute shutdown"
}

# Function to begin shutdown
begin_shutdown() {
    local mode_dir="$1"
    
    echo "=== BEGINNING SHUTDOWN SEQUENCE ==="
    echo ""
    
    # Run status check first
    if show_shutdown_status "$mode_dir"; then
        echo ""
        echo "Shutdown sequence completed successfully"
        echo "Session ready to end"
    else
        echo ""
        echo "Shutdown sequence incomplete - please address issues above"
        return 1
    fi
}

# Function to check documentation status
check_documentation_status() {
    echo "=== DOCUMENTATION CURRENCY CHECK ==="
    echo ""
    
    local doc_gaps=0
    
    # Check project documentation
    echo "Project Documentation:"
    local project_docs=("README.md" "ARCHITECTURE.md" "REQUIREMENTS.md" "PROJECTPLAN.md")
    for doc in "${project_docs[@]}"; do
        if [ -f "$PROJECT_ROOT/$doc" ]; then
            # Check if file has been recently modified (within last day)
            if [ "$PROJECT_ROOT/$doc" -nt "$PROJECT_ROOT/.git/HEAD" ] 2>/dev/null; then
                echo -e "  ${GREEN}✓ $doc (recently updated)${NC}"
            else
                echo -e "  ${YELLOW}⚠ $doc (may need updates)${NC}"
                doc_gaps=1
            fi
        else
            echo -e "  ${RED}✗ $doc missing${NC}"
            doc_gaps=1
        fi
    done
    
    echo ""
    echo "Implementation Documentation:"
    local impl_dirs=("gol-bevy" "gol-entt" "gol-flecs" "gol-console-client")
    for impl_dir in "${impl_dirs[@]}"; do
        if [ -d "$PROJECT_ROOT/$impl_dir" ]; then
            if [ -f "$PROJECT_ROOT/$impl_dir/README.md" ]; then
                echo -e "  ${GREEN}✓ $impl_dir/README.md${NC}"
            else
                echo -e "  ${RED}✗ $impl_dir/README.md missing${NC}"
                doc_gaps=1
            fi
        fi
    done
    
    echo ""
    if [ $doc_gaps -eq 0 ]; then
        echo -e "${GREEN}Documentation is current and comprehensive${NC}"
        echo "Documentation-first process was followed correctly"
    else
        echo -e "${RED}Documentation needs updates${NC}"
        echo "This indicates documentation-first process breakdown"
    fi
    
    return $doc_gaps
}

# Determine mode directory
case $MODE in
    "bevy")
        MODE_DIR="$PROJECT_ROOT/gol-bevy"
        ;;
    "entt")
        MODE_DIR="$PROJECT_ROOT/gol-entt"
        ;;
    "flecs")
        MODE_DIR="$PROJECT_ROOT/gol-flecs"
        ;;
    "console")
        MODE_DIR="$PROJECT_ROOT/gol-console-client"
        ;;
    "meta")
        MODE_DIR="$PROJECT_ROOT/meta"
        ;;
    "orchestrator"|"default")
        MODE_DIR="$PROJECT_ROOT"
        ;;
    *)
        echo "Error: Unknown mode '$MODE'"
        echo "Available modes: bevy, entt, flecs, console, meta, orchestrator, default"
        exit 1
        ;;
esac

# Execute command
case $COMMAND in
    "status")
        show_shutdown_status "$MODE_DIR"
        ;;
    "prepare")
        prepare_shutdown "$MODE_DIR"
        ;;
    "begin")
        begin_shutdown "$MODE_DIR"
        ;;
    "doc_check")
        check_documentation_status
        ;;
    *)
        echo "Error: Unknown command '$COMMAND'"
        echo "Available commands: status, prepare, begin, doc_check"
        exit 1
        ;;
esac