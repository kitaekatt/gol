use criterion::{black_box, criterion_group, criterion_main, Criterion};
use gol_bevy::components::{Position, CellState};
use gol_bevy::systems::{neighbor_counting_system, cell_lifecycle_system, cleanup_system};
use gol_bevy::resources::{GridConfig, SimulationState, Simulations};
use bevy::prelude::*;

fn setup_test_world(cell_count: usize) -> World {
    let mut world = World::new();
    
    // Initialize resources
    world.insert_resource(GridConfig::default());
    world.insert_resource(SimulationState::new());
    world.insert_resource(Simulations::new());
    
    // Create a glider pattern and replicate it
    let glider_pattern = vec![
        (1, 0), (2, 1), (0, 2), (1, 2), (2, 2)
    ];
    
    let mut entities_spawned = 0;
    let mut x_offset = 0;
    let mut y_offset = 0;
    
    // Spawn cells in a grid pattern to reach target count
    while entities_spawned < cell_count {
        for &(x, y) in &glider_pattern {
            if entities_spawned >= cell_count {
                break;
            }
            
            world.spawn((
                Position::new(x + x_offset, y + y_offset),
                CellState::new(),
            ));
            entities_spawned += 1;
        }
        
        x_offset += 10;
        if x_offset > 100 {
            x_offset = 0;
            y_offset += 10;
        }
    }
    
    world
}

fn benchmark_neighbor_counting(c: &mut Criterion) {
    let mut group = c.benchmark_group("neighbor_counting");
    
    for cell_count in [100, 500, 1000, 2000, 5000].iter() {
        group.bench_with_input(
            format!("cells_{}", cell_count),
            cell_count,
            |b, &cell_count| {
                let mut world = setup_test_world(cell_count);
                let mut system = IntoSystem::into_system(neighbor_counting_system);
                system.initialize(&mut world);
                
                b.iter(|| {
                    system.run((), &mut world);
                    world.clear_trackers();
                });
            },
        );
    }
    
    group.finish();
}

fn benchmark_lifecycle_system(c: &mut Criterion) {
    let mut group = c.benchmark_group("lifecycle_system");
    
    for cell_count in [100, 500, 1000, 2000, 5000].iter() {
        group.bench_with_input(
            format!("cells_{}", cell_count),
            cell_count,
            |b, &cell_count| {
                let mut world = setup_test_world(cell_count);
                let mut neighbor_system = IntoSystem::into_system(neighbor_counting_system);
                let mut lifecycle_system = IntoSystem::into_system(cell_lifecycle_system);
                let mut cleanup_system = IntoSystem::into_system(cleanup_system);
                
                neighbor_system.initialize(&mut world);
                lifecycle_system.initialize(&mut world);
                cleanup_system.initialize(&mut world);
                
                b.iter(|| {
                    // Run neighbor counting first
                    neighbor_system.run((), &mut world);
                    world.clear_trackers();
                    
                    // Then lifecycle
                    lifecycle_system.run((), &mut world);
                    world.clear_trackers();
                    
                    // Then cleanup
                    cleanup_system.run((), &mut world);
                    world.clear_trackers();
                });
            },
        );
    }
    
    group.finish();
}

fn benchmark_full_generation(c: &mut Criterion) {
    let mut group = c.benchmark_group("full_generation");
    
    for cell_count in [100, 500, 1000, 2000].iter() {
        group.bench_with_input(
            format!("cells_{}", cell_count),
            cell_count,
            |b, &cell_count| {
                b.iter(|| {
                    let mut world = setup_test_world(cell_count);
                    let mut neighbor_system = IntoSystem::into_system(neighbor_counting_system);
                    let mut lifecycle_system = IntoSystem::into_system(cell_lifecycle_system);
                    let mut cleanup_system = IntoSystem::into_system(cleanup_system);
                    
                    neighbor_system.initialize(&mut world);
                    lifecycle_system.initialize(&mut world);
                    cleanup_system.initialize(&mut world);
                    
                    // Run 10 generations
                    for _ in 0..10 {
                        neighbor_system.run((), &mut world);
                        world.clear_trackers();
                        
                        lifecycle_system.run((), &mut world);
                        world.clear_trackers();
                        
                        cleanup_system.run((), &mut world);
                        world.clear_trackers();
                    }
                });
            },
        );
    }
    
    group.finish();
}

fn benchmark_memory_usage(c: &mut Criterion) {
    let mut group = c.benchmark_group("memory_usage");
    
    group.bench_function("component_size", |b| {
        b.iter(|| {
            let position = Position::new(black_box(42), black_box(17));
            let cell_state = CellState::new();
            black_box((position, cell_state));
        });
    });
    
    group.finish();
}

criterion_group!(
    benches,
    benchmark_neighbor_counting,
    benchmark_lifecycle_system,
    benchmark_full_generation,
    benchmark_memory_usage
);
criterion_main!(benches);