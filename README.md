# vectorized_ford_johnson

This project now uses a structure-of-arrays pipeline for pair normalization and pair ordering:
- AVX2 on `x86_64`
- NEON on `arm64`
- scalar fallback when SIMD is unavailable

The vector path on Apple Silicon builds the pair blocks directly from the raw input with a NEON deinterleave step, then applies partner-bounded Jacobsthal insertions on the main chain.

```bash
make MODE=fast opti
make MODE=mincmp opti
```

# Example
```bash
./PmergeMe-fast $(jot -r 3000 1 3000)
./PmergeMe-mincmp --stats $(jot -r 3000 1 3000)
```

the paper need to be updated
