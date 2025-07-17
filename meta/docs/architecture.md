# Architecture and Project Structure

## Current Project Structure
```
/mnt/c/dev/gol/
├── README.md                    # Project overview
├── ARCHITECTURE.md              # Unified design principles
├── REQUIREMENTS.md              # Functional specifications
├── PROJECTPLAN.md              # Development phases and tasks
├── CLAUDE.md                   # This file
├── gol-bevy/                   # Bevy ECS implementation
├── gol-entt/                   # EnTT ECS implementation
├── gol-flecs/                  # Flecs ECS implementation
├── gol-console-client/         # Unified terminal client
├── bevy-game-of-life/          # Legacy implementation (archived)
├── entts-game-of-life/         # Legacy implementation (archived)
└── flecs-game-of-life/         # Legacy implementation (archived)
```

## Success Criteria

### Individual Implementation
- All tasks completed successfully
- All tests passing
- Performance targets met
- Documentation complete

### Cross-Implementation
- Functional equivalence validated
- API compatibility confirmed
- Performance parity achieved
- Integration tests passing

### Project Completion
- All three implementations functional
- Console client working with all backends
- Comprehensive documentation
- Automated testing pipeline

## Following Conventions
When making changes to files, first understand the file's code conventions. Mimic code style, use existing libraries and utilities, and follow existing patterns.
- NEVER assume that a given library is available, even if it is well known. Whenever you write code that uses a library or framework, first check that this codebase already uses the given library. For example, you might look at neighboring files, or check the package.json (or cargo.toml, and so on depending on the language).
- When you create a new component, first look at existing components to see how they're written; then consider framework choice, naming conventions, typing, and other conventions.
- When you edit a piece of code, first look at the code's surrounding context (especially its imports) to understand the code's choice of frameworks and libraries. Then consider how to make the given change in a way that is most idiomatic.
- Always follow security best practices. Never introduce code that exposes or logs secrets and keys. Never commit secrets or keys to the repository.

## Code Style
- IMPORTANT: DO NOT ADD ***ANY*** COMMENTS unless asked