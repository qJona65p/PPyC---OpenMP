### Execution time by scheduler and chunk size

| Chunk Size | Dynamic (s) | Static (s)  | Guided (s)  |
| ---------- | ----------- | ----------- | ----------- |
| 1          | 1.19301     | 1.24539     | 1.37554     |
| 2          | 1.21258     | 1.25504     | 1.40224     |
| 4          | 1.15799     | 1.18727     | **1.37467** |
| 8          | **1.15336** | **1.19625** | 1.42052     |
| 16         | 1.17552     | 1.26987     | 1.46039     |
| 32         | 1.18561     | 1.27342     | 1.41108     |
| 64         | 1.34727     | 1.41294     | 1.53674     |
| 128        | 1.39833     | 1.38828     | 1.71127     |

### Best configuration for each scheduler

| Scheduler | Best Chunk Size | Best Time (s) |
| --------- | --------------- | ------------- |
| Dynamic   | 8               | **1.15336**   |
| Static    | 4               | **1.18727**   |
| Guided    | 4               | **1.37467**   |

### Relative performance against the fastest result (`dynamic`, chunk = 8)

| Scheduler | Time (s) | Slowdown |
| --------- | -------- | -------- |
| Dynamic   | 1.15336  | Baseline |
| Static    | 1.18727  | +2.9%    |
| Guided    | 1.37467  | +19.2%   |
