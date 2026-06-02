import matplotlib.pyplot as plt

threads = [1, 2, 3, 4]

block_speedup = [1, 1.54669, 2.6212, 2.32606]
mutex_queue_speedup = [1, 1.76962, 2.64925, 3.14593]
atomic_index_speedup = [1, 1.70836, 2.7429, 3.31191]

block_efficiency = [1, 0.773345, 0.873735, 0.581515]
mutex_queue_efficiency = [1, 0.88481, 0.883083, 0.786482]
atomic_index_efficiency = [1, 0.854179, 0.9143, 0.827978]

# График ускорения
plt.figure(figsize=(8, 5))

plt.plot(threads, block_speedup, marker='o', label='block')
plt.plot(threads, mutex_queue_speedup, marker='o', label='mutex_queue')
plt.plot(threads, atomic_index_speedup, marker='o', label='atomic_index')

plt.title('График ускорения')
plt.xlabel('Количество потоков')
plt.ylabel('Speedup')

plt.xticks(threads)
plt.grid(True)
plt.legend()

plt.savefig('speedup.png', dpi=300, bbox_inches='tight')
plt.show()

# График эффективности
plt.figure(figsize=(8, 5))

plt.plot(threads, block_efficiency, marker='o', label='block')
plt.plot(threads, mutex_queue_efficiency, marker='o', label='mutex_queue')
plt.plot(threads, atomic_index_efficiency, marker='o', label='atomic_index')

plt.title('График эффективности')
plt.xlabel('Количество потоков')
plt.ylabel('Efficiency')

plt.xticks(threads)
plt.grid(True)
plt.legend()

plt.savefig('efficiency.png', dpi=300, bbox_inches='tight')
plt.show()
