# Weather Simulation with OpenMP

This project implements a simplified weather simulation in C based on measurement data from multiple weather stations. The focus lies on computing local temperature changes and incorporating the influence of neighboring stations.

## Core Idea

For each weather station and each timestamp, the simulation performs two main steps:

### 1. Local Temperature Change

For every station, the temperature change is computed as the difference between consecutive measurements:

temp_change[t] = temp[t] - temp[t-1]


The first value is initialized as `0` since no previous measurement exists.

This step captures how the temperature evolves locally over time.

---

### 2. Neighbor-Based Averaging

Each station is connected to up to four neighbors (north, south, east, west).  
To simulate spatial influence, the local temperature change is averaged with the changes of its neighbors:


final[t] = average(local[t] + neighbor_local[t])


Only existing neighbors are considered. This results in a smoothed temperature change that reflects both local dynamics and surrounding conditions.

---

## Parallelization (OpenMP)

The computation is parallelized using OpenMP.  
Each station is processed independently, allowing multiple stations to be computed in parallel without race conditions, since each thread writes to its own memory region.

---

## Input and Output

- **Input:** Preprocessed CSV files (one per station)
- **Output:** One CSV file per station containing:
  - Timestamp (year, month, day, hour, minute)
  - Final (smoothed) temperature change

---

## Data Preprocessing (`cleaner.py`)

The Python script `cleaner.py` is used to preprocess raw weather data.  
It extracts relevant columns (timestamp and temperature) and converts them into a simplified format that can be efficiently read by the C program.
