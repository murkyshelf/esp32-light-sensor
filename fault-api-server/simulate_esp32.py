#!/usr/bin/env python3
"""
ESP32 Fault Detection System Simulator
Simulates various fault conditions and sends them to the API server
"""

import requests
import json
import time
import random
from datetime import datetime

# Configuration
API_URL = "http://localhost:3000/api/fault/esp32"
API_KEY = "sexophobia69"
LIGHT_ID = "CB-420 light#2"

# Fault simulation scenarios
fault_scenarios = [
    {"name": "Normal Operation", "fault_type": "normal", "duration": 10},
    {"name": "Short Circuit", "fault_type": "short_circuit", "duration": 5},
    {"name": "Open Circuit", "fault_type": "open_circuit", "duration": 8},
    {"name": "Overvoltage", "fault_type": "overvoltage", "duration": 6},
    {"name": "Undervoltage", "fault_type": "undervoltage", "duration": 7},
    {"name": "Voltage Fluctuation", "fault_type": "voltage_fluctuation", "duration": 4},
    {"name": "Normal Recovery", "fault_type": "normal", "duration": 15},
]

def get_iso_timestamp():
    """Generate ISO 8601 timestamp"""
    return datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")

def send_fault_report(fault_type, light_id, timestamp):
    """Send fault report to API server"""
    headers = {
        "Content-Type": "application/json",
        "x-api-key": API_KEY
    }
    
    payload = {
        "fault-type": fault_type,
        "light-ID": light_id,
        "timestamp": timestamp
    }
    
    try:
        response = requests.post(API_URL, headers=headers, json=payload, timeout=5)
        if response.status_code in [200, 201]:
            print(f"✅ {fault_type.upper()}: Report sent successfully")
            print(f"   📊 Server Response: {response.json()}")
        else:
            print(f"❌ {fault_type.upper()}: Server error {response.status_code}")
            print(f"   📊 Response: {response.text}")
    except requests.exceptions.RequestException as e:
        print(f"❌ {fault_type.upper()}: Network error - {e}")
    
    print(f"   📡 Sent: {json.dumps(payload, indent=2)}")
    print("-" * 60)

def simulate_esp32_fault_detection():
    """Main simulation function"""
    print("🚀 ESP32 Fault Detection System Simulation")
    print("="*60)
    print(f"🎯 Target API: {API_URL}")
    print(f"🔑 API Key: {API_KEY}")
    print(f"💡 Light ID: {LIGHT_ID}")
    print("="*60)
    
    # Check if server is running
    try:
        response = requests.get("http://localhost:3000/api/health")
        if response.status_code == 200:
            print("✅ API Server is running and healthy")
            health_data = response.json()
            print(f"   📊 Server Status: {health_data}")
        else:
            print("⚠️  API Server responded with error")
    except requests.exceptions.RequestException as e:
        print(f"❌ Cannot connect to API server: {e}")
        print("   Make sure the server is running on port 3000")
        return
    
    print("\n🔄 Starting fault simulation scenarios...")
    print("=" * 60)
    
    scenario_count = 1
    for scenario in fault_scenarios:
        print(f"\n📋 Scenario {scenario_count}: {scenario['name']}")
        print(f"⏱️  Duration: {scenario['duration']} seconds")
        print(f"🔍 Fault Type: {scenario['fault_type']}")
        
        # Simulate periodic reports during the scenario
        start_time = time.time()
        report_interval = 3  # Send report every 3 seconds
        last_report_time = 0
        
        while (time.time() - start_time) < scenario['duration']:
            current_time = time.time()
            
            # Send report at intervals
            if (current_time - last_report_time) >= report_interval:
                timestamp = get_iso_timestamp()
                send_fault_report(scenario['fault_type'], LIGHT_ID, timestamp)
                last_report_time = current_time
            
            time.sleep(0.5)  # Check every 0.5 seconds
        
        scenario_count += 1
        
        # Brief pause between scenarios
        if scenario_count <= len(fault_scenarios):
            print(f"\n⏳ Transitioning to next scenario in 2 seconds...")
            time.sleep(2)
    
    print("\n🎉 Simulation completed successfully!")
    print("📈 Check the API server logs to see all received fault reports")

if __name__ == "__main__":
    try:
        simulate_esp32_fault_detection()
    except KeyboardInterrupt:
        print("\n\n⏹️  Simulation stopped by user")
    except Exception as e:
        print(f"\n❌ Simulation error: {e}")
