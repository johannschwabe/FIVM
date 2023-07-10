import math

import pandas as pd
import matplotlib.pyplot as plt
import os
import matplotlib.patches as mpatches
import re

# --------------------------
# tpch vs jcch
# --------------------------
QueryNameTranslate = {
    'Q1': r'$TNQ_4$',
    'Q2': r'$TQ_5$',
}
ExecutorTranslate = {
    'CAVIER': 'CaVieR',
    'FIVM': 'F-IVM',
}

textures = {
    'BTre': '/',
    'Hash': 'O',
    'Cust': '.',
    'map': '-'
}
plt.rcParams['hatch.linewidth'] = 1.5
# Read the text file
with open('output/output_exp7.txt', 'r') as f:
    lines = f.readlines()

root_regex = r'(\D*\d+)'
# Parse the data
data = []
for line in lines:
    name_dataset_processing, *rest = line.strip().split(':')
    name, executor, dataset, _, all_relations = name_dataset_processing.split('|')
    base = [name, dataset, all_relations]
    for res in rest:
        splitted = res.split('|')
        if len(splitted) == 7:
            subname, maptype, update_time, enumeration_time, count, varnames, relations = splitted
            batch_size = 1000
        else:
            subname, maptype, update_time, enumeration_time, count, varnames, relations, batch_size = splitted

        splitted_var_names = [x.strip() for x in varnames.lower().split(',')]
        sorted_var_names = ','.join(sorted(splitted_var_names)).replace(' ', '').replace('_', '')
        # extract the query root using the regex r'(\D*\d+)' and strip the whitespaces
        query_root = re.search(root_regex, subname).group(1).strip()

        query_root_unique = f"{query_root} - {len(sorted_var_names.split(',')):02d}"
        data.append(
            base + [executor, subname.strip(), maptype.strip(), int(update_time), int(enumeration_time), int(count),
                    sorted_var_names, relations, query_root, query_root_unique, int(batch_size)])

# Create a DataFrame
df = pd.DataFrame(data)
df.columns = ['name', 'dataset', 'all Relations', 'executor', 'query', 'maptype', 'update_time', 'enumeration time',
              'nr tuples', 'free variables', 'relations', 'query_root', 'query_root_unique', 'batch_size']


# Directory to save plots
output_dir = 'viz'
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# Colors for executors

base_colors = {'CAVIER': 'blue', "FIVM": 'orange'}
# Width of a bar
bar_width = 0.35
bar_distance = 0.05

df = df[df['query'] != 'Q1c']


tpch_1_unordered = df[(df['name'] == 'tpch_1') & ((df['dataset'] == 'tpch_unordered10'))]
retailer_unordered = df[(df['name'] == 'retailer_3') & ((df['dataset'] == 'retailer_unordered'))]


fig, axes = plt.subplots(2, 2, figsize=(12, 12))

axes[1,0].get_shared_y_axes().join(axes[0,0], axes[1,0])
axes[1,1].get_shared_y_axes().join(axes[0,1], axes[1,1])

combination_legend_handles = []
handled_combinations = set()  # To keep track of handled combinations

ax = axes[0][0]
start_pos = 0
last_post = 0
x_ticks = []
x_tick_labels = []
length_unique = len(tpch_1_unordered['query_root_unique'].unique())
length_non_unique = len(tpch_1_unordered['query_root'].unique())
for query_root_unique in sorted(tpch_1_unordered['query_root_unique'].unique()):
    query_root_unique_data = tpch_1_unordered[tpch_1_unordered['query_root_unique'] == query_root_unique]
    for executor in query_root_unique_data['executor'].unique():
        exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
        for maptype in exucutor_data['maptype'].unique():
            maptype_data = exucutor_data[exucutor_data['maptype'] == maptype]
            best_avg = math.inf
            std = 0
            for query in maptype_data['query'].unique():
                query_data = maptype_data[maptype_data['query'] == query]
                # Calculate the average and standard deviation for the group
                avg_height = query_data["update_time"].mean() / 1000
                std_height = query_data["update_time"].std() / 1000
                if avg_height < best_avg:
                    best_avg = avg_height
                    std = std_height


            # Plot bars with error bars
            bar = ax.bar(start_pos, best_avg,
                         width=bar_width,
                         color=base_colors[executor],
                         alpha=1,
                         yerr=std,
                         label=executor,
                         hatch=textures[maptype[0:4]],
                         )
            start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
    start_pos += (bar_width + bar_distance)
    last_post = start_pos


ax.set_xlabel(r'TPC-H', fontsize=14)
ax.set_ylabel(f'Update time (s)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0,fontsize=13)

ax = axes[0][1]
start_pos = 0
last_post = 0
x_ticks = []
x_tick_labels = []
length_unique = len(tpch_1_unordered['query_root_unique'].unique())
length_non_unique = len(tpch_1_unordered['query_root'].unique())
for query_root_unique in sorted(tpch_1_unordered['query_root_unique'].unique()):
    query_root_unique_data = tpch_1_unordered[tpch_1_unordered['query_root_unique'] == query_root_unique]
    for executor in query_root_unique_data['executor'].unique():
        exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
        for maptype in exucutor_data['maptype'].unique():
            maptype_data = exucutor_data[exucutor_data['maptype'] == maptype]
            best_avg = math.inf
            std = 0
            for query in maptype_data['query'].unique():
                query_data = maptype_data[maptype_data['query'] == query]
                # Calculate the average and standard deviation for the group
                avg_height = query_data["enumeration time"].mean() * 1000 / query_data["nr tuples"].mean()
                std_height = query_data["enumeration time"].std() * 1000 / query_data["nr tuples"].mean()
                if avg_height < best_avg:
                    best_avg = avg_height
                    std = std_height


            # Plot bars with error bars
            bar = ax.bar(start_pos, best_avg,
                         width=bar_width,
                         color=base_colors[executor],
                         alpha=1,
                         yerr=std,
                         label=executor,
                         hatch=textures[maptype[0:4]],
                         )

            start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
    start_pos += (bar_width + bar_distance)
    last_post = start_pos


ax.set_xlabel(r'TPC-H', fontsize=14)
ax.set_ylabel(r'Enumeration delay ($\mu s$)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0,fontsize=13)

QueryNameTranslate = {
    'Q1': r'$RNQ_1$',
    'Q2': r'$RQ_3$',
}
ax = axes[1][0]
x_ticks = []
x_tick_labels = []
start_pos = 0
last_post = 0
length_unique = len(retailer_unordered['query_root_unique'].unique())
length_non_unique = len(retailer_unordered['query_root'].unique())
for query_root_unique in sorted(retailer_unordered['query_root_unique'].unique()):
    query_root_unique_data = retailer_unordered[retailer_unordered['query_root_unique'] == query_root_unique]
    for executor in query_root_unique_data['executor'].unique():
        exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
        for maptype in exucutor_data['maptype'].unique():
            maptype_data = exucutor_data[exucutor_data['maptype'] == maptype]
            best_avg = math.inf
            std = 0
            for query in maptype_data['query'].unique():
                query_data = maptype_data[maptype_data['query'] == query]
                # Calculate the average and standard deviation for the group
                avg_height = query_data["update_time"].mean() / 1000
                std_height = query_data["update_time"].std() / 1000
                if avg_height < best_avg:
                    best_avg = avg_height
                    std = std_height


            # Plot bars with error bars
            bar = ax.bar(start_pos, best_avg,
                         width=bar_width,
                         color=base_colors[executor],
                         alpha=1,
                         yerr=std,
                         label=executor,
                         hatch=textures[maptype[0:4]],
                         )
            start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
    start_pos += (bar_width + bar_distance)
    last_post = start_pos


ax.set_xlabel(r'Retailer', fontsize=14)
ax.set_ylabel(f'Update time (s)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0,fontsize=13)

ax = axes[1][1]
start_pos = 0
last_post = 0
x_ticks = []
x_tick_labels = []
length_unique = len(retailer_unordered['query_root_unique'].unique())
length_non_unique = len(retailer_unordered['query_root'].unique())
for query_root_unique in sorted(retailer_unordered['query_root_unique'].unique()):
    query_root_unique_data = retailer_unordered[retailer_unordered['query_root_unique'] == query_root_unique]
    for executor in query_root_unique_data['executor'].unique():
        exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
        for maptype in exucutor_data['maptype'].unique():
            maptype_data = exucutor_data[exucutor_data['maptype'] == maptype]
            best_avg = math.inf
            std = 0
            for query in maptype_data['query'].unique():
                query_data = maptype_data[maptype_data['query'] == query]
                # Calculate the average and standard deviation for the group
                avg_height = query_data["enumeration time"].mean() * 1000 / query_data["nr tuples"].mean()
                std_height = query_data["enumeration time"].std() * 1000 / query_data["nr tuples"].mean()
                if avg_height < best_avg:
                    best_avg = avg_height
                    std = std_height


            # Plot bars with error bars
            bar = ax.bar(start_pos, best_avg,
                         width=bar_width,
                         color=base_colors[executor],
                         alpha=1,
                         yerr=std,
                         label=executor,
                         hatch=textures[maptype[0:4]],
                         )

            start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
    start_pos += (bar_width + bar_distance)
    last_post = start_pos


ax.set_xlabel(r'Retailer', fontsize=14)
ax.set_ylabel(r'Enumeration delay ($\mu s$)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0,fontsize=13)


legend_executors = [('CaVieR', 'blue'), ('F-IVM', 'orange')]
legend_hatches = [('B+ Tree', '/'), ('Hash Table', 'O'), ('Custom Hash Table', '.'), ('Binary Tree', '-')]

color_patches = [mpatches.Patch(color=c[1], label=c[0]) for c in legend_executors]
hatch_patches = [mpatches.Patch(facecolor='gray', hatch=h[1], label=h[0]) for h in legend_hatches]
plt.legend(handles=color_patches + hatch_patches, handleheight=2.5, handlelength=4)

plt.tight_layout(rect=[0, 0, 1, 0.95])
fig.suptitle("Map Types - Update time", fontsize=18)
fig.text(0.5, 0.01, "TPC-H & Retailer, unordered", ha='center', fontsize=14)

# plt.show()
plt.savefig(os.path.join(output_dir, f'MapTypes.png'), bbox_inches='tight', dpi=300)

print("gugus")
