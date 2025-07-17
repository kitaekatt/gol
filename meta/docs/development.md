# Development Commands and Quality Assurance

## Package Management
- **Python**: Use pipx (user preference)
- **Node.js**: Use npx (user preference)
- **System**: Use apt (user preference)
- **Always ask user permission** before installing packages
- **NEVER use sudo** - prompt user to run command if needed

## Development Workflow
- **Documentation First**: Always refer to REQUIREMENTS.md and ARCHITECTURE.md before starting work
- **Real-time Documentation**: Update documentation DURING task completion, not after
- **Command Documentation**: When creating new commands, IMMEDIATELY document them in CLAUDE.md
- **Task Focus**: Complete assigned tasks before taking on additional work
- **Testing**: Write tests for all new functionality
- **Communication**: Report progress and issues clearly
- **Quality**: Maintain high code quality standards
- **Consistency**: Ensure cross-implementation equivalence
- **Performance**: Meet all performance requirements

## Quality Assurance Requirements
- **VERY IMPORTANT**: When you have completed a task, you MUST run the lint and typecheck commands (eg. npm run lint, npm run typecheck, ruff, etc.) with Bash if they were provided to you to ensure your code is correct
- If you are unable to find the correct command, ask the user for the command to run
- If they supply it, proactively suggest writing it to CLAUDE.md so that you will know to run it next time
- **Documentation Currency**: Verify documentation was updated during task completion before marking task complete
- **Documentation-First Validation**: Confirm "Is documentation up to date?" can be answered "Yes"
- NEVER commit changes unless the user explicitly asks you to
- It is VERY IMPORTANT to only commit when explicitly asked, otherwise the user will feel that you are being too proactive

## Documentation Learning Protocol
Capture implementation insights at completion points to make future re-implementation faster:

**Documentation Targets by Insight Type**:
- **CLAUDE.md**: Workflow improvements, command patterns, process optimizations
- **ARCHITECTURE.md**: Design decisions, architectural insights, framework comparisons
- **REQUIREMENTS.md**: Requirement clarifications discovered during implementation
- **Implementation READMEs**: Framework-specific patterns, gotchas, and best practices
- **PROJECTPLAN.md**: Process improvements, timeline insights, task breakdown optimizations

**Key Documentation Goals**:
- Enable faster re-implementation from scratch using improved documentation
- Capture "why" behind decisions, not just "what" was implemented
- Document framework-specific insights that prevent repeated discovery
- Record optimal development sequences and patterns