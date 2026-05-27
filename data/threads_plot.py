import matplotlib.pyplot as plt

plt.figure(figsize=(6,4))
plt.plot([i for i in range(1, 17)], [22.40440,12.11910,8.67843,6.98253,6.05611,5.37496,4.96944,4.63854,4.52371,4.33845,4.15566,4.07319,3.95097,3.87049,3.86496,3.75561])
plt.title('Threads vs Execution Time')
plt.ylabel('Execution Time')
plt.xlabel('Thread Count')
plt.savefig("threads_vs_time_plot.jpg")

plt.figure(figsize=(6,4))
plt.plot([i for i in range(1, 17)], [1.00,1.85,2.58,3.21,3.70,4.17,4.51,4.83,4.95,5.16,5.39,5.50,5.67,5.79,5.80,5.97])
plt.title('Speedup vs Thread Count')
plt.ylabel('Speedup')
plt.xlabel('Thread Count')
plt.savefig("threads_vs_speedup.jpg")

