import math

import pandas as pd
import matplotlib.pyplot as plt
import os
import re

# --------------------------
# Enumeration times
# --------------------------

QueryNameTranslate = {
    'Q1': r'$TNQ_4$',
    'Q2': r'$TQ_6$',
    'Q1a': r'$TNQ_{4a}$',
    'Q1b': r'$TNQ_{4b}$',
}

ExecutorTranslate = {
    'CAVIER': 'CaVieR',
    'FIVM': 'F-IVM',
}
# Read the text file
with open('output/output_exp4.txt', 'r') as f:
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
bar_width = 0.35
bar_distance = 0.05
df = df[df['dataset'] != 'tpch_unordered1']
tpch_2 = df[df['name'] == 'tpch_2']



fig, ax = plt.subplots(1, 1, figsize=(6, 6), sharey=True)

combination_legend_handles = []
handled_combinations = set()  # To keep track of handled combinations

x_ticks = []
x_tick_labels = []

start_pos = 0
last_post = 0
length_unique = len(tpch_2['query_root_unique'].unique())
length_non_unique = len(tpch_2['query_root'].unique())
for query_root_unique in sorted(tpch_2['query_root_unique'].unique()):
    query_root_unique_data = tpch_2[tpch_2['query_root_unique'] == query_root_unique]
    for executor in query_root_unique_data['executor'].unique():
        exucutor_data = query_root_unique_data[query_root_unique_data['executor'] == executor]
        for query in exucutor_data['query'].unique():
            query_data = exucutor_data[exucutor_data['query'] == query]

            # Calculate the average and standard deviation for the group
            avg_height = query_data["enumeration_time"].mean() * 1000 / query_data["nr tuples"]
            std_height = query_data["enumeration_time"].std() * 1000 / query_data["nr tuples"]


            # Plot bars with error bars
            bar = ax.bar(start_pos, avg_height, width=bar_width, color=base_colors[executor], alpha=1, yerr=std_height, label=executor)

            if query_root_unique == "Q1 - 07":
                x_ticks.append(start_pos)
                x_tick_labels.append(QueryNameTranslate[query_data.iloc[0]['query']])
            start_pos += (bar_width + bar_distance)
    if query_root_unique == "Q2 - 06":
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
fig.suptitle("Enumeration delay", fontsize=18)
fig.text(0.5, 0.0, "TPC-H scale 10, unordered", ha='center', fontsize=14)

# plt.show()
plt.savefig(os.path.join(output_dir, f'EnumerationTimes.png'), bbox_inches='tight', dpi=300)

print("gugus")
