# Example Orchestration Task - Bevy Performance Analysis

## Task Overview
**Priority**: Medium  
**Assigned Mode**: Bevy  
**Created By**: Orchestrator sub-task orchestration  
**Purpose**: Demonstrate the orchestration workflow pattern

## Objective
Analyze the current Bevy implementation's performance characteristics and provide optimization recommendations.

## Context
This is an example task demonstrating the orchestration workflow. The orchestrator creates this task file and spawns a sub-task to delegate work to the Bevy agent while maintaining mode boundaries.

## Requirements
1. Analyze current Bevy implementation performance metrics
2. Identify potential bottlenecks in the ECS system
3. Evaluate memory usage patterns
4. Provide specific optimization recommendations
5. Compare performance against project requirements

## Agent Constraints
- Work exclusively within gol-bevy/ directory scope
- Focus on Bevy ECS implementation patterns only
- Cannot modify other implementations or orchestrator files
- Maintain mode boundaries and constraints
- End response with [Mode:Bevy] declaration

## Expected Deliverables
1. Performance analysis report
2. Bottleneck identification
3. Memory usage assessment
4. Optimization recommendations
5. Requirements compliance evaluation

## Acceptance Criteria
- [ ] Performance metrics documented
- [ ] Bottlenecks identified with specific line references
- [ ] Memory usage patterns analyzed
- [ ] Optimization recommendations provided
- [ ] Requirements compliance verified
- [ ] Response ends with [Mode:Bevy] declaration

## Communication Flow Example
```
1. ORCHESTRATOR SENDS PROMPT TO BEVY AGENT:
   "You are operating in Bevy Mode... analyze performance..."

2. EXECUTING SUBTASK:
   Task(Bevy performance analysis)
   ⎿ [Bevy agent analyzes gol-bevy/ and provides detailed performance report]

3. ORCHESTRATOR RECEIVES SUBTASK RESPONSE:
   [Performance analysis with specific findings and recommendations]

4. ORCHESTRATOR ANALYSIS:
   [Orchestrator processes results and determines next steps]

5. ORCHESTRATOR DECISION:
   [Actions to take based on performance analysis]
```

## Notes
- This task demonstrates the high-fidelity orchestration workflow
- Task file creation is mandatory for all orchestration requests
- Full communication flow must be visible to user
- Orchestrator delegates rather than performing restricted work directly

## Task Completion Protocol
Upon completion:
1. Bevy agent provides comprehensive performance analysis
2. Orchestrator receives and processes results
3. Orchestrator moves task file to completed/ directory
4. Orchestrator determines next steps based on findings