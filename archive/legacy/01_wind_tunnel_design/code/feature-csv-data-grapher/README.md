# BME280 Data Grapher

Python script to create aesthetically pleasing graphs from BME280 CSV data using `matplotlib` + `seaborn`.

## Install

```powershell
pip install pandas matplotlib seaborn
```

## CSV format

Use columns like:

- `timestamp` (optional but recommended)
- `temperature_c` (or `temperature`, `temp_c`, `temp`)
- `humidity` (or `humidity_percent`, `rh`)
- `pressure_hpa` (or `pressure`, `press_hpa`)

Example:

```csv
timestamp,temperature_c,humidity,pressure_hpa
2026-03-04 10:00:00,23.4,41.2,1008.6
2026-03-04 10:01:00,23.5,41.0,1008.7
2026-03-04 10:02:00,23.6,40.9,1008.8
```

## Run

```powershell
python bme280_grapher.py sensor_data.csv -o output\bme280_dashboard.png
```

Optional:

```powershell
python bme280_grapher.py sensor_data.csv --title "Office Environment (BME280)"
```
