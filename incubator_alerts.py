#!/usr/bin/env python3
"""
═══════════════════════════════════════════════════════════════════════
 INCUBATOR EMERGENCY ALERT SYSTEM
 
 Authors: Mahaveer Katighar, S. Hema Vaishnavi, S.K Asifa,
          Samyuga, Akshara Mikkilineni
 
 License: MIT
 Repository: https://github.com/YOUR_USERNAME/smart-incubator-control
═══════════════════════════════════════════════════════════════════════
"""

import smtplib
import firebase_admin
from firebase_admin import credentials, db
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from datetime import datetime
import time

try:
    from config import *
except ImportError:
    print("ERROR: config.py not found!")
    print("Please create config.py with your credentials.")
    exit(1)

last_alert_time = {}
last_data_time = None

def send_email_alert(subject, body_html, recipients, priority="high"):
    """Send email alert to specified recipients"""
    
    try:
        msg = MIMEMultipart('alternative')
        msg['From'] = SENDER_EMAIL
        msg['To'] = ', '.join(recipients)
        msg['Subject'] = subject
        
        if priority == "high":
            msg['X-Priority'] = '1'
            msg['X-MSMail-Priority'] = 'High'
            msg['Importance'] = 'High'
        
        html_part = MIMEText(body_html, 'html')
        msg.attach(html_part)
        
        server = smtplib.SMTP(SMTP_SERVER, SMTP_PORT, timeout=30)
        server.starttls()
        server.login(SENDER_EMAIL, SENDER_PASSWORD)
        server.send_message(msg)
        server.quit()
        
        print(f"✓ Email sent: {subject}")
        print(f"  Recipients: {', '.join(recipients)}")
        return True
        
    except Exception as e:
        print(f"✗ Email send failed: {e}")
        return False


def create_alert_html(alert_type, device_data, details):
    """Create HTML email body for alert"""
    
    current_temp = device_data.get('currentTemp', 0)
    target_temp = device_data.get('targetTemp', 36.5)
    status = device_data.get('status', 'UNKNOWN')
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    if alert_type in ['CRITICAL_TEMP', 'POWER_FAILURE', 'SENSOR_FAULT', 'EMERGENCY_SHUTDOWN']:
        severity_color = "#ff0066"
        severity_text = "🚨 CRITICAL EMERGENCY"
    else:
        severity_color = "#ffaa00"
        severity_text = "⚠️ WARNING"
    
    html = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <style>
            body {{
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                background-color: #f5f5f5;
                margin: 0;
                padding: 20px;
            }}
            .container {{
                max-width: 600px;
                margin: 0 auto;
                background: white;
                border-radius: 10px;
                overflow: hidden;
                box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            }}
            .header {{
                background: {severity_color};
                color: white;
                padding: 30px;
                text-align: center;
            }}
            .header h1 {{
                margin: 0;
                font-size: 24px;
                font-weight: 600;
            }}
            .severity {{
                margin-top: 10px;
                font-size: 18px;
                font-weight: 700;
            }}
            .content {{
                padding: 30px;
            }}
            .alert-details {{
                background: #f8f9fa;
                border-left: 4px solid {severity_color};
                padding: 20px;
                margin: 20px 0;
                border-radius: 4px;
            }}
            .detail-row {{
                display: flex;
                justify-content: space-between;
                padding: 10px 0;
                border-bottom: 1px solid #e0e0e0;
            }}
            .detail-row:last-child {{
                border-bottom: none;
            }}
            .label {{
                font-weight: 600;
                color: #333;
            }}
            .value {{
                color: #666;
                text-align: right;
            }}
            .temp-critical {{
                color: {severity_color};
                font-weight: 700;
                font-size: 18px;
            }}
            .action-required {{
                background: #fff3cd;
                border: 2px solid #ffc107;
                padding: 20px;
                margin: 20px 0;
                border-radius: 4px;
            }}
            .action-required h3 {{
                margin-top: 0;
                color: #856404;
            }}
            .action-required ul {{
                margin: 10px 0;
                padding-left: 20px;
            }}
            .action-required li {{
                margin: 5px 0;
                color: #856404;
            }}
            .footer {{
                background: #f8f9fa;
                padding: 20px;
                text-align: center;
                color: #666;
                font-size: 12px;
            }}
        </style>
    </head>
    <body>
        <div class="container">
            <div class="header">
                <h1>INCUBATOR ALERT</h1>
                <div class="severity">{severity_text}</div>
            </div>
            
            <div class="content">
                <h2 style="color: {severity_color};">Alert: {alert_type}</h2>
                <p><strong>{details}</strong></p>
                
                <div class="alert-details">
                    <h3 style="margin-top: 0;">Device Information</h3>
                    <div class="detail-row">
                        <span class="label">Device ID:</span>
                        <span class="value">{DEVICE_ID}</span>
                    </div>
                    <div class="detail-row">
                        <span class="label">Hospital:</span>
                        <span class="value">{HOSPITAL_NAME}</span>
                    </div>
                    <div class="detail-row">
                        <span class="label">Location:</span>
                        <span class="value">NICU Ward A</span>
                    </div>
                    <div class="detail-row">
                        <span class="label">Alert Time:</span>
                        <span class="value">{timestamp}</span>
                    </div>
                </div>
                
                <div class="alert-details">
                    <h3 style="margin-top: 0;">Current Status</h3>
                    <div class="detail-row">
                        <span class="label">Current Temperature:</span>
                        <span class="value temp-critical">{current_temp:.1f}°C</span>
                    </div>
                    <div class="detail-row">
                        <span class="label">Target Temperature:</span>
                        <span class="value">{target_temp:.1f}°C</span>
                    </div>
                    <div class="detail-row">
                        <span class="label">System Status:</span>
                        <span class="value">{status}</span>
                    </div>
                    <div class="detail-row">
                        <span class="label">Heater:</span>
                        <span class="value">{'ON' if device_data.get('heaterState') else 'OFF'}</span>
                    </div>
                    <div class="detail-row">
                        <span class="label">Cooler:</span>
                        <span class="value">{'ON' if device_data.get('coolerState') else 'OFF'}</span>
                    </div>
                </div>
                
                <div class="action-required">
                    <h3>⚡ IMMEDIATE ACTION REQUIRED</h3>
                    <ul>
                        <li>Check infant immediately</li>
                        <li>Verify incubator display and controls</li>
                        <li>Monitor temperature closely</li>
                        <li>Contact biomedical engineering if needed</li>
                        <li>Document all actions taken</li>
                    </ul>
                </div>
            </div>
            
            <div class="footer">
                <p><strong>Incubator Monitoring System v5.1</strong></p>
                <p>This is an automated alert. Do not reply to this email.</p>
                <p>For technical support, contact Biomedical Engineering</p>
            </div>
        </div>
    </body>
    </html>
    """
    
    return html


def send_critical_alert(alert_type, device_data, details):
    """Send critical alert to all doctors and nurses"""
    
    alert_key = f"{alert_type}_{DEVICE_ID}"
    current_time = time.time()
    
    if alert_key in last_alert_time:
        time_since_last = current_time - last_alert_time[alert_key]
        if time_since_last < ALERT_COOLDOWN:
            print(f"⏳ Alert in cooldown ({int(ALERT_COOLDOWN - time_since_last)}s remaining)")
            return False
    
    recipients = []
    for doctor in ALERT_CONTACTS['doctors']:
        if doctor['email'] not in recipients:
            recipients.append(doctor['email'])
    for nurse in ALERT_CONTACTS['nurses']:
        if nurse['email'] not in recipients:
            recipients.append(nurse['email'])
    
    subject = f"🚨 CRITICAL: Incubator {DEVICE_ID} - {alert_type}"
    body_html = create_alert_html(alert_type, device_data, details)
    
    success = send_email_alert(subject, body_html, recipients, priority="high")
    
    if success:
        last_alert_time[alert_key] = current_time
        
        print(f"\n{'='*60}")
        print(f"🚨 CRITICAL ALERT SENT")
        print(f"{'='*60}")
        print(f"Type: {alert_type}")
        print(f"Device: {DEVICE_ID}")
        print(f"Details: {details}")
        print(f"Recipients: {len(recipients)} people")
        print(f"Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"{'='*60}\n")
    
    return success


def check_temperature(device_data):
    """Check if temperature is in critical range"""
    
    current_temp = device_data.get('currentTemp', 0)
    
    if current_temp < CRITICAL_LOW_TEMP:
        details = f"Temperature dangerously low: {current_temp:.1f}°C (Critical minimum: {CRITICAL_LOW_TEMP}°C)"
        send_critical_alert("CRITICAL LOW TEMPERATURE", device_data, details)
        return False
    
    if current_temp > CRITICAL_HIGH_TEMP:
        details = f"Temperature dangerously high: {current_temp:.1f}°C (Critical maximum: {CRITICAL_HIGH_TEMP}°C)"
        send_critical_alert("CRITICAL HIGH TEMPERATURE", device_data, details)
        return False
    
    return True


def check_sensor_fault(device_data):
    """Check for sensor malfunction"""
    
    status = device_data.get('status', '')
    
    if 'SENSOR_FAULT' in status or 'SENSOR' in status:
        details = f"Sensor malfunction detected. System status: {status}"
        send_critical_alert("SENSOR FAULT", device_data, details)
        return False
    
    return True


def check_emergency_shutdown(device_data):
    """Check if system is in emergency shutdown"""
    
    status = device_data.get('status', '')
    
    if 'EMERGENCY' in status:
        details = f"System in emergency shutdown. Status: {status}"
        send_critical_alert("EMERGENCY SHUTDOWN", device_data, details)
        return False
    
    return True


def data_listener(event):
    """Firebase data change listener"""
    
    global last_data_time
    last_data_time = time.time()
    
    data = event.data
    if not data:
        print("⚠️ No data received")
        return
    
    print(f"📊 Data received - Temp: {data.get('currentTemp', 0):.1f}°C | Status: {data.get('status', 'UNKNOWN')}")
    
    check_temperature(data)
    check_sensor_fault(data)
    check_emergency_shutdown(data)


def main():
    """Main monitoring loop"""
    
    print("╔═══════════════════════════════════════════════════════════╗")
    print("║     INCUBATOR EMERGENCY ALERT SYSTEM v5.1                ║")
    print("╚═══════════════════════════════════════════════════════════╝")
    print()
    
    print("→ Initializing Firebase...")
    try:
        cred = credentials.Certificate(SERVICE_ACCOUNT_PATH)
        firebase_admin.initialize_app(cred, FIREBASE_CONFIG)
        print("  ✓ Firebase connected")
    except Exception as e:
        print(f"  ✗ Firebase connection failed: {e}")
        print("\n⚠️  Please download serviceAccountKey.json from Firebase Console:")
        print("    Project Settings → Service Accounts → Generate New Private Key")
        return
    
    print("\n→ Alert Configuration:")
    print(f"  Device: {DEVICE_ID}")
    print(f"  Hospital: {HOSPITAL_NAME}")
    print(f"  Critical Low Temp: {CRITICAL_LOW_TEMP}°C")
    print(f"  Critical High Temp: {CRITICAL_HIGH_TEMP}°C")
    print(f"  Power Failure Timeout: {POWER_FAILURE_TIMEOUT}s")
    
    print("\n→ Alert Recipients:")
    print(f"  Doctors: {len(ALERT_CONTACTS['doctors'])}")
    for doc in ALERT_CONTACTS['doctors']:
        print(f"    - {doc['name']} ({doc['email']})")
    print(f"  Nurses: {len(ALERT_CONTACTS['nurses'])}")
    for nurse in ALERT_CONTACTS['nurses']:
        print(f"    - {nurse['name']} ({nurse['email']})")
    
    print("\n→ Starting real-time monitoring...")
    print("  Press Ctrl+C to stop\n")
    
    device_ref = db.reference(f"{HOSPITAL_NAME}/devices/{DEVICE_ID}/current")
    device_ref.listen(data_listener)
    
    try:
        while True:
            time.sleep(5)
            
            if last_data_time:
                time_since_data = time.time() - last_data_time
                if time_since_data > POWER_FAILURE_TIMEOUT:
                    print(f"⚠️ WARNING: No data for {int(time_since_data)}s")
            
    except KeyboardInterrupt:
        print("\n\n→ Monitoring stopped by user")
    except Exception as e:
        print(f"\n✗ Error: {e}")
    
    print("\n╔═══════════════════════════════════════════════════════════╗")
    print("║     Alert System Shutdown                                ║")
    print("╚═══════════════════════════════════════════════════════════╝\n")


if __name__ == "__main__":
    main()
