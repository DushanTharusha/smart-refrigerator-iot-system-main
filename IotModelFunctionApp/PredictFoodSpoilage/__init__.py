import logging
import pandas as pd
import pyodbc
import pickle
import os
import pytz
from datetime import datetime
import azure.functions as func

# Load the model and label encoder
model_path = os.path.join(os.path.dirname(__file__), "../food_risk_model.pkl")
encoder_path = os.path.join(os.path.dirname(__file__), "../label_encoder.pkl")

model = pickle.load(open(model_path, "rb"))
encoder = pickle.load(open(encoder_path, "rb"))

# Define Sri Lanka timezone
sri_lanka_tz = pytz.timezone("Asia/Colombo")

def main(mytimer: func.TimerRequest) -> None:
    logging.info("⏰ Azure Function triggered for prediction.")

    try:
        # Get database credentials from environment variables
        server = os.environ["SQL_SERVER"]
        database = os.environ["SQL_DATABASE"]
        username = os.environ["SQL_USER"]
        password = os.environ["SQL_PASSWORD"]

        conn_str = (
            f"DRIVER={{ODBC Driver 17 for SQL Server}};"
            f"SERVER={server};DATABASE={database};UID={username};PWD={password}"
        )

        # Connect to Azure SQL
        conn = pyodbc.connect(conn_str)
        cursor = conn.cursor()

        # Get the latest valid sensor data from FridgeData
        query = """
        SELECT TOP 1 temperature_dht11, humidity_dht11, gas1
        FROM FridgeData
        WHERE gas1 < 4000
        ORDER BY [date] DESC, [time] DESC
        """
        df = pd.read_sql(query, conn)

        # Prepare data for prediction
        X = df[['temperature_dht11', 'humidity_dht11', 'gas1']]
        prediction = model.predict(X)[0]
        predicted_label = encoder.inverse_transform([prediction])[0]

        # Get current time in Sri Lanka timezone
        local_time = datetime.now(sri_lanka_tz)

        # Insert prediction into PredictedFoodRisk
        cursor.execute("""
            INSERT INTO PredictedFoodRisk (
                timestamp, temperature_dht11, humidity_dht11, gas1, predicted_risk
            ) VALUES (?, ?, ?, ?, ?)
        """,
            local_time,
            float(df['temperature_dht11'].iloc[0]),
            float(df['humidity_dht11'].iloc[0]),
            int(df['gas1'].iloc[0]),
            str(predicted_label)
        )

        conn.commit()
        cursor.close()
        conn.close()

        logging.info(f"✅ Prediction saved: {predicted_label} at {local_time}")

    except Exception as e:
        logging.error(f"❌ Error occurred: {str(e)}")
