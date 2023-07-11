import math

import pandas as pd
import matplotlib.pyplot as plt
import os
import matplotlib.colors as mcolors
import re

# --------------------------
# Input Cardinalities
# --------------------------

QueryNameTranslate = {
    'Q1': r'$TQ_7$',
    'Q2': r'$TNQ_8$',
    'Q3': r'$TNQ_9$',
    'Q4': r'$TNQ_{10}$',
    'Q5': r'$TNQ_{11}$',
    'Q6': r'$TNQ_{12}$',
    'Q7': r'$TNQ_{13}$',
}
ExecutorTranslate = {
    'CAVIER': 'CaVieR',
    'FIVM': 'F-IVM',
}

# Read the text file
with open('output/output_exp9.txt', 'r') as f:
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
            base + [executor, subname.strip(), maptype, int(update_time), int(enumeration_time), int(count),
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
# Unique executors
executors = df['executor'].unique()
# Width of a bar
bar_width = 0.35
bar_distance = 0.05

df = df[df['query'] != 'Q1c']
df = df[df['dataset'] == 'tpch_unordered10']

tpch_7 = df[df['name'].str.startswith('tpch_7_0')]
tpch_8 = df[df['name'].str.startswith('tpch_8_0')]

fig, axes = plt.subplots(2, 2, figsize=(12, 12))

axes[1,0].get_shared_y_axes().join(axes[0,0], axes[1,0])
axes[1,1].get_shared_y_axes().join(axes[0,1], axes[1,1])

ax = axes[0, 0]
x_ticks = []
x_tick_labels = []

# Start position for the first bar
start_pos = 0
last_post = 0
for query_root_unique in sorted(tpch_7['query_root'].unique()):
    query_root_unique_data = tpch_7[tpch_7['query_root'] == query_root_unique]
    for executor in query_root_unique_data['executor'].unique():
        exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
        best_avg = math.inf
        std = 0
        for query in exucutor_data['query'].unique():
            query_data = exucutor_data[exucutor_data['query'] == query]
            # Calculate the average and standard deviation for the group
            avg_height = exucutor_data["update_time"].mean() / 1000
            std_height = exucutor_data["update_time"].std() / 1000
            if avg_height < best_avg:
                best_avg = avg_height
                std = std_height


        # Plot bars with error bars
        bar = ax.bar(start_pos, best_avg, width=bar_width, color=base_colors[executor], alpha=1, yerr=std)

        start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
    start_pos += (bar_width + bar_distance)
    last_post = start_pos

ax.set_xlabel(r'Queryset $\mathcal{S}_6$', fontsize=14)
ax.set_ylabel(f'Update time (s)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0,fontsize=13)

ax = axes[0, 1]
x_ticks = []
x_tick_labels = []

# Start position for the first bar
start_pos = 0
last_post = 0
for query_root_unique in sorted(tpch_7['query_root'].unique()):
    query_root_unique_data = tpch_7[tpch_7['query_root'] == query_root_unique]
    for executor in query_root_unique_data['executor'].unique():
        exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
        best_avg = math.inf
        std = 0
        for query in exucutor_data['query'].unique():
            query_data = exucutor_data[exucutor_data['query'] == query]
            # Calculate the average and standard deviation for the group
            avg_height = query_data["enumeration time"].mean() * 1000 / query_data["nr tuples"].mean()
            std_height = query_data["enumeration time"].std() * 1000 / query_data["nr tuples"].mean()
            if avg_height < best_avg:
                best_avg = avg_height
                std = std_height


        # Plot bars with error bars
        bar = ax.bar(start_pos, best_avg, width=bar_width, color=base_colors[executor], alpha=1, yerr=std)

        start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
    start_pos += (bar_width + bar_distance)
    last_post = start_pos

ax.set_xlabel(r'Queryset $\mathcal{S}_6$', fontsize=14)
ax.set_ylabel(r'Enumeration delay ($\mu s$)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0,fontsize=13)

QueryNameTranslate = {
    'Q1': r'$TQ_{14}$',
    'Q2': r'$TQ_{15}$',
    'Q3': r'$TNQ_{16}$',
    'Q4': r'$TQ_6$',
    'Q5': r'$TNQ_{17}$',
    'Q6': r'$TNQ_{18}$',
    'Q7': r'$TNQ_{19}$',
}

ax = axes[1,0]
x_ticks = []
x_tick_labels = []
for version_idx, dataset_version in enumerate(tpch_8['dataset'].unique()):
    dataset_data = tpch_8[tpch_8['dataset'] == dataset_version]
    # Start position for the first bar
    start_pos = 0
    last_post = 0
    for query_root_unique in sorted(dataset_data['query_root'].unique()):
        query_root_unique_data = dataset_data[dataset_data['query_root'] == query_root_unique]
        for executor in query_root_unique_data['executor'].unique():
            exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
            best_avg = math.inf
            std = 0
            for query in exucutor_data['query'].unique():
                query_data = exucutor_data[exucutor_data['query'] == query]
                # Calculate the average and standard deviation for the group
                avg_height = exucutor_data["update_time"].mean() / 1000
                std_height = exucutor_data["update_time"].std() / 1000
                if avg_height < best_avg:
                    best_avg = avg_height
                    std = std_height

            base_color = mcolors.hex2color(base_colors[executor])
            base_hsv = mcolors.rgb_to_hsv(base_color)
            shade = base_hsv.copy()
            shade[2] = max(0.1, shade[2] - version_idx * 0.4)
            color = mcolors.hsv_to_rgb(shade)

            # Plot bars with error bars
            bar = ax.bar(start_pos, best_avg, width=bar_width, color=color, alpha=1, yerr=std, label=ExecutorTranslate[executor])

            start_pos += (bar_width + bar_distance)

        x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
        x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
        start_pos += (bar_width + bar_distance)
        last_post = start_pos

ax.set_xlabel(r'Queryset $\mathcal{S}_7$', fontsize=14)
ax.set_ylabel(f'Update time (s)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0, fontsize=13)

ax = axes[1,1]
x_ticks = []
x_tick_labels = []
for version_idx, dataset_version in enumerate(tpch_8['dataset'].unique()):
    dataset_data = tpch_8[tpch_8['dataset'] == dataset_version]
    # Start position for the first bar
    start_pos = 0
    last_post = 0
    for query_root_unique in sorted(dataset_data['query_root'].unique()):
        query_root_unique_data = dataset_data[dataset_data['query_root'] == query_root_unique]
        for executor in query_root_unique_data['executor'].unique():
            exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
            best_avg = math.inf
            std = 0
            for query in exucutor_data['query'].unique():
                query_data = exucutor_data[exucutor_data['query'] == query]
                # Calculate the average and standard deviation for the group
                avg_height = query_data["enumeration time"].mean() * 1000 / query_data["nr tuples"].mean()
                std_height = query_data["enumeration time"].std() * 1000 / query_data["nr tuples"].mean()
                if avg_height < best_avg:
                    best_avg = avg_height
                    std = std_height

            base_color = mcolors.hex2color(base_colors[executor])
            base_hsv = mcolors.rgb_to_hsv(base_color)
            shade = base_hsv.copy()
            shade[2] = max(0.1, shade[2] - version_idx * 0.4)
            color = mcolors.hsv_to_rgb(shade)

            # Plot bars with error bars
            bar = ax.bar(start_pos, best_avg, width=bar_width, color=color, alpha=1, yerr=std, label=ExecutorTranslate[executor])

            start_pos += (bar_width + bar_distance)

        x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
        x_tick_labels.append(QueryNameTranslate[query_root_unique_data.iloc[0]['query_root']])
        start_pos += (bar_width + bar_distance)
        last_post = start_pos

ax.set_xlabel(r'Queryset $\mathcal{S}_7$', fontsize=14)
ax.set_ylabel(r'Enumeration delay ($\mu s$)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=0, fontsize=13)

# Get handles and labels
handles, labels = ax.get_legend_handles_labels()

# Remove duplicates
unique = dict(zip(labels, handles))
ax.legend(unique.values(), unique.keys())

plt.tight_layout(rect=[0, 0, 1, 0.95])
fig.suptitle("Many Queries", fontsize=18)
fig.text(0.5, 0.01, "TPC-H scale 10 unordered", ha='center', fontsize=14)

# plt.show()
plt.savefig(os.path.join(output_dir, f'LargeExample.png'), bbox_inches='tight', dpi=300)

print("gugus")
