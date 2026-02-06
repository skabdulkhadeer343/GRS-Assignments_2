# Roll No: MT25086
# File: MT25086_Part_D_Plots.py

import matplotlib.pyplot as plt
import numpy as np

# ==============================================================================
# HARDCODED DATA SECTION
# INSTRUCTIONS: Copy values from 'MT25086_Experiment_Results.csv' into these arrays.
# Example format provided below.
# ==============================================================================

# Data for Plot 1: Throughput vs Message Size (Fix Threads = 1)
msg_sizes_kb = [1, 4, 64, 512] 
tp_a1 = [1.2, 3.5, 8.1, 9.5]  # Replace with A1 Throughput readings
tp_a2 = [1.5, 4.0, 9.2, 11.0] # Replace with A2 Throughput readings
tp_a3 = [0.8, 2.5, 8.0, 15.0] # Replace with A3 Throughput readings

# Data for Plot 2: Latency (Inverse Throughput approx) vs Thread Count (Fix Msg Size = 64KB)
threads = [1, 2, 4, 8]
lat_a1 = [50, 60, 120, 200]   # Replace with A1 Latency readings
lat_a2 = [40, 50, 100, 150]   # Replace with A2 Latency readings
lat_a3 = [45, 55, 110, 180]   # Replace with A3 Latency readings

# Data for Plot 3: Cache Misses (LLC) vs Message Size (Fix Threads = 1)
llc_a1 = [1000, 5000, 20000, 100000] # Replace with A1 LLC Misses
llc_a2 = [800, 4000, 15000, 80000]   # Replace with A2 LLC Misses
llc_a3 = [900, 4500, 18000, 90000]   # Replace with A3 LLC Misses

# Data for Plot 4: CPU Cycles per Byte (Fix Threads = 1)
# Calculate: Cycles / (Throughput_Gbps * 1e9 / 8 * Duration) roughly, or just raw cycles if constant load
cycles_a1 = [100, 80, 50, 40] # Replace with A1 Cycles/Byte
cycles_a2 = [90, 70, 40, 30]  # Replace with A2 Cycles/Byte
cycles_a3 = [150, 100, 60, 20] # Replace with A3 Cycles/Byte

# ==============================================================================
# PLOTTING LOGIC
# ==============================================================================

def plot_graph(x, y1, y2, y3, x_label, y_label, title, filename):
    plt.figure(figsize=(10, 6))
    plt.plot(x, y1, marker='o', label='A1 (Two-Copy)')
    plt.plot(x, y2, marker='s', label='A2 (One-Copy)')
    plt.plot(x, y3, marker='^', label='A3 (Zero-Copy)')
    
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(f"{title}\nSystem: Linux x86_64, i7-Gen12, 16GB RAM") # Update Config!
    plt.legend()
    plt.grid(True)
    plt.savefig(filename)
    plt.close()
    print(f"Generated {filename}")

# 1. Throughput vs Message Size
plot_graph(msg_sizes_kb, tp_a1, tp_a2, tp_a3, 
           "Message Size (KB)", "Throughput (Gbps)", 
           "Throughput vs Message Size", "MT25086_Plot_Throughput.pdf")

# 2. Latency vs Thread Count
plot_graph(threads, lat_a1, lat_a2, lat_a3, 
           "Thread Count", "Latency (us)", 
           "Latency vs Thread Count", "MT25086_Plot_Latency.pdf")

# 3. Cache Misses vs Message Size
plot_graph(msg_sizes_kb, llc_a1, llc_a2, llc_a3, 
           "Message Size (KB)", "LLC Misses", 
           "Cache Misses vs Message Size", "MT25086_Plot_Cache.pdf")

# 4. CPU Cycles per Byte
plot_graph(msg_sizes_kb, cycles_a1, cycles_a2, cycles_a3, 
           "Message Size (KB)", "Cycles per Byte", 
           "CPU Efficiency vs Message Size", "MT25086_Plot_Cycles.pdf")