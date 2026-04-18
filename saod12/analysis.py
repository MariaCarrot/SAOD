import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np

df = pd.read_csv("results.csv")

avg = df.groupby(["size", "generator", "sort"], as_index=False).agg({
    "time": "mean",
    "comparisons": "mean",
    "pointer_changes": "mean"
})

# чтобы log-шкала работала для времени
avg["time"] = avg["time"].replace(0, 1e-6)

# короткие названия сортировок
avg["sort"] = avg["sort"].replace({
    "Insertion Sort": "Insertion",
    "Merge Sort": "Merge"
})

generators = ["Random", "Sorted", "Reverse", "Almost Sorted"]


def power_of_two_formatter(x, pos):
    if x <= 0:
        return ""
    k = np.log2(x)
    if abs(k - round(k)) < 1e-10:
        return rf"$2^{{{int(round(k))}}}$"
    return ""


def plot_metric_grid_base2(data, metric, y_label, title, filename=None):
    fig, axes = plt.subplots(2, 2, figsize=(12, 9))
    axes = axes.flatten()

    for i, gen in enumerate(generators):
        ax = axes[i]
        subset = data[data["generator"] == gen]

        for sort_name in ["Insertion", "Merge"]:
            s = subset[subset["sort"] == sort_name].sort_values("size")
            ax.plot(s["size"], s[metric], marker="o", label=sort_name)

        ax.set_title(gen)
        ax.set_xlabel("Размер списка")
        ax.set_ylabel(y_label)

        # логарифмические оси по основанию 2
        ax.set_xscale("log", base=2)
        ax.set_yscale("log", base=2)

        # тики по X в степенях двойки
        ax.xaxis.set_major_locator(mticker.LogLocator(base=2))
        ax.xaxis.set_major_formatter(mticker.FuncFormatter(power_of_two_formatter))

        # тики по Y тоже в степенях двойки
        ax.yaxis.set_major_locator(mticker.LogLocator(base=2))
        ax.yaxis.set_major_formatter(mticker.FuncFormatter(power_of_two_formatter))

        ax.grid(True, which="both", linestyle="--", alpha=0.6)
        ax.legend()

    fig.suptitle(title, fontsize=14)
    fig.tight_layout()

    if filename:
        fig.savefig(filename, dpi=300, bbox_inches="tight")

    plt.show()


plot_metric_grid_base2(
    avg,
    metric="time",
    y_label="Время (с)",
    title="Зависимость времени от размера списка",
    filename="time_vs_size_base2.png"
)

plot_metric_grid_base2(
    avg,
    metric="comparisons",
    y_label="Число сравнений",
    title="Зависимость числа сравнений от размера списка",
    filename="comparisons_vs_size_base2.png"
)

plot_metric_grid_base2(
    avg,
    metric="pointer_changes",
    y_label="Число перестановок / изменений указателей",
    title="Зависимость числа перестановок от размера списка",
    filename="pointer_changes_vs_size_base2.png"
)
