### OpenMP affinity and environment configuration comparison

| Configuration      | Threads | Proc Bind | Places  |  Task A (s) |  Task B (s) | Total Time (s) |    Cache Misses |        Cache Refs | Miss Rate |
| ------------------ | ------: | --------- | ------- | ----------: | ----------: | -------------: | --------------: | ----------------: | --------: |
| Default (no flags) |      16 | Default   | Default |     1.74231 |     1.17419 |        5.21041 |     146,991,640 |     5,854,898,201 |     2.51% |
| Config 1           |      16 | spread    | cores   |     1.60743 |     1.13877 |        5.05193 |     141,804,411 |     6,017,438,968 |     2.36% |
| Config 2           |       8 | spread    | cores   | **1.49319** |     1.15814 |        4.84301 |     129,037,277 |     6,199,677,332 |     2.08% |
| Config 3           |      16 | closed    | threads |     1.53771 | **0.94772** |    **4.58780** | **106,943,812** | **6,248,842,853** | **1.71%** |
| Config 4           |       8 | closed    | cores   |     1.65799 |     1.15390 |        4.96444 |     113,640,014 |     6,219,672,619 |     1.83% |

### Improvement relative to baseline

(Baseline = no environment flags)

| Configuration                  | Total Time Improvement | Cache Miss Reduction |
| ------------------------------ | ---------------------: | -------------------: |
| Config 1 (16, spread, cores)   |                   3.0% |                 3.5% |
| Config 2 (8, spread, cores)    |                   7.0% |                12.2% |
| Config 3 (16, closed, threads) |              **11.9%** |            **27.2%** |
| Config 4 (8, closed, cores)    |                   4.7% |                22.7% |

### Key observations

| Observation                            | Result                                                             |
| -------------------------------------- | ------------------------------------------------------------------ |
| Fastest total execution                | `OMP_PROC_BIND=closed`, `OMP_PLACES=threads`, `OMP_NUM_THREADS=16` |
| Lowest cache misses                    | `OMP_PROC_BIND=closed`, `OMP_PLACES=threads`, `OMP_NUM_THREADS=16` |
| Fastest Mandelbrot generation (Task A) | `OMP_NUM_THREADS=8`, `spread`, `cores`                             |
| Fastest Gaussian Blur (Task B)         | `OMP_PROC_BIND=closed`, `OMP_PLACES=threads`, `OMP_NUM_THREADS=16` |


### Monitor cache usage while using OMP PROC BIND y OMP PLACES
Using this command to monitor cache usage.
perf stat -e cache-misses,cache-references ./mandelbrot_omp