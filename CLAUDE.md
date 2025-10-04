# CLAUDE.md - Performance-Aware Programming Standards

## Core Philosophy

**Measure, don't guess.** Performance optimization is an empirical science. Always profile before optimizing, and verify improvements with measurements.

**Understand the hardware.** Modern CPUs, memory hierarchies, and I/O systems have specific characteristics that determine performance. Write code that works with the hardware, not against it.

**Simplicity enables performance.** Complex abstractions often hide performance costs. Prefer simple, direct code that clearly expresses what the machine needs to do.

---

## Performance Principles

### 1. Data-Oriented Design
- Design around data access patterns, not abstract concepts
- Consider cache behavior: sequential access >> random access
- Structure of Arrays (SoA) often beats Array of Structures (AoS)
- Keep hot data together, cold data separate
- Minimize pointer chasing and indirection

### 2. Memory is the Bottleneck
- CPU cycles are cheap; memory access is expensive
- L1 cache: ~4 cycles, L2: ~10 cycles, L3: ~40 cycles, RAM: ~200+ cycles
- Design to minimize cache misses
- Understand your working set size
- Batch similar operations to maintain cache locality

### 3. Bandwidth vs Latency
- Optimize for bandwidth when processing large amounts of data
- Optimize for latency when response time matters
- Prefetching and pipelining can hide latency
- SIMD can multiply bandwidth

### 4. Measurement Methodology
```
Measure wall clock time for real-world performance
Use cycle counters for detailed analysis
Test on representative data and workloads
Measure multiple times to account for variance
Profile hot paths, not assumptions
```

### 5. Common Performance Killers
- **Allocations**: Memory allocation is expensive; reuse buffers
- **Branching**: Unpredictable branches cause pipeline stalls
- **Virtual calls**: Indirect calls prevent inlining and optimization
- **Excessive abstraction**: Each layer adds overhead
- **String operations**: Often surprisingly expensive
- **Locks**: Contention destroys parallelism

---

## Code Standards

### C/C++ Performance

```c
// GOOD: Sequential access, cache-friendly
void ProcessEntities(Entity* entities, int count) {
    for (int i = 0; i < count; i++) {
        entities[i].position.x += entities[i].velocity.x * dt;
        entities[i].position.y += entities[i].velocity.y * dt;
    }
}

// BETTER: SoA layout for better cache utilization
void ProcessEntities(EntitySoA* entities, int count) {
    for (int i = 0; i < count; i++) {
        entities->positions_x[i] += entities->velocities_x[i] * dt;
        entities->positions_y[i] += entities->velocities_y[i] * dt;
    }
}

// AVOID: Pointer chasing, cache-unfriendly
void ProcessEntities(Entity** entities, int count) {
    for (int i = 0; i < count; i++) {
        entities[i]->position.x += entities[i]->velocity.x * dt;
        entities[i]->position.y += entities[i]->velocity.y * dt;
    }
}
```

### Memory Management

```c
// Prefer stack allocation or arena allocators
typedef struct {
    u8* base;
    size_t used;
    size_t size;
} Arena;

void* PushSize(Arena* arena, size_t size) {
    assert(arena->used + size <= arena->size);
    void* result = arena->base + arena->used;
    arena->used += size;
    return result;
}

// Batch allocations, minimize malloc/free calls
// Clear and reuse instead of free/alloc
```

### Avoid Premature Abstraction

```c
// GOOD: Direct, measurable, optimizable
f32 Distance(v2 a, v2 b) {
    f32 dx = b.x - a.x;
    f32 dy = b.y - a.y;
    return sqrtf(dx*dx + dy*dy);
}

// AVOID: Hidden costs, hard to optimize
template<typename T>
auto Distance(const Vector<T, 2>& a, const Vector<T, 2>& b)
    -> decltype(std::sqrt(std::declval<T>())) {
    return (a - b).magnitude();
}
```

### SIMD When It Matters

```c
// Use SIMD for hot paths processing multiple values
__m128 positions_x = _mm_load_ps(&entities->x[i]);
__m128 velocities_x = _mm_load_ps(&entities->vx[i]);
__m128 dt_vec = _mm_set1_ps(dt);
positions_x = _mm_add_ps(positions_x, _mm_mul_ps(velocities_x, dt_vec));
_mm_store_ps(&entities->x[i], positions_x);
```

---

## Profiling & Measurement

### Essential Metrics
- **Wall clock time**: Real-world performance
- **CPU cycles**: Instruction-level detail
- **Cache misses**: L1/L2/L3 miss rates
- **Branch mispredictions**: Pipeline efficiency
- **Instructions retired**: Work accomplished
- **Bandwidth utilization**: Memory/disk throughput

### Profiling Tools
- CPU cycle counters (RDTSC on x86)
- perf (Linux)
- VTune (Intel)
- Tracy Profiler
- Custom instrumentation for critical paths

### Timing Code

```c
#include <time.h>

u64 ReadCPUTimer() {
    return __rdtsc();
}

u64 start = ReadCPUTimer();
// ... code to measure ...
u64 end = ReadCPUTimer();
u64 cycles = end - start;
```

---

## C# Performance Considerations

While C# has garbage collection overhead, performance-aware practices still apply:

```csharp
// Minimize allocations in hot paths
Span<int> values = stackalloc int[256]; // Stack allocation

// Reuse collections
private List<Entity> _entityList = new(capacity: 1000);

// Avoid LINQ in performance-critical code
for (int i = 0; i < entities.Count; i++) {
    ProcessEntity(entities[i]); // Direct iteration
}

// Use structs for small, short-lived data
public struct Vector2 {
    public float X, Y;
}

// Profile allocations with dotMemory or PerfView
```

---

## Design Principles

### Start Simple
1. Write straightforward code first
2. Measure to find bottlenecks
3. Optimize hot paths only
4. Measure again to verify improvement

### Question Abstractions
- Does this abstraction have measurable benefit?
- What is the performance cost?
- Can I achieve the same goal more directly?

### Understand Trade-offs
- **Code clarity vs performance**: Sometimes performance wins
- **Development time vs runtime**: Optimize what runs often
- **Portability vs optimization**: Know your target platform

### Batch Operations
- Group similar work together
- Maintain cache locality
- Reduce function call overhead
- Enable SIMD opportunities

---

## Anti-Patterns to Avoid

❌ **Over-engineering**: Don't add complexity "for future flexibility"
❌ **Premature optimization**: But also don't write wasteful code by default
❌ **Ignoring data layout**: Structure drives performance
❌ **Excessive inheritance**: Virtual calls and cache pollution
❌ **String concatenation in loops**: Allocations everywhere
❌ **Trusting compiler magic**: Verify what's actually generated
❌ **Assuming "good enough"**: Measure actual performance

---

## Testing Performance

```c
// Benchmark with realistic data and workloads
void BenchmarkFunction(TestData* data, int iterations) {
    u64 min_cycles = UINT64_MAX;

    for (int test = 0; test < iterations; test++) {
        u64 start = ReadCPUTimer();
        FunctionUnderTest(data);
        u64 end = ReadCPUTimer();

        u64 elapsed = end - start;
        if (elapsed < min_cycles) {
            min_cycles = elapsed;
        }
    }

    printf("Best: %llu cycles\n", min_cycles);
}
```

---

## Remember

> "The purpose of abstractions in programming is to reduce the complexity of the problem to something that fits in a human mind. If your abstractions don't do that, they're bad abstractions."

> "You can't optimize what you don't measure."

> "Make it work, make it right, make it fast - in that order."

**Measure everything. Question assumptions. Understand your hardware.**