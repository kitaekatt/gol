use criterion::{black_box, criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};
use bevy_game_of_life::systems::game_of_life::*;
use std::collections::HashSet;

fn bench_neighbor_counting(c: &mut Criterion) {
    let mut group = c.benchmark_group("neighbor_counting");
    
    for density in [0.1, 0.3, 0.5].iter() {
        for grid_size in [100, 500, 1000].iter() {
            let cell_count = (*grid_size * *grid_size) as f64 * density;
            group.throughput(Throughput::Elements(cell_count as u64));
            
            // Generate random live cells
            let mut live_cells = HashSet::new();
            let cells_to_generate = cell_count as usize;
            
            for i in 0..cells_to_generate {
                let x = (i % grid_size) as i32;
                let y = (i / grid_size) as i32;
                live_cells.insert((x, y));
            }
            
            group.bench_with_input(
                BenchmarkId::new("infinite_grid", format!("{}x{}_{}%", grid_size, grid_size, (density * 100.0) as u32)),
                &live_cells,
                |b, cells| {
                    b.iter(|| {
                        for &pos in cells.iter().take(100) { // Sample 100 cells
                            black_box(count_live_neighbors(pos, cells, false));
                        }
                    });
                },
            );
        }
    }
    group.finish();
}

fn bench_game_of_life_generation(c: &mut Criterion) {
    let mut group = c.benchmark_group("generation_step");
    
    for pattern_size in [10, 50, 100].iter() {
        // Create a glider pattern scaled up
        let mut cells = Vec::new();
        for scale in 0..*pattern_size/5 {
            let offset_x = scale * 10;
            let offset_y = scale * 10;
            
            // Add glider pattern
            cells.push((1 + offset_x, 0 + offset_y));
            cells.push((2 + offset_x, 1 + offset_y));
            cells.push((0 + offset_x, 2 + offset_y));
            cells.push((1 + offset_x, 2 + offset_y));
            cells.push((2 + offset_x, 2 + offset_y));
        }
        
        group.throughput(Throughput::Elements(cells.len() as u64));
        
        group.bench_with_input(
            BenchmarkId::new("full_generation", format!("{}_cells", cells.len())),
            &cells,
            |b, cells| {
                b.iter(|| {
                    black_box(apply_game_of_life_rules(cells, false, None, None));
                });
            },
        );
    }
    group.finish();
}

fn bench_sparse_vs_dense_patterns(c: &mut Criterion) {
    let mut group = c.benchmark_group("sparse_vs_dense");
    
    let grid_size = 100;
    
    // Sparse pattern (few scattered cells)
    let sparse_cells: Vec<(i32, i32)> = (0..20).map(|i| (i * 5, i * 5)).collect();
    
    // Dense pattern (filled rectangle)
    let mut dense_cells = Vec::new();
    for x in 0..20 {
        for y in 0..20 {
            dense_cells.push((x, y));
        }
    }
    
    group.bench_function("sparse_pattern", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&sparse_cells, false, None, None));
        });
    });
    
    group.bench_function("dense_pattern", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&dense_cells, false, None, None));
        });
    });
    
    group.finish();
}

fn bench_boundary_conditions(c: &mut Criterion) {
    let mut group = c.benchmark_group("boundary_conditions");
    
    let cells = vec![(0, 0), (1, 0), (2, 0)]; // Simple pattern
    
    group.bench_function("infinite_grid", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&cells, false, None, None));
        });
    });
    
    group.bench_function("wrapped_grid_10x10", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&cells, true, Some(10), Some(10)));
        });
    });
    
    group.bench_function("wrapped_grid_100x100", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&cells, true, Some(100), Some(100)));
        });
    });
    
    group.finish();
}

fn bench_classic_patterns(c: &mut Criterion) {
    let mut group = c.benchmark_group("classic_patterns");
    
    // Blinker pattern
    let blinker = vec![(1, 0), (1, 1), (1, 2)];
    
    // Block pattern (still life)
    let block = vec![(0, 0), (0, 1), (1, 0), (1, 1)];
    
    // Glider pattern
    let glider = vec![
        (1, 0),
        (2, 1),
        (0, 2), (1, 2), (2, 2)
    ];
    
    group.bench_function("blinker", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&blinker, false, None, None));
        });
    });
    
    group.bench_function("block", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&block, false, None, None));
        });
    });
    
    group.bench_function("glider", |b| {
        b.iter(|| {
            black_box(apply_game_of_life_rules(&glider, false, None, None));
        });
    });
    
    group.finish();
}

criterion_group!(
    benches,
    bench_neighbor_counting,
    bench_game_of_life_generation,
    bench_sparse_vs_dense_patterns,
    bench_boundary_conditions,
    bench_classic_patterns
);
criterion_main!(benches);