import matplotlib.pyplot as plt

#Amdahls Law
def max_theoric_speedup(N, P):
    return 1/((1-P)+(P/N))

# Threads vs Exec Time
plt.figure(figsize=(8,5))
plt.plot([i for i in range(1, 17)], [22.40440,12.11910,8.67843,6.98253,6.05611,5.37496,4.96944,4.63854,4.52371,4.33845,4.15566,4.07319,3.95097,3.87049,3.86496,3.75561], c="blue")
plt.plot([i for i in range(1, 17)], [14.55100,7.30743,4.86816,3.68461,2.95771,2.52630,2.16670,1.93162,1.79413,1.65906,1.54750,1.42096,1.37945,1.31011,1.28887,1.22814], c="green")
plt.plot([i for i in range(1, 17)], [6.14614,3.08553,2.08034,1.60732,1.36441,1.17882,1.05869,0.95921,0.98691,0.91921,0.86451,0.83276,0.83895,0.77777,0.81663,0.78671], c="orange")

plt.scatter([9], [4.52371], c="red", marker="o", s=15, zorder=5)
plt.scatter([9], [4.52371], c="blue", marker="o", s=50, zorder=4)

plt.scatter([9], [1.79413], c="red", marker="o", s=15, zorder=5)
plt.scatter([9], [1.79413], c="green", marker="o", s=50, zorder=4)

plt.scatter([9], [0.98691], c="red", marker="o", s=15, zorder=5)
plt.scatter([9], [0.98691], c="orange", marker="o", s=50, zorder=4)

plt.title('Threads vs Execution Time')
plt.ylabel('Execution Time')
plt.xlabel('Thread Count')
plt.legend(["Tiempo de ejecución total", "Tiempo de ejecución Tarea A", "Tiempo de ejecución Tarea B", "Puntos de degradación por overhead"])
plt.grid(True)
plt.xticks(range(1, 17))
plt.tight_layout()
plt.savefig("threads_vs_time_plot.jpg")

# Threads vs Speedup
plt.figure(figsize=(8,5))
plt.plot([i for i in range(1, 17)], [1.00,1.85,2.58,3.21,3.70,4.17,4.51,4.83,4.95,5.16,5.39,5.50,5.67,5.79,5.80,5.97], c="blue", zorder=2)
plt.plot([i for i in range(1, 17)], [max_theoric_speedup(i, 1) for i in range(1, 17)], zorder=3)
plt.plot([i for i in range(1, 17)], [max_theoric_speedup(i, 0.888) for i in range(1, 17)], c="red", linestyle="--", zorder=4)
plt.scatter([9], [4.95], c="orange", marker="o", s=15, zorder=5)
plt.scatter([9], [4.95], c="blue", marker="o", s=50, zorder=4)
plt.title('Speedup vs Thread Count')
plt.ylabel('Speedup')
plt.xlabel('Thread Count')
plt.legend(["Speedup real", "Speedup Ideal", "Speedup máximo teórico con P=0.888", "Punto de degradación por overhead"])
plt.grid(True)
plt.xticks(range(1, 17))
plt.tight_layout()
plt.savefig("threads_vs_speedup.jpg")

