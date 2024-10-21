#!/bin/bash

# Check if the number of images is provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <number_of_images>"
    exit 1
fi

# Number of images to capture
NUM_IMAGES=$1

# Directory to save images
SAVE_DIR="./images"
# Ensure the directory exists
mkdir -p "$SAVE_DIR"

# Initialize the counter
COUNTER=1

# Function to capture an image
capture_image() {
    local filename="$SAVE_DIR/image_$COUNTER.jpg"
    local start_time=$(date +%s)
    libcamera-still -o "$filename"
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    echo "Captured $filename in $duration seconds"
    COUNTER=$((COUNTER + 1))
}

# Record the start time
START_TIME=$(date +%s)

# Loop to capture the specified number of images
for ((i=1; i<=NUM_IMAGES; i++)); do
    capture_image
    sleep 60 # Wait for 60 seconds
done

# Record the end time
END_TIME=$(date +%s)

# Calculate the total duration in seconds
TOTAL_DURATION=$((END_TIME - START_TIME))

# Print the total duration in seconds
echo "Time taken to capture $NUM_IMAGES images: $TOTAL_DURATION seconds"