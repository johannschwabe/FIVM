import math

import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import matplotlib.patches as patches
import matplotlib.colors as mcolors
import re

# --------------------------
# Batch Sizes
# --------------------------
# Read the text file
with open('output/output_batch_sizes.txt', 'r') as f:
    lines = f.readlines()

root_regex = r'(\D*\d+)'
# Parse the data
data = []
for line in lines:
    name_dataset_processing, *rest = line.strip().split(':')
    name, executor, dataset, _, all_relations = name_dataset_processing.split('|')
    base = [name, dataset, all_relations]
    for res in rest:
        subname, maptype, update_time, enumeration_time, count, varnames, relations, batch_size = res.split('|')
        splitted_var_names = [x.strip() for x in varnames.lower().split(',')]
        sorted_var_names = ','.join(sorted(splitted_var_names)).replace(' ', '').replace('_', '')
        # extract the query root using the regex r'(\D*\d+)' and strip the whitespaces
        query_root = re.search(root_regex, subname).group(1).strip()

        query_root_unique = f"{query_root} - {len(sorted_var_names.split(',')):02d}"
        data.append(
            base + [executor, subname.strip(), maptype, int(update_time), int(enumeration_time), int(count),
                    sorted_var_names, relations, query_root, query_root_unique, batch_size])

# Create a DataFrame
df = pd.DataFrame(data)
df.columns = ['name', 'dataset', 'all Relations', 'executor', 'query', 'maptype', 'update_time', 'enumeration_time',
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
bar_width = 0.45
bar_distance = 0.1

df = df[df['query'] != 'Q1c']

fig, ax = plt.subplots(1, 1, figsize=(6, 6), sharey=True)

combination_legend_handles = []
handled_combinations = set()  # To keep track of handled combinations

x_ticks = []
x_tick_labels = []

# Start position for the first bar
start_pos = 0
last_post = 0
length_unique = len(df['query_root_unique'].unique())
length_non_unique = len(df['query_root'].unique())
for query_root_unique in sorted(df['query_root_unique'].unique()):
    query_root_unique_data = df[df['query_root_unique'] == query_root_unique]
    for query in query_root_unique_data['query'].unique():
        query_data = query_root_unique_data[query_root_unique_data['query'] == query]
        for batch_size in query_data['batch_size'].unique():
            dataset_data = query_data[query_data['batch_size'] == batch_size]
            for executor in dataset_data['executor'].unique():
                exucutor_data = dataset_data[dataset_data['executor'] == executor]

                # Calculate the average and standard deviation for the group
                avg_height = exucutor_data["update_time"].mean() / 1000
                std_height = exucutor_data["update_time"].std() / 1000

                base_color = mcolors.hex2color(base_colors[executor])

                # Plot bars with error bars
                bar = ax.bar(start_pos, avg_height, width=bar_width, color=base_color, alpha=1, yerr=std_height)
                query_name = round(math.log(int(exucutor_data.iloc[0]['batch_size']),10))
                ax.text(start_pos, (avg_height + std_height) * 1.01 + 0.5, query_name, ha='center', va='bottom',
                        rotation=0, fontsize=12)

                start_pos += (bar_width + bar_distance)
                combination = f'{executor}'
                if combination not in handled_combinations:
                    combination_patch = patches.Patch(color=base_color, label=combination)
                    combination_legend_handles.append(combination_patch)
                    handled_combinations.add(combination)

        x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
        x_tick_labels.append(query_data.iloc[0]['query'])
        start_pos += (bar_width + bar_distance)
        last_post = start_pos

ax.set_xlabel(r'Queryset $\mathcal{Q}_1$', fontsize=14)
ax.set_ylabel(f'Update time (s)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=90)
#ax.set_yscale("log")

ax.legend(handles=combination_legend_handles, loc='upper right', bbox_to_anchor=(1, 1), title="Executor")

plt.tight_layout(rect=[0, 0, 1, 0.95])
fig.suptitle("Batch Sizes", fontsize=18)
# plt.show()
plt.savefig(os.path.join(output_dir, f'BatchSizes.png'), bbox_inches='tight', dpi=300)

print("gugus")
