# Task: Bevy Integration Demo

## Task Overview
**Priority**: High
**Assigned Mode**: Console
**Dependencies**: Bevy server running
**Status**: Pending

## Coordination Metadata
**Task Classification**: Implementation-Specific
**Conflicts With**: None
**Depends On**: Bevy server running
**Modifies Files**: gol-console-client/src/*, gol-console-client/demo/*
**Reads Files**: patterns/*, proto/*, gol-bevy/target/* (for server)
**Parallel Safe**: Yes (with other implementations)
**Estimated Duration**: 45 minutes
**File Lock Priority**: Medium

## Description
Create a working end-to-end demo where the user can experience the Game of Life through the console client connected to the validated Bevy server. This is the first complete user-facing experience of the project.

## Requirements
- Console client successfully connects to Bevy server
- User can load patterns and see Game of Life simulation in terminal
- Interactive controls work (play, pause, step, reset)
- Real-time visualization updates smoothly
- Pattern files load correctly from patterns/ directory
- Clean user experience with clear instructions

## Implementation Steps
1. **Validate Console Build** - Ensure console client builds successfully
2. **Test Bevy Connection** - Verify gRPC connection to Bevy server
3. **Pattern Loading** - Test loading patterns from patterns/ directory
4. **UI Experience** - Ensure terminal visualization is clean and responsive
5. **Interactive Controls** - Validate play/pause/step/reset functionality
6. **User Documentation** - Create clear startup instructions

## User Experience Requirements
- **Simple Startup**: Clear instructions to start both server and client
- **Pattern Selection**: Easy way to load different Game of Life patterns
- **Visual Quality**: Clean terminal display with live cell visualization
- **Responsive Controls**: Smooth interaction with keyboard commands
- **Error Handling**: Graceful handling of connection issues

## Demo Script
Create a demonstration workflow:
1. Start Bevy server (instructions provided)
2. Start console client and connect to server
3. Load glider pattern and watch it move across screen
4. Try different patterns (block, blinker, beacon)
5. Use interactive controls (play/pause/step)
6. Show performance with more complex patterns

## Validation Criteria
- [ ] Console client builds without errors
- [ ] Successfully connects to Bevy server on port 50051
- [ ] Loads patterns from patterns/ directory
- [ ] Terminal UI renders Game of Life grid clearly
- [ ] Interactive controls (play/pause/step/reset) work
- [ ] Real-time updates show Game of Life evolution
- [ ] Multiple patterns can be loaded and simulated
- [ ] User can easily start and use the demo
- [ ] Performance is smooth for typical patterns
- [ ] Clear error messages for connection issues

## Success Metrics
- User can go from "nothing running" to "watching Game of Life" in under 2 minutes
- Terminal visualization is clear and engaging
- All standard patterns work correctly
- Controls feel responsive and intuitive
- Demo showcases the project's capabilities effectively

## Agent Instructions
- Focus on user experience quality - this is what the user will see first
- Test thoroughly with actual Bevy server connection
- Ensure patterns load correctly and display properly
- Validate that all interactive controls work as expected
- Create simple, clear startup instructions
- Update STATUS.md with integration results

## Next Steps After Completion
1. Document the complete startup process
2. Create user-friendly README with demo instructions
3. Enable development of EnTT and Flecs servers
4. Prepare for multi-backend client testing