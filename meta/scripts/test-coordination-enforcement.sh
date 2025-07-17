#!/bin/bash

# Coordination Enforcement System Test Script
# Tests all components of the coordination enforcement system
# Usage: bash meta/scripts/test-coordination-enforcement.sh

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

echo "=== COORDINATION ENFORCEMENT SYSTEM TEST ==="
echo ""

# Test counter
tests_passed=0
tests_total=0

# Function to run test
run_test() {
    local test_name="$1"
    local test_command="$2"
    local expected_result="$3"  # 0 = success, non-zero = failure expected
    
    echo -n "Testing: $test_name ... "
    tests_total=$((tests_total + 1))
    
    if eval "$test_command" >/dev/null 2>&1; then
        result=0
    else
        result=1
    fi
    
    if [[ $result -eq $expected_result ]]; then
        echo -e "${GREEN}PASS${NC}"
        tests_passed=$((tests_passed + 1))
    else
        echo -e "${RED}FAIL${NC}"
        echo "  Expected: $expected_result, Got: $result"
    fi
}

# Test 1: Check if coordination scripts exist
run_test "Coordination scripts exist" "test -f '$SCRIPT_DIR/enforce-coordination.sh' && test -f '$SCRIPT_DIR/mandatory-coordination-check.sh' && test -f '$SCRIPT_DIR/coordination-recovery.sh'" 0

# Test 2: Check if scripts are executable
run_test "Scripts are executable" "test -x '$SCRIPT_DIR/enforce-coordination.sh' && test -x '$SCRIPT_DIR/mandatory-coordination-check.sh' && test -x '$SCRIPT_DIR/coordination-recovery.sh'" 0

# Test 3: Test coordination enforcement with invalid mode
run_test "Invalid agent mode rejection" "bash '$SCRIPT_DIR/enforce-coordination.sh' invalid-mode test-task.md" 1

# Test 4: Test coordination enforcement with non-existent task
run_test "Non-existent task rejection" "bash '$SCRIPT_DIR/enforce-coordination.sh' meta non-existent-task.md" 1

# Test 5: Test coordination check with invalid mode
run_test "Invalid mode in coordination check" "bash '$SCRIPT_DIR/mandatory-coordination-check.sh' invalid-mode validate" 1

# Test 6: Test coordination check with invalid action
run_test "Invalid action in coordination check" "bash '$SCRIPT_DIR/mandatory-coordination-check.sh' meta invalid-action" 1

# Test 7: Test recovery system show_conflicts
run_test "Recovery system show_conflicts" "bash '$SCRIPT_DIR/coordination-recovery.sh' show_conflicts" 0

# Test 8: Test task claiming system
run_test "Task claiming system exists" "test -f '$SCRIPT_DIR/claim-task.sh' && test -x '$SCRIPT_DIR/claim-task.sh'" 0

# Test 9: Test task availability checker
run_test "Task availability checker exists" "test -f '$SCRIPT_DIR/check-task-availability.sh' && test -x '$SCRIPT_DIR/check-task-availability.sh'" 0

# Test 10: Test enhanced task analysis
run_test "Enhanced task analysis exists" "test -f '$SCRIPT_DIR/enhanced-analyze-agent-tasks.sh' && test -x '$SCRIPT_DIR/enhanced-analyze-agent-tasks.sh'" 0

# Test 11: Test coordination recovery clear_stale_claims
run_test "Recovery system clear_stale_claims" "bash '$SCRIPT_DIR/coordination-recovery.sh' clear_stale_claims" 0

# Test 12: Test coordination recovery cleanup_locks
run_test "Recovery system cleanup_locks" "bash '$SCRIPT_DIR/coordination-recovery.sh' cleanup_locks" 0

# Test 13: Test coordination system integration
run_test "Coordination system Python integration" "test -f '$PROJECT_ROOT/meta/coordination/coordination-system.py'" 0

# Test 14: Test coordination directory structure
run_test "Coordination directory structure" "test -d '$PROJECT_ROOT/meta/coordination' && test -f '$PROJECT_ROOT/meta/coordination/coordination-protocol.md'" 0

# Test 15: Test agent registration system
run_test "Agent registration system exists" "test -f '$SCRIPT_DIR/register-agent.sh' && test -x '$SCRIPT_DIR/register-agent.sh'" 0

echo ""
echo "=== TEST RESULTS ==="
echo "Tests passed: $tests_passed/$tests_total"

if [[ $tests_passed -eq $tests_total ]]; then
    echo -e "${GREEN}✅ ALL TESTS PASSED${NC}"
    echo ""
    echo "Coordination enforcement system is ready for use!"
    echo ""
    echo "Usage Summary:"
    echo "1. Before task work: bash meta/scripts/enforce-coordination.sh [mode] [task-file]"
    echo "2. During development: bash meta/scripts/mandatory-coordination-check.sh [mode] continue_development"
    echo "3. Recovery: bash meta/scripts/coordination-recovery.sh [action]"
    echo "4. Status: bash meta/scripts/coordination-recovery.sh show_conflicts"
    echo ""
    echo "All agents must now use coordination enforcement before task work."
    exit 0
else
    echo -e "${RED}❌ SOME TESTS FAILED${NC}"
    echo ""
    echo "Please fix the failing tests before using the coordination system."
    exit 1
fi