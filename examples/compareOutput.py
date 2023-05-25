import pandas as pd

# Read the CSV files
df1 = pd.read_csv('Q1a.csv',
                  names=["orderkey", "partkey", "suppkey", "l_quantity", "ps_availqty", "p_name", "o_totalprice",
                         "combi"], header=None, sep='|')
df2 = pd.read_csv('Q1b.csv',
                  names=["partkey", "suppkey", "orderkey", "l_quantity", "o_totalprice", "ps_availqty", "p_name",
                         "combi"], header=None, sep='|')

# Sort the DataFrames by column names
df1 = df1.sort_index(axis=1)
df2 = df2.sort_index(axis=1)

# Sort the DataFrames by column values
df1 = df1.sort_values(by=list(df1.columns)).reset_index(drop=True)
df2 = df2.sort_values(by=list(df2.columns)).reset_index(drop=True)

# Compare the two dataframes
comparison = df1.equals(df2)

print(f"The two csv files are {'the same.' if comparison else 'different.'}")
if not comparison:
    diff_frame = df1.compare(df2)
    print(diff_frame.head(1))
