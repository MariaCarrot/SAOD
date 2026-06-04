# import os
# def convert_snap_to_project_format(
#     input_file,
#     output_file,
#     max_vertices=None,
#     max_edges=None,
#     undirected=True
# ):
#     old_to_new = {}
#     edges = set()

#     def get_new_id(old_id):
#         if old_id not in old_to_new:
#             if max_vertices is not None and len(old_to_new) >= max_vertices:
#                 return None

#             old_to_new[old_id] = len(old_to_new)

#         return old_to_new[old_id]

#     with open(input_file, "r", encoding="utf-8") as file:
#         for line in file:
#             line = line.strip()

#             if not line:
#                 continue

#             if line.startswith("#"):
#                 continue

#             parts = line.split()

#             if len(parts) < 2:
#                 continue

#             old_u = int(parts[0])
#             old_v = int(parts[1])

#             if old_u == old_v:
#                 continue

#             u = get_new_id(old_u)
#             v = get_new_id(old_v)

#             if u is None or v is None:
#                 continue

#             if undirected:
#                 if u > v:
#                     u, v = v, u

#             edge = (u, v)

#             if edge in edges:
#                 continue

#             edges.add(edge)

#             if max_edges is not None and len(edges) >= max_edges:
#                 break

#     n = len(old_to_new)
#     m = len(edges)

#     with open(output_file, "w", encoding="utf-8") as file:
#         file.write(f"{n} {m}\n")

#         for u, v in sorted(edges):
#             file.write(f"{u} {v} 1\n")

#     print("Converted successfully")
#     print("Input file:", input_file)
#     print("Output file:", output_file)
#     print("Vertices:", n)
#     print("Edges:", m)


# if __name__ == "__main__":
#     for i in [[200, 600], [300, 900], [500, 1000], [500, 2500], [600, 2500]]:
#         # convert_snap_to_project_format(
#         #     input_file=r"D:\загрузки яндекс\roadNet-PA.txt",
#         #     output_file=f"roadNet-PA_{i[0]}_{i[1]}.txt",
#         #     max_vertices=i[0],
#         #     max_edges=i[1],
#         #     undirected=True
#         # )

#         # convert_snap_to_project_format(
#         #     input_file=r"D:\загрузки яндекс\roadNet-CA.txt",
#         #     output_file=f"roadNet-CA_{i[0]}_{i[1]}.txt",
#         #     max_vertices=i[0],
#         #     max_edges=i[1],
#         #     undirected=True
#         # )

#         convert_snap_to_project_format(
#             input_file=r"D:\загрузки яндекс\facebook_combined.txt",
#             output_file=f"facebook_{i[0]}_{i[1]}.txt",
#             max_vertices=i[0],
#             max_edges=i[1],
#             undirected=True
#         )

#         convert_snap_to_project_format(
#             input_file=r"D:\загрузки яндекс\twitter_combined.txt",
#             output_file=f"twitter_{i[0]}_{i[1]}.txt",
#             max_vertices=i[0],
#             max_edges=i[1],
#             undirected=True
#         )

import os

def convert_snap_to_project_format(
    input_file,
    output_directory,
    dataset_name,
    max_vertices=None,
    max_edges=None,
    undirected=True
):
    old_to_new = {}
    edges = set()

    def get_new_id(old_id):
        if old_id not in old_to_new:
            if max_vertices is not None and len(old_to_new) >= max_vertices:
                return None

            old_to_new[old_id] = len(old_to_new)

        return old_to_new[old_id]

    with open(input_file, "r", encoding="utf-8") as file:
        for line in file:
            line = line.strip()

            if not line:
                continue

            if line.startswith("#"):
                continue

            parts = line.split()

            if len(parts) < 2:
                continue

            old_u = int(parts[0])
            old_v = int(parts[1])

            if old_u == old_v:
                continue

            u = get_new_id(old_u)
            v = get_new_id(old_v)

            if u is None or v is None:
                continue

            if undirected and u > v:
                u, v = v, u

            edge = (u, v)

            if edge in edges:
                continue

            edges.add(edge)

            if max_edges is not None and len(edges) >= max_edges:
                break

    real_vertices = len(old_to_new)
    real_edges = len(edges)

    output_filename = f"{dataset_name}_{real_vertices}_{real_edges}.txt"
    output_file = os.path.join(output_directory, output_filename)

    with open(output_file, "w", encoding="utf-8") as file:
        file.write(f"{real_vertices} {real_edges}\n")

        for u, v in sorted(edges):
            file.write(f"{u} {v} 1\n")

    print("Converted successfully")
    print("Input file:", input_file)
    print("Output file:", output_file)
    print("Vertices:", real_vertices)
    print("Edges:", real_edges)
    print()


if __name__ == "__main__":
    output_directory = "converted_graphs"
    os.makedirs(output_directory, exist_ok=True)

    datasets = [
        {
            "name": "facebook",
            "path": r"D:\загрузки яндекс\facebook_combined.txt",
            "undirected": True
        },
        {
            "name": "twitter",
            "path": r"D:\загрузки яндекс\twitter_combined.txt",
            "undirected": True
        },
        {
            "name": "roadNet-PA",
            "path": r"D:\загрузки яндекс\roadNet-PA.txt\roadNet-PA.txt",
            "undirected": True
        },
        {
            "name": "roadNet-CA",
            "path": r"D:\загрузки яндекс\roadNet-CA.txt\roadNet-CA.txt",
            "undirected": True
        }
    ]

    graph_limits = [
    [1000, 3000],
    [2000, 6000],
    [3000, 9000],
    [5000, 15000]
]

    for dataset in datasets:
        for max_vertices, max_edges in graph_limits:
            convert_snap_to_project_format(
                input_file=dataset["path"],
                output_directory=output_directory,
                dataset_name=dataset["name"],
                max_vertices=max_vertices,
                max_edges=max_edges,
                undirected=dataset["undirected"]
            )