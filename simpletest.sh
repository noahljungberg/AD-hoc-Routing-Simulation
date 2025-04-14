#!/bin/bash
# Simple script to build and run the NS-3 test

# Navigate to the NS-3 directory
cd /ns-3

# Copy the test file to the scratch directory
cp /project/src/ns3test.cc scratch/

# Build and run the example
./ns3 run scratch/ns3test.cc --verbose

echo "Test completed!"
echo "If you see 'Simulation complete!' above, NS-3 is working correctly."