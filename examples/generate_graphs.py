import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import matplotlib.patches as patches
import matplotlib.colors as mcolors
import re

# Read the text file
with open('output/custom_hash/unordered/cavier/tpch.txt', 'r') as f:
    lines = f.readlines()
with open('output/custom_hash/unordered/fivm/tpch.txt', 'r') as f:
    lines.extend(f.readlines())
root_regex = r'(\D*\d+)'
# Parse the data
data = []
for line in lines:
    name_dataset_processing, *rest = line.strip().split(':')
    name, executor, dataset, _, all_relations = name_dataset_processing.split('|')
    base = [name, dataset, all_relations]
    for res in rest:
        subname, maptype, update_time, enumeration_time, count, varnames, relations = res.split('|')
        splitted_var_names = [x.strip() for x in varnames.lower().split(',')]
        sorted_var_names = ','.join(sorted(splitted_var_names)).replace(' ', '').replace('_', '')
        # extract the query root using the regex r'(\D*\d+)' and strip the whitespaces
        query_root = re.search(root_regex, subname).group(1).strip()

        query_root_unique = f"{query_root} - {len(sorted_var_names.split(',')):02d}"
        data.append(
            base + [executor, subname.strip(), maptype, int(update_time), int(enumeration_time), int(count),
                    sorted_var_names, relations, query_root, query_root_unique])

# Create a DataFrame
df = pd.DataFrame(data)
df.columns = ['name', 'dataset', 'all Relations', 'executor', 'query', 'maptype', 'update_time', 'enumeration time',
              'nr tuples', 'free variables', 'relations', 'query_root', 'query_root_unique']


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
for name in df['name'].unique():
    name_data = df[df['name'] == name]
    fig, axes = plt.subplots(1, 2, figsize=(12, 6), sharey=True)

    combination_legend_handles = []
    handled_combinations = set()  # To keep track of handled combinations

    for idx, metric in enumerate(['update_time', 'enumeration time']):
        ax = axes[idx]
        x_ticks = []
        x_tick_labels = []
        for version_idx, dataset_version in enumerate(name_data['dataset'].unique()):
            dataset_data = name_data[name_data['dataset'] == dataset_version]
            # Start position for the first bar
            start_pos = 0
            last_post = 0
            length_unique = len(dataset_data['query_root_unique'].unique())
            length_non_unique = len(dataset_data['query_root'].unique())
            for query_root_unique in sorted(dataset_data['query_root_unique'].unique()):
                query_root_unique_data = dataset_data[dataset_data['query_root_unique'] == query_root_unique]
                for query in query_root_unique_data['query'].unique():
                    query_data = query_root_unique_data[query_root_unique_data['query'] == query]
                    for executor in query_data['executor'].unique():
                        exucutor_data = query_data[query_data['executor'] == executor]
                        # Calculate the average and standard deviation for the group
                        avg_height = exucutor_data[metric].mean() / 1000
                        std_height = exucutor_data[metric].std() / 1000

                        base_color = mcolors.hex2color(base_colors[executor])
                        base_hsv = mcolors.rgb_to_hsv(base_color)
                        shade = base_hsv.copy()
                        shade[2] = max(0.1, shade[2] - version_idx * 0.4)
                        color = mcolors.hsv_to_rgb(shade)

                        # Plot bars with error bars
                        bar = ax.bar(start_pos, avg_height, width=bar_width, color=color, alpha=1, yerr=std_height)
                        if version_idx == 0:
                            query_name = query if length_unique == length_non_unique else f"{query} - {len(exucutor_data.iloc[0]['free variables'].split(','))}"
                            ax.text(start_pos, (avg_height + std_height) * 1.01, query_name, ha='center', va='bottom',
                                    rotation=90, fontsize=8)

                        start_pos += (bar_width + bar_distance)
                        combination = f'{executor} - {dataset_version}'
                        if combination not in handled_combinations:
                            combination_patch = patches.Patch(color=color, label=combination)
                            combination_legend_handles.append(combination_patch)
                            handled_combinations.add(combination)

                x_ticks.append((start_pos + last_post) / 2 - bar_width / 2 - bar_distance / 2)
                x_tick_labels.append(query_root_unique_data.iloc[0]['query_root'])
                start_pos += (bar_width + bar_distance)
                last_post = start_pos

        ax.set_xlabel('Query')
        ax.set_ylabel(f'{metric.replace("_", " ").capitalize()} (s)')
        ax.set_xticks(x_ticks)
        ax.set_xticklabels(x_tick_labels, rotation=90)
        if idx == 1:
            ax.legend(handles=combination_legend_handles, loc='upper right', bbox_to_anchor=(1, 1), title="Executor - Version")

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    fig.suptitle(name)
    # plt.show()
    plt.savefig(os.path.join(output_dir, f'{name}_plot.png'), bbox_inches='tight', dpi=300)

    print(f"gugus: {name}")
