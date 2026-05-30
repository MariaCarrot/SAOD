import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("times.csv")

plt.figure(figsize=(8, 5))
df.boxplot(column="time_us", by="algorithm")

plt.title("Boxplot времени поиска")
plt.suptitle("")
plt.xlabel("Алгоритм")
plt.ylabel("Время, мкс")
plt.grid(True)

plt.savefig("boxplot.png", dpi=300)
plt.show()

table = df.groupby("algorithm")["time_us"].agg(
    ["mean", "min", "max", "median"]
)

print(table)
table.to_csv("stats_table.csv")
