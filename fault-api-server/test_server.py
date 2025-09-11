#!/usr/bin/env python3
"""
Test server to simulate the fault detection API endpoint
Receives POST requests from ESP32 fault detector
"""

from flask import Flask, request, jsonify
import json
from datetime import datetime
import logging

app = Flask(__name__)

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Store fault reports for demonstration
fault_reports = []

@app.route('/api/fault/esp32', methods=['POST'])
def receive_fault_data():
    """Receive fault data from ESP32"""
    
    # Check API key
    api_key = request.headers.get('x-api-key')
    if api_key != 'sexophobia69':
        logger.warning(f"Invalid API key: {api_key}")
        return jsonify({"error": "Invalid API key"}), 401
    
    # Check content type
    if request.content_type != 'application/json':
        return jsonify({"error": "Content-Type must be application/json"}), 400
    
    try:
        # Parse JSON data
        fault_data = request.get_json()
        
        # Validate required fields
        required_fields = ['fault-type', 'light-ID', 'timestamp']
        for field in required_fields:
            if field not in fault_data:
                return jsonify({"error": f"Missing required field: {field}"}), 400
        
        # Log the received data
        logger.info("="*60)
        logger.info("🚨 FAULT REPORT RECEIVED")
        logger.info("="*60)
        logger.info(f"📊 Fault Type: {fault_data['fault-type']}")
        logger.info(f"💡 Light ID: {fault_data['light-ID']}")
        logger.info(f"🕒 Timestamp: {fault_data['timestamp']}")
        logger.info(f"🌐 Source IP: {request.remote_addr}")
        logger.info("-"*60)
        
        # Add server timestamp
        fault_data['server_received_at'] = datetime.utcnow().isoformat() + 'Z'
        fault_data['source_ip'] = request.remote_addr
        
        # Store the report
        fault_reports.append(fault_data)
        
        # Pretty print the JSON
        print(json.dumps(fault_data, indent=2))
        
        # Return success response
        response = {
            "status": "success",
            "message": "Fault report received successfully",
            "report_id": len(fault_reports),
            "received_at": fault_data['server_received_at']
        }
        
        return jsonify(response), 201
        
    except json.JSONDecodeError:
        logger.error("Invalid JSON in request body")
        return jsonify({"error": "Invalid JSON"}), 400
    except Exception as e:
        logger.error(f"Error processing request: {str(e)}")
        return jsonify({"error": "Internal server error"}), 500

@app.route('/api/fault/reports', methods=['GET'])
def get_fault_reports():
    """Get all fault reports (for testing/debugging)"""
    return jsonify({
        "total_reports": len(fault_reports),
        "reports": fault_reports[-10:]  # Return last 10 reports
    })

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        "status": "healthy",
        "server_time": datetime.utcnow().isoformat() + 'Z',
        "total_reports": len(fault_reports)
    })

@app.route('/', methods=['GET'])
def index():
    """Simple index page"""
    return f"""
    <h1>ESP32 Fault Detection API Server</h1>
    <p>Server is running and ready to receive fault reports.</p>
    <ul>
        <li><strong>POST</strong> /api/fault/esp32 - Receive fault data</li>
        <li><strong>GET</strong> /api/fault/reports - View recent reports</li>
        <li><strong>GET</strong> /api/health - Health check</li>
    </ul>
    <p>Total reports received: <strong>{len(fault_reports)}</strong></p>
    <p>Server time: {datetime.utcnow().isoformat()}Z</p>
    """

if __name__ == '__main__':
    print("🚀 Starting ESP32 Fault Detection API Server")
    print("📡 Listening on http://0.0.0.0:3000")
    print("🔑 API Key: sexophobia69")
    print("="*50)
    
    app.run(host='0.0.0.0', port=3000, debug=True)
