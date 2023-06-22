
# Process ID
pid=$1

# Interval in seconds
interval=$2

# Ensure that both PID and interval are supplied
if [[ -z "$pid" ]] || [[ -z "$interval" ]]; then
    echo "Usage: ./sample_memory.sh <pid> <interval_in_seconds>"
    exit 1
fi

# Sample memory in intervals
while true; do
    top -l 1 -pid "$pid" | awk '/^PID/{getline; print $8}'
    sleep "$interval"
done