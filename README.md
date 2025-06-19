# Smart Refrigerator System with Real-Time Monitoring and Predictive Analytics

## 📌 Overview

This project implements an IoT-based smart refrigerator system designed to monitor internal environmental conditions and predict the risk of food spoilage using real-time data and machine learning. The system is intended to enhance food safety, reduce waste, and improve refrigerator efficiency through intelligent analytics and automated decision-making.

[![Demo Video](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID/0.jpg)](https://youtu.be/pIoxiVyhqx8)

---

## 🛠️ Technologies Used

- **ESP32 Microcontroller**  
- **Sensors**: DHT11/DHT22 (Temperature & Humidity), MQ135 (Gas), Magnetic Reed Switch (Door Open Detection)  
- **Azure Functions** (HTTP Trigger & Timer Trigger)  
- **Azure SQL Database**  
- **Python (Jupyter Notebook)** for ML model training  
- **Power BI** for real-time dashboards  
- **Azure Blob Storage** (initial phase, for raw CSV storage)

---

## 🔁 System Workflow

ESP32 Sensors --> Azure HTTP Function --> Azure SQL DB
↓
ML Prediction via Timer Function
↓
Prediction Results → Power BI


---

## ⚙️ Features

- Real-time sensor data collection (temperature, humidity, gas, door count)
- Cloud-based ingestion using Azure Functions
- Live storage and querying using Azure SQL Database
- Predictive food spoilage risk model (Safe / Warning / High)
- Real-time visualization in Power BI (DirectQuery)
- Status dashboards for food quality, fridge performance, and historical trends

---

## 🧠 Machine Learning Model

- **Problem Type**: Multi-class classification (food spoilage risk)
- **Model Inputs**: Temperature, Humidity, Gas levels
- **Classes**:
  - `Safe`: All environmental conditions within ideal ranges
  - `Warning`: Borderline values indicating possible spoilage
  - `High`: Clear signs of potential food spoilage
- **Model Used**: Random Forest (trained in Jupyter Notebook)
- **Deployment**: Model and label encoder serialized as `.pkl` and used in Azure Timer Trigger Function

---

## 📊 Power BI Dashboard Highlights

- **Real-Time View**: Live temperature, humidity, and gas readings
- **Food Status Panel**: ML-based risk classification
- **Fridge Condition Panel**: 30-day performance analysis
- **Door Activity Trends**: Daily and hourly breakdowns
- **Analytics View**: Historical trends and summaries

---

## 📷 Screenshots
![Screenshot 2025-05-03 072908](https://github.com/user-attachments/assets/76db85f8-fc32-43c6-98a8-5ba50f2b4fa0)
![Screenshot 2025-05-03 074524](https://github.com/user-attachments/assets/dc3c4f2e-f6c5-48c4-95a4-d41b8656ee7b)
![Screenshot 2025-05-03 075011](https://github.com/user-attachments/assets/ec49cbb0-dcc5-4ea2-9e3d-984fe47bbe90)




---

## 👥 Contributors

- **BiNuda Dewhan**  
- **Tharusha Nelligahawatte**

---

## 📬 Contact

For questions or collaboration, feel free to reach out via LinkedIn!
www.linkedin.com/in/binuda-dewhan

