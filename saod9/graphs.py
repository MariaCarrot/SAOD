import matplotlib.pyplot as plt

threads = [1, 2, 3, 4]

block_speedup = [1, 1.68065, 1.90776, 2.26422]
mutex_queue_speedup = [1, 2.26347, 2.99984, 6.12898]
atomic_index_speedup = [1, 1.63253, 2.59409, 5.65968]

block_efficiency = [1, 0.840327, 0.635919, 0.566054]
mutex_queue_efficiency = [1, 1.13173, 0.999946, 1.53224]
atomic_index_efficiency = [1, 0.816265, 0.864697, 1.41492]

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
