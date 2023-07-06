import math

import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import matplotlib.patches as patches
import matplotlib.colors as mcolors
import re

# --------------------------
# Nr Free Variables
# --------------------------

QueryNameTranslate = {
    'Q1': r"$RNQ_1'$",
    'Q2': r"$RQ_2'$",
}
# Read the text file
with open('output/output_exp3.txt', 'r') as f:
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

retailer_6 = df[df['name'] == 'retailer_6']
retailer_7 = df[df['name'] == 'retailer_7']

fig, axes = plt.subplots(1, 2, figsize=(12, 6), sharey=True)

combination_legend_handles = []
handled_combinations = set()  # To keep track of handled combinations

ax = axes[0]
x_ticks = []
x_tick_labels = []

# Start position for the first bar
start_pos = 0
last_post = 0
length_unique = len(retailer_7['query_root_unique'].unique())
length_non_unique = len(retailer_7['query_root'].unique())
for query_root_unique in sorted(retailer_7['query_root_unique'].unique()):
    query_root_unique_data = retailer_7[retailer_7['query_root_unique'] == query_root_unique]
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
        bar = ax.bar(start_pos, best_avg, width=bar_width, color=base_colors[executor], alpha=1, yerr=std, label=executor)

        start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(len(query_root_unique_data.iloc[0]['free variables'].split(',')))
    start_pos += (bar_width + bar_distance)
    last_post = start_pos

ax.set_xlabel(r'# of free variables in $RNQ_1$', fontsize=14)
ax.set_ylabel(f'Update time (s)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=90)


ax = axes[1]
x_ticks = []
x_tick_labels = []
for query_root_unique in sorted(retailer_6['query_root_unique'].unique()):
    query_root_unique_data = retailer_6[retailer_6['query_root_unique'] == query_root_unique]
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
        bar = ax.bar(start_pos, best_avg, width=bar_width, color=base_colors[executor], alpha=1, yerr=std, label=executor)

        start_pos += (bar_width + bar_distance)

    x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
    x_tick_labels.append(len(query_root_unique_data.iloc[0]['free variables'].split(',')))
    start_pos += (bar_width + bar_distance)
    last_post = start_pos
ax.set_xlabel(r'# of free variables in $RQ_2$', fontsize=14)
ax.set_ylabel(f'Update time (s)', fontsize=14)
ax.set_xticks(x_ticks)
ax.set_xticklabels(x_tick_labels, rotation=90)


ax.legend(handles=combination_legend_handles, loc='upper right', bbox_to_anchor=(1, 1), title="Executor - Version")

plt.tight_layout(rect=[0, 0, 1, 0.95])
fig.suptitle("Number Free Variables", fontsize=18)
# plt.show()
plt.savefig(os.path.join(output_dir, f'NrFreeVariables.png'), bbox_inches='tight', dpi=300)

print("gugus")
