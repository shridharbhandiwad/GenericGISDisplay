#!/usr/bin/env python3
"""
GPS UDP Test Sender

This script sends simulated GPS data via UDP to test the GPS Map Viewer application.
It supports multiple data formats: JSON, CSV, and NMEA.

Usage:
    python3 test_sender.py [options]

Options:
    --host HOST     Target host (default: localhost)
    --port PORT     Target port (default: 12345)
    --format FORMAT Data format: json, csv, nmea (default: json)
    --interval SEC  Send interval in seconds (default: 1.0)
    --simulate      Simulate moving GPS coordinates (default: False)
    --help          Show this help message
"""

import socket
import time
import json
import math
import argparse
import sys
from datetime import datetime

class GPSSimulator:
    def __init__(self):
        # Starting position (example: New York City)
        self.base_lat = 40.7128
        self.base_lon = -74.0060
        self.base_alt = 10.0
        
        # Movement parameters
        self.radius = 0.01  # Movement radius in degrees
        self.speed = 0.1    # Movement speed
        self.time_offset = 0
        
    def get_position(self, simulate_movement=False):
        """Get current GPS position"""
        if simulate_movement:
            # Simulate circular movement
            angle = self.time_offset * self.speed
            lat = self.base_lat + self.radius * math.sin(angle)
            lon = self.base_lon + self.radius * math.cos(angle)
            alt = self.base_alt + 5 * math.sin(angle * 2)  # Varying altitude
            self.time_offset += 1
        else:
            # Static position with small random variations
            lat = self.base_lat + (time.time() % 10 - 5) * 0.0001
            lon = self.base_lon + (time.time() % 10 - 5) * 0.0001
            alt = self.base_alt + (time.time() % 10 - 5) * 0.5
        
        return lat, lon, alt

class UDPSender:
    def __init__(self, host='localhost', port=12345):
        self.host = host
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.gps_sim = GPSSimulator()
        
    def format_json(self, lat, lon, alt):
        """Format GPS data as JSON"""
        data = {
            "latitude": lat,
            "longitude": lon,
            "altitude": alt,
            "timestamp": datetime.now().isoformat(),
            "accuracy": 3.5,
            "speed": 0.0,
            "heading": 0.0
        }
        return json.dumps(data).encode('utf-8')
    
    def format_csv(self, lat, lon, alt):
        """Format GPS data as CSV"""
        return f"{lat:.6f},{lon:.6f},{alt:.2f}".encode('utf-8')
    
    def format_nmea(self, lat, lon, alt):
        """Format GPS data as NMEA GPGGA sentence"""
        # Convert decimal degrees to NMEA format
        lat_deg = int(abs(lat))
        lat_min = (abs(lat) - lat_deg) * 60
        lat_dir = 'N' if lat >= 0 else 'S'
        
        lon_deg = int(abs(lon))
        lon_min = (abs(lon) - lon_deg) * 60
        lon_dir = 'E' if lon >= 0 else 'W'
        
        # Create GPGGA sentence (simplified)
        time_str = datetime.now().strftime("%H%M%S")
        lat_str = f"{lat_deg:02d}{lat_min:07.4f}"
        lon_str = f"{lon_deg:03d}{lon_min:07.4f}"
        
        sentence = f"$GPGGA,{time_str},{lat_str},{lat_dir},{lon_str},{lon_dir},1,08,1.0,{alt:.1f},M,46.9,M,,*47"
        return sentence.encode('utf-8')
    
    def send_data(self, data_format='json', simulate_movement=False, interval=1.0):
        """Send GPS data continuously"""
        print(f"Starting GPS UDP sender...")
        print(f"Target: {self.host}:{self.port}")
        print(f"Format: {data_format}")
        print(f"Interval: {interval}s")
        print(f"Movement: {'Simulated' if simulate_movement else 'Static with noise'}")
        print("Press Ctrl+C to stop\n")
        
        try:
            while True:
                lat, lon, alt = self.gps_sim.get_position(simulate_movement)
                
                # Format data according to specified format
                if data_format == 'json':
                    data = self.format_json(lat, lon, alt)
                elif data_format == 'csv':
                    data = self.format_csv(lat, lon, alt)
                elif data_format == 'nmea':
                    data = self.format_nmea(lat, lon, alt)
                else:
                    raise ValueError(f"Unknown format: {data_format}")
                
                # Send data
                self.socket.sendto(data, (self.host, self.port))
                
                # Print status
                timestamp = datetime.now().strftime("%H:%M:%S")
                print(f"[{timestamp}] Sent: Lat={lat:.6f}, Lon={lon:.6f}, Alt={alt:.2f}m")
                
                time.sleep(interval)
                
        except KeyboardInterrupt:
            print("\nStopping GPS sender...")
        except Exception as e:
            print(f"Error: {e}")
        finally:
            self.socket.close()

def main():
    parser = argparse.ArgumentParser(description='GPS UDP Test Sender')
    parser.add_argument('--host', default='localhost', help='Target host (default: localhost)')
    parser.add_argument('--port', type=int, default=12345, help='Target port (default: 12345)')
    parser.add_argument('--format', choices=['json', 'csv', 'nmea'], default='json',
                       help='Data format (default: json)')
    parser.add_argument('--interval', type=float, default=1.0,
                       help='Send interval in seconds (default: 1.0)')
    parser.add_argument('--simulate', action='store_true',
                       help='Simulate moving GPS coordinates')
    
    args = parser.parse_args()
    
    # Create and start sender
    sender = UDPSender(args.host, args.port)
    sender.send_data(args.format, args.simulate, args.interval)

if __name__ == '__main__':
    main()