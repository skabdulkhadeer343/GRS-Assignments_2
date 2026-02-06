#!/bin/bash
# Roll No: MT25086
# File: MT25086_Part_C_Runner.sh

# Configuration
DURATION=5
SERVER_IP="127.0.0.1"
SIZES=(1024 4096 65536 524288) # 1KB, 4KB, 64KB, 512KB
THREADS=(1 2 4 8)
OUTPUT_FILE="MT25086_Experiment_Results.csv"

# Compile everything
make clean
make all

# Initialize CSV
echo "Implementation,MsgSize,Threads,Throughput_Gbps,Cycles,L1_Misses,LLC_Misses,CS" > $OUTPUT_FILE

run_experiment() {
    IMPL_NAME=$1
    SERVER_BIN=$2
    CLIENT_BIN=$3
    
    echo "Starting experiments for $IMPL_NAME..."

    for SZ in "${SIZES[@]}"; do
        for TH in "${THREADS[@]}"; do
            echo "Running: $IMPL_NAME | Size: $SZ | Threads: $TH"
            
            # Start Server in background
            ./$SERVER_BIN $SZ > /dev/null 2>&1 &
            SERVER_PID=$!
            sleep 1 # Wait for server start

            # Run Client wrapped in perf
            # Note: We profile the CLIENT and SYSTEM behavior mostly, but profiling 
            # the server is often more interesting for copy costs. 
            # The assignment asks to profile "Implementations". We will profile the Client process tree.
            
            # However, for accurate CPU/Cache on the data movement, profiling the Server is better 
            # as it does the sending/copying. Let's profile the SERVER PID.
            
            # Start Client in background to drive load
            ./$CLIENT_BIN $SERVER_IP $SZ $TH $DURATION > client_output.tmp &
            CLIENT_PID=$!

            # Profile the Server for duration-1 seconds
            perf stat -e cycles,l1-dcache-misses,LLC-misses,cs \
                -p $SERVER_PID sleep $(($DURATION - 1)) > perf_output.tmp 2>&1
            
            wait $CLIENT_PID
            
            # Parse Throughput from Client Output
            TP=$(grep "TOTAL_THROUGHPUT" client_output.tmp | awk '{print $2}')
            
            # Parse Perf Output
            CYCLES=$(grep "cycles" perf_output.tmp | awk '{print $1}' | tr -d ',')
            L1=$(grep "l1-dcache-misses" perf_output.tmp | awk '{print $1}' | tr -d ',')
            LLC=$(grep "LLC-misses" perf_output.tmp | awk '{print $1}' | tr -d ',')
            CS=$(grep "cs" perf_output.tmp | awk '{print $1}' | tr -d ',')

            # Handle empty perf data (permissions issue fallback)
            CYCLES=${CYCLES:-0}
            L1=${L1:-0}
            LLC=${LLC:-0}
            CS=${CS:-0}
            TP=${TP:-0}

            echo "$IMPL_NAME,$SZ,$TH,$TP,$CYCLES,$L1,$LLC,$CS" >> $OUTPUT_FILE
            
            # Kill Server
            kill -9 $SERVER_PID 2>/dev/null
            wait $SERVER_PID 2>/dev/null
            sleep 1
        done
    done
}

# Run A1
run_experiment "A1_TwoCopy" "MT25086_Part_A1_Server" "MT25086_Part_A1_Client"

# Run A2
run_experiment "A2_OneCopy" "MT25086_Part_A2_Server" "MT25086_Part_A1_Client"

# Run A3
run_experiment "A3_ZeroCopy" "MT25086_Part_A3_Server" "MT25086_Part_A1_Client"

echo "Experiments Completed. Results in $OUTPUT_FILE"
rm *.tmp