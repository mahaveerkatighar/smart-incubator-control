"""
═══════════════════════════════════════════════════════════════════════
 PYTHON CONFIGURATION FILE
 
 IMPORTANT: Fill in your actual credentials below
 This file contains sensitive data - do NOT commit to GitHub
═══════════════════════════════════════════════════════════════════════
"""

# ═══════════════════════════════════════════════════════════════════════
# Firebase Configuration
# ═══════════════════════════════════════════════════════════════════════

FIREBASE_CONFIG = {
    "databaseURL": "https://your-project-default-rtdb.region.firebasedatabase.app/"
}

# Download from Firebase Console: Project Settings → Service Accounts
SERVICE_ACCOUNT_PATH = "serviceAccountKey.json"

# ═══════════════════════════════════════════════════════════════════════
# Hospital Configuration
# ═══════════════════════════════════════════════════════════════════════

HOSPITAL_NAME = "City_Hospital"
DEVICE_ID = "INCUBATOR_001"

# ═══════════════════════════════════════════════════════════════════════
# Email Configuration
# ═══════════════════════════════════════════════════════════════════════

SMTP_SERVER = "smtp.gmail.com"
SMTP_PORT = 587

# IMPORTANT: Use Gmail App Password, not regular password!
# How to get App Password:
# 1. Go to https://myaccount.google.com/apppasswords
# 2. Enable 2-Factor Authentication first
# 3. Generate new app password for "Mail"
# 4. Copy the 16-character password here

SENDER_EMAIL = "your_email@gmail.com"
SENDER_PASSWORD = "your_16_character_app_password"

# ═══════════════════════════════════════════════════════════════════════
# Alert Recipients
# ═══════════════════════════════════════════════════════════════════════

ALERT_CONTACTS = {
    "doctors": [
        {
            "name": "Dr. John Doe",
            "email": "doctor1@hospital.com",
            "phone": "+1-234-567-8900"
        },
        {
            "name": "Dr. Jane Smith",
            "email": "doctor2@hospital.com",
            "phone": "+1-234-567-8901"
        }
    ],
    "nurses": [
        {
            "name": "Nurse Alice",
            "email": "nurse1@hospital.com",
            "phone": "+1-234-567-8902"
        },
        {
            "name": "Nurse Bob",
            "email": "nurse2@hospital.com",
            "phone": "+1-234-567-8903"
        }
    ]
}

# ═══════════════════════════════════════════════════════════════════════
# Alert Thresholds
# ═══════════════════════════════════════════════════════════════════════

CRITICAL_LOW_TEMP = 34.0
CRITICAL_HIGH_TEMP = 38.5
POWER_FAILURE_TIMEOUT = 15  # seconds
ALERT_COOLDOWN = 300  # 5 minutes between same alerts
```

---

## 📁 **File 9: requirements.txt**

**Filename:** `requirements.txt`
**Location:** Same folder as `incubator_alerts.py`
```
firebase-admin>=6.0.0
```

---

## 📋 **SUMMARY - All Files Created:**

### **Files to Upload to GitHub:**

1. ✅ **README.md** - Main documentation
2. ✅ **LICENSE** - MIT License
3. ✅ **.gitignore** - Protects credentials
4. ✅ **incubator_control.ino** - ESP32 code
5. ✅ **dashboard.html** - Web interface
6. ✅ **incubator_alerts.py** - Python alerts
7. ✅ **requirements.txt** - Python dependencies

### **Files to Create Locally (NOT upload):**

8. ⚠️ **config.h** - Your WiFi/Firebase credentials
9. ⚠️ **config.py** - Your email/Firebase credentials
10. ⚠️ **serviceAccountKey.json** - Firebase service key

---

## 🎯 **EXACT STEPS:**

### **Step 1: Create GitHub Repository**
1. Go to https://github.com/new
2. Repository name: `smart-incubator-control`
3. Public or Private: Your choice
4. ✅ Add LICENSE: MIT
5. Click **Create repository**

### **Step 2: Upload Files**
1. Click **"uploading an existing file"**
2. **Create these files one by one:**
   - Click **"Add file" → "Create new file"**
   - Copy filename and paste code
   - Click **"Commit new file"**

**Upload these 7 files:**
- README.md
- LICENSE  
- .gitignore
- incubator_control.ino
- dashboard.html
- incubator_alerts.py
- requirements.txt

### **Step 3: Create Config Files Locally**
**On your computer** (not GitHub):
- Create **config.h** with your WiFi/Firebase credentials
- Create **config.py** with your email credentials
- These stay private on your computer!

---

## ✅ **DONE!**

Your repository is ready at:
```
https://github.com/mahaveerkatighar/smart-incubator-control
