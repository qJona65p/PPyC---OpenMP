### Histogram implementation comparison

| Implementation   | Execution Time (s) | Unique Colors | Speedup vs Critical |
| ---------------- | -----------------: | ------------: | ------------------: |
| Critical section |            6.88902 |          1510 |               1.00× |
| Local variables  |       **0.023033** |          1510 |         **299.09×** |

### Top 10 most frequent colors

(The results are identical for both implementations)

| Rank |   R |  G |  B |    HEX    |    Pixels |
| ---: | --: | -: | -: | :-------: | --------: |
|    1 |   0 |  0 |  0 | `#000000` | 5,963,229 |
|    2 | 254 | 57 | 70 | `#FE3946` | 3,649,794 |
|    3 | 254 | 55 | 72 | `#FE3748` | 2,769,433 |
|    4 | 254 | 56 | 71 | `#FE3847` | 2,468,499 |
|    5 | 254 | 60 | 67 | `#FE3C43` | 2,451,282 |
|    6 | 254 | 59 | 67 | `#FE3B43` | 1,956,958 |
|    7 | 254 | 59 | 68 | `#FE3B44` | 1,557,647 |
|    8 | 254 | 54 | 73 | `#FE3649` | 1,290,245 |
|    9 | 254 | 53 | 74 | `#FE354A` | 1,190,532 |
|   10 | 254 | 58 | 69 | `#FE3A45` |   860,040 |
