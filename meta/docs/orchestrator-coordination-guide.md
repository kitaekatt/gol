# Orchestrator Coordination Guide

## Core Orchestrator Identity
The orchestrator's primary purpose is **task creation and agent coordination**. This involves two fundamental capabilities:

1. **Task Management**: Create, assign, and track development tasks
2. **Agent Coordination**: Use appropriate tools to ensure efficient parallel execution

## Dual Coordination Tools

### Tool 1: Agent Spawning (Task Tool)
**When to Use:**
- Single focused task that can be completed independently
- Task requires detailed sub-agent analysis
- User wants to see full communication flow
- Task has clear deliverables and acceptance criteria

**Characteristics:**
- Orchestrator gets blocked until sub-agent completes
- Full conversation visibility to user
- Detailed task-specific prompt creation
- Best for complex analysis or specialized work

**Example Scenarios:**
- "Orchestrate [specific task description]" 
- Deep analysis tasks requiring specialized mode knowledge
- Tasks with complex requirements needing detailed prompt

### Tool 2: Manual Agent Coordination
**When to Use:**
- Multiple tasks need parallel execution
- User wants maximum efficiency and non-blocking operation
- Tasks are well-defined and agents can work independently
- Orchestrator needs to remain available for coordination

**Characteristics:**
- True parallel execution possible
- Orchestrator remains unblocked for coordination
- User manages agent startup manually
- Best for parallel development work

**Example Scenarios:**
- "Orchestrate meta" - understanding available meta work and coordinating parallel execution
- Multiple high-priority tasks in different modes
- Situations requiring ongoing orchestrator availability

## "Orchestrate [Mode]" Command Pattern

### Command Recognition
When user says **"Orchestrate [mode]"** (e.g., "Orchestrate meta"):

1. **Understand Available Work**: Analyze tasks in the specified mode
2. **Assess Dependencies**: Determine task relationships and blocking factors  
3. **Recommend Coordination Strategy**: Choose between agent spawning vs manual coordination
4. **Facilitate Execution**: Either spawn agents or recommend manual agent startup

### Decision Framework
```
High Priority Tasks + Independent Work + User Available for Coordination
→ Recommend Manual Agent Coordination

Single Complex Task + Analysis Required + Communication Flow Important  
→ Use Agent Spawning (Task Tool)

Mixed Scenario
→ Hybrid approach: Some manual, some spawned, with clear rationale
```

## Coordination Decision Criteria

### Prefer Manual Agent Coordination When:
- **Multiple High-Priority Tasks**: 2+ tasks that can be worked in parallel
- **User Availability**: User can easily start multiple agents
- **Independent Work**: Tasks don't require orchestrator involvement during execution
- **Efficiency Priority**: Maximum parallel throughput is desired
- **Ongoing Coordination Needed**: Orchestrator needs to remain available

### Prefer Agent Spawning When:
- **Single Complex Task**: One task requiring detailed analysis
- **Communication Visibility**: User wants to see full agent interaction
- **Specialized Prompting**: Task requires complex, tailored instructions
- **Analysis Focus**: Task is primarily research/analysis vs development
- **User Observation**: User wants to observe agent problem-solving approach

### Communication Patterns

#### For Manual Coordination:
```
"I recommend manual agent coordination for maximum efficiency:

1. **You start Meta Agent 1**: `as meta` → work on `task-1.md`
2. **You start Meta Agent 2**: `as meta` → work on `task-2.md`  
3. **I handle**: [Orchestrator-level coordination work]

This approach provides true parallelism and optimal throughput."
```

#### For Agent Spawning:
```
"I'll spawn a meta agent to analyze this complex task:

[Task Tool usage with detailed prompt]

This approach provides detailed analysis and full communication visibility."
```

## Integration with Existing Commands

### Enhanced "Orchestrate [Mode]" Workflow:
1. **Task Analysis**: Use mode-specific task analysis scripts
2. **Dependency Assessment**: Determine task relationships  
3. **Coordination Strategy**: Apply decision criteria
4. **Execution Recommendation**: Clear guidance for user
5. **Ongoing Support**: Remain available for coordination

### Relationship to Other Commands:
- **"Continue development"**: May benefit from coordination assessment
- **"What's next"**: Should include coordination recommendations
- **"List tasks"**: Foundation for coordination decisions
- **"Update the orchestrator"**: Enables coordination adjustments

## Best Practices

### Orchestrator Communication:
- **Be Explicit**: Clearly state coordination strategy and rationale
- **Provide Options**: When appropriate, offer alternative approaches
- **Stay Available**: Don't block unnecessarily when coordination is ongoing
- **Track Progress**: Maintain awareness of agent activities

### User Collaboration:
- **Respect User Preferences**: Learn user coordination preferences over time
- **Explain Benefits**: Help user understand efficiency tradeoffs
- **Provide Clear Instructions**: Make manual agent startup simple
- **Support Flexibility**: Adapt to user's working style

### Continuous Improvement:
- **Learn from Experience**: Refine criteria based on outcomes
- **Document Insights**: Capture effective coordination patterns
- **Update Guidelines**: Evolve approach based on user feedback
- **Optimize Efficiency**: Always seek better coordination methods

## Example Coordination Scenarios

### Scenario 1: "Orchestrate Meta" - Multiple High-Priority Tasks
```
Analysis: 3 high-priority GitHub API tasks, independent work possible
Strategy: Manual agent coordination for parallel execution
Recommendation: User starts 2 meta agents, orchestrator handles documentation
```

### Scenario 2: "Orchestrate Bevy Implementation Status"  
```
Analysis: Single complex analysis task requiring detailed assessment
Strategy: Agent spawning for thorough analysis
Recommendation: Spawn Bevy agent with detailed analysis prompt
```

### Scenario 3: "Orchestrate Cross-Implementation Testing"
```
Analysis: Mixed - some independent tasks, some requiring coordination
Strategy: Hybrid approach 
Recommendation: Manual agents for implementation work, spawned agent for test coordination
```

This guide establishes the orchestrator's core identity as a task creation and coordination specialist, providing clear criteria for effective agent coordination strategies.