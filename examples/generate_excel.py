import pandas as pd

# Read the text file
with open('output/output.txt', 'r') as f:
    lines = f.readlines()

# Parse the data
data = []
for line in lines:
    name_dataset_processing, *rest = line.strip().split(':')
    name, executor, dataset, version, all_relations = name_dataset_processing.split('|')
    data.append([name, dataset+version, all_relations, "", "", "", "", ])
    for res in rest:
        subname, maptype, update_time, enumeration_time, count, varnames, relations = res.split('|')

        data.append(
            ["", "", "", executor, subname, maptype, int(update_time), int(enumeration_time), int(count), varnames, relations])

# Create a DataFrame
df = pd.DataFrame(data)

# Write the DataFrame to an Excel file
df.to_excel('output.xlsx', index=False,
            header=['name', 'dataset', 'all Relations', 'executor', 'query','maptype' ,'update time','enumeration time',  'nr tuples','free variables', 'relations'], engine='openpyxl')
