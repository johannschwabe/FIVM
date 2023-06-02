import pandas as pd

# Read the CSV files
df1 = pd.read_csv('output/Q2_hybrid.csv', sep='|')
df2 = pd.read_csv('output/Q2_correct.csv', sep='|')

# Set column names
df1.columns = ["locn", "dateid", "ksn", "rain", "maxtemp", "zip"]
df2.columns = ["locn", "dateid", "ksn", "rain", "maxtemp", "zip"]

# Merge the DataFrames
merged = pd.merge(df1, df2, on=["locn", "dateid", "ksn", "rain", "maxtemp", "zip"], how="outer", indicator=True)

# Identify differences
diff_frame = merged[merged["_merge"] != "both"]

if diff_frame.empty:
    print("The two CSV files are the same.")
else:
    print("The two CSV files are different.")
    print(diff_frame.head(1))
