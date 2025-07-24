#!/usr/bin/env python3
"""
VL53L8CH Data Parser and Visualizer

This script parses VL53L8CH hex data from PlatformIO logs and generates
PNG visualizations of the 8x8 sensor array data.
"""

import re
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import argparse

def parse_hex_data(hex_string):
    """
    Parse hex string into 8x8 array of sensor values.
    
    VL53L8CH produces 64 values (8x8 grid) where each value is 1 byte.
    The hex string should be 128 characters (64 bytes * 2 hex chars per byte).
    """
    if len(hex_string) != 128:
        raise ValueError(f"Expected 128 hex characters, got {len(hex_string)}")
    
    # Convert hex string to bytes
    bytes_data = bytes.fromhex(hex_string)
    
    # Convert to numpy array and reshape to 8x8
    array_8x8 = np.frombuffer(bytes_data, dtype=np.uint8).reshape(8, 8)
    
    return array_8x8

def extract_hex_data_from_log(log_file_path):
    """
    Extract all VL53L8CH hex data and target status from a log file.
    
    Returns tuple of (distance_arrays, target_status_arrays, valid_measurements)
    """
    hex_data_pattern = r'TOF: HEX DATA:\s+([0-9A-Fa-f]{128})'
    target_status_pattern = r'TOF: TARGET STATUS: ([0-9A-Fa-f]{128})'
    
    distance_arrays = []
    target_status_arrays = []
    valid_measurements = []
    
    with open(log_file_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    # Process lines in pairs (distance data followed by target status)
    for i in range(len(lines) - 1):
        distance_match = re.search(hex_data_pattern, lines[i])
        if distance_match:
            # Look for target status in the next line
            target_match = re.search(target_status_pattern, lines[i + 1])
            
            if target_match:
                try:
                    # Parse distance data
                    distance_hex = distance_match.group(1)
                    distance_array = parse_hex_data(distance_hex)
                    
                    # Parse target status
                    target_hex = target_match.group(1)
                    target_array = parse_hex_data(target_hex)
                    
                    # Count valid measurements (target status 5 or 9 = valid target)
                    valid_count = np.sum((target_array == 5) | (target_array == 9))
                    
                    distance_arrays.append(distance_array)
                    target_status_arrays.append(target_array)
                    valid_measurements.append(valid_count)
                    
                except ValueError as e:
                    print(f"Warning: Skipping invalid hex data: {e}")
                    continue
            else:
                # No target status found, process distance only (legacy format)
                try:
                    distance_hex = distance_match.group(1)
                    distance_array = parse_hex_data(distance_hex)
                    distance_arrays.append(distance_array)
                    target_status_arrays.append(np.zeros((8, 8), dtype=np.uint8))  # Unknown validity
                    valid_measurements.append(0)  # Unknown validity
                except ValueError as e:
                    print(f"Warning: Skipping invalid hex data: {e}")
                    continue
    
    return distance_arrays, target_status_arrays, valid_measurements

def create_heatmap(data, title, output_path, cmap='viridis'):
    """
    Create a heatmap PNG image from 8x8 data array.
    """
    plt.figure(figsize=(10, 8))
    im = plt.imshow(data, cmap=cmap, interpolation='nearest')
    plt.colorbar(im, label='Value')
    plt.title(title)
    plt.xlabel('X Position')
    plt.ylabel('Y Position')
    
    # Add text annotations for each cell
    for i in range(8):
        for j in range(8):
            text = plt.text(j, i, f'{data[i, j]:.1f}', 
                          ha="center", va="center", color="white", fontsize=8)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Saved heatmap: {output_path}")

def analyze_sensor_data(log_file_path):
    """
    Main analysis function that processes log file and generates visualizations.
    """
    print(f"Processing log file: {log_file_path}")
    
    # Extract hex data
    distance_arrays, target_status_arrays, valid_measurements = extract_hex_data_from_log(log_file_path)
    
    if not distance_arrays:
        print("No TOF hex data found in log file!")
        return
    
    print(f"Found {len(distance_arrays)} sensor measurements")
    
    # Convert to numpy arrays (n x 8 x 8)
    distance_data = np.stack(distance_arrays, axis=0)
    target_status_data = np.stack(target_status_arrays, axis=0)
    valid_counts = np.array(valid_measurements)
    
    print(f"Distance data shape: {distance_data.shape}")
    print(f"Target status data shape: {target_status_data.shape}")
    print(f"Valid measurements per frame: {np.mean(valid_counts):.1f} ± {np.std(valid_counts):.1f} (out of 64)")
    
    # Calculate statistics for distance data
    mean_distance = np.mean(distance_data, axis=0)
    std_distance = np.std(distance_data, axis=0)
    
    # Calculate validity statistics (percentage of valid measurements per zone)
    validity_percentage = np.mean((target_status_data == 5) | (target_status_data == 9), axis=0) * 100
    
    print(f"Distance mean range: {np.min(mean_distance):.2f} - {np.max(mean_distance):.2f} cm")
    print(f"Distance std range: {np.min(std_distance):.2f} - {np.max(std_distance):.2f} cm")
    print(f"Validity range: {np.min(validity_percentage):.1f}% - {np.max(validity_percentage):.1f}%")
    
    # Create output directory in the project root
    script_dir = Path(__file__).parent
    project_dir = script_dir.parent
    output_dir = project_dir / "vl53l8ch_analysis"
    output_dir.mkdir(exist_ok=True)
    
    # Generate visualizations
    log_name = Path(log_file_path).stem
    
    # Distance mean heatmap
    mean_output = output_dir / f"{log_name}_distance_mean.png"
    create_heatmap(mean_distance, f"VL53L8CH Distance Mean ({len(distance_arrays)} measurements)", 
                   mean_output, cmap='plasma')
    
    # Distance standard deviation heatmap
    std_output = output_dir / f"{log_name}_distance_std.png"
    create_heatmap(std_distance, f"VL53L8CH Distance Std Dev ({len(distance_arrays)} measurements)", 
                   std_output, cmap='hot')
    
    # Validity percentage heatmap
    validity_output = output_dir / f"{log_name}_validity.png"
    create_heatmap(validity_percentage, f"VL53L8CH Validity Percentage ({len(distance_arrays)} measurements)", 
                   validity_output, cmap='RdYlGn')
    
    # Save raw data
    np.save(output_dir / f"{log_name}_distance_data.npy", distance_data)
    np.save(output_dir / f"{log_name}_target_status_data.npy", target_status_data)
    np.save(output_dir / f"{log_name}_distance_mean.npy", mean_distance)
    np.save(output_dir / f"{log_name}_distance_std.npy", std_distance)
    np.save(output_dir / f"{log_name}_validity.npy", validity_percentage)
    
    print(f"\nAnalysis complete! Files saved to: {output_dir}")
    print(f"- Distance mean heatmap: {mean_output}")
    print(f"- Distance std heatmap: {std_output}")
    print(f"- Validity heatmap: {validity_output}")
    print(f"- Raw distance data: {output_dir / f'{log_name}_distance_data.npy'}")
    print(f"- Raw target status data: {output_dir / f'{log_name}_target_status_data.npy'}")

def find_latest_log_with_data():
    """
    Find the latest log file that contains VL53L8CH hex data.
    """
    # Get the parent directory (section-controller) from scripts directory
    script_dir = Path(__file__).parent
    project_dir = script_dir.parent
    log_dir = project_dir / 'logs'
    
    # Get all log files sorted by modification time (newest first)
    log_files = sorted(log_dir.glob('device-monitor-*.log'), key=lambda f: f.stat().st_mtime, reverse=True)
    
    if not log_files:
        print("No log files found!")
        return None
    
    # Check each log file for VL53L8CH data
    for log_file in log_files:
        try:
            with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
                # Check first few lines for TOF data
                for i, line in enumerate(f):
                    if 'TOF: HEX DATA:' in line:
                        print(f"Found TOF data in: {log_file}")
                        return str(log_file)
                    if i > 100:  # Don't check entire file, just first 100 lines
                        break
        except Exception as e:
            print(f"Error reading {log_file}: {e}")
            continue
    
    print("No log files with TOF data found!")
    return None

def main():
    parser = argparse.ArgumentParser(description='Parse VL53L8CH sensor data from PlatformIO logs')
    parser.add_argument('log_file', nargs='?', 
                       help='Path to log file (default: automatically find latest)')
    
    args = parser.parse_args()
    
    if args.log_file:
        log_file = args.log_file
        if not Path(log_file).exists():
            print(f"Error: Log file not found: {log_file}")
            return
    else:
        # Automatically find latest log with VL53L8CH data
        log_file = find_latest_log_with_data()
        if not log_file:
            return
    
    analyze_sensor_data(log_file)

if __name__ == "__main__":
    main()