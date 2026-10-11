# BME280 Data Platform

A Flask web application for visualizing and analyzing BME280 sensor data (temperature, humidity, pressure, altitude) with real-time offset correction.

## Features

- **5 dashboard views** — overview, temperature, humidity, pressure, multi-sensor comparison
- **Live offset correction** — adjust temperature and humidity offsets with sliders; all charts update instantly
- **CSV upload** — load your own sensor data from a CSV file
- **CSV export** — download corrected data with a single click
- **Statistics** — mean, min, max, std dev, median per channel

## Project Structure

```
bme280-platform/
├── app.py                 # Flask backend & API routes
├── templates/
│   └── index.html         # Single-page frontend (Plotly charts)
├── requirements.txt
├── .gitignore
└── README.md
```

## Quick Start

```bash
# 1. Clone the repo
git clone https://github.com/YOUR_USERNAME/bme280-platform.git
cd bme280-platform

# 2. Create and activate a virtual environment
python -m venv venv
source venv/bin/activate        # Windows: venv\Scripts\activate

# 3. Install dependencies
pip install -r requirements.txt

# 4. Run the app
python app.py
```

Open [http://localhost:5000](http://localhost:5000) in your browser.

## CSV Format

Upload files with these columns (header row required):

| Column | Description |
|---|---|
| `Timestamp(ms)` | Millisecond timestamp |
| `Temperature(C)` | Raw temperature in °C |
| `Humidity(%)` | Raw relative humidity |
| `Pressure(hPa)` | Atmospheric pressure |
| `Altitude(m)` | Altitude in metres |

Lowercase column names (`timestamp`, `temperature`, etc.) are also accepted.

## API Endpoints

| Method | Route | Description |
|---|---|---|
| GET | `/api/data` | Corrected data as JSON |
| GET | `/api/stats` | Statistics per channel |
| GET | `/api/charts/<sheet>` | Plotly chart JSON for a tab |
| GET | `/api/export` | Download corrected CSV |
| POST | `/api/upload` | Upload a CSV file |
| POST | `/api/reset` | Reset to built-in sample data |

Query parameters `temp_offset` (default `159.67`) and `humidity_offset` (default `-55`) are accepted by all GET endpoints.

## Offset Correction

The BME280 inside an enclosure reads high due to self-heating. Default offsets:

- **Temperature**: subtract 159.67 °C from raw reading
- **Humidity**: add −55 % to raw reading (clamped 0–100)

Adjust these with the sliders in the UI or pass them as query parameters.

## License

MIT
