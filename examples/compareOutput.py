import pandas as pd

# Read the CSV files
df1 = pd.read_csv('output/R3Q1.csv', sep='|', header=None)
df2 = pd.read_csv('output/R3Q1c.csv', sep='|', header=None)

# Set column names
df1.columns = ["locn", "dateid", "ksn", "category", "rain", "zip"]
df2.columns = ["locn", "dateid", "ksn", "category", "rain", "zip"]

# Merge the DataFrames
merged = pd.merge(df1, df2, on=["locn", "dateid", "ksn", "category", "rain", "zip"], how="outer", indicator=True)

# Identify differences
diff_frame = merged[merged["_merge"] != "both"]
# Set the maximum number of columns to display
pd.set_option('display.max_columns', None)

# Set the maximum width of each column
pd.set_option('display.max_colwidth', None)

if diff_frame.empty:
    print("The two CSV files are the same.")
else:
    print(f"The two CSV files are different in {len(diff_frame)} rows.")
    print(diff_frame.head(100))
