import pandas as pd

# Read the text file
with open('output/config.txt', 'r') as f:
    lines = f.readlines()

# Parse the data
data = []
for line in lines:
    name_dataset_processing, *rest = line.strip().split(':')
    name, executor, dataset, stream_processing, relations = name_dataset_processing.split('|')
    if len(rest) > 1:
        data.append([name, dataset, int(stream_processing),"","", "", "","", relations])
        # Parse the repeating patterns
        repeating_patterns = []
        for query in rest:
            subname, time1, time2, count, varnames = query.split('|')

            data.append(["", "", "",executor, subname, int(time1), int(time2) if time2 != '-' else -1, int(count), varnames])
    else:
        subname, time1, time2, count, varnames = rest[0].split('|')
        data.append([name, dataset, int(stream_processing),executor, subname, int(time1), int(time2) if time2 != '-' else -1, int(count), varnames])
# Create a DataFrame
df = pd.DataFrame(data)

# Write the DataFrame to an Excel file
df.to_excel('output.xlsx', index=False,
            header=['name', 'dataset','stream time','executor', 'query', 'enumeration time', 'propagation time', 'nr tuples',
                    'variables'], engine='openpyxl')
