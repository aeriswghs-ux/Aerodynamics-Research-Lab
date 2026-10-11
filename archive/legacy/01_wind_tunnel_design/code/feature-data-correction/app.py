"""
BME280 Professional Data Platform - Python Flask Application
Converts sensor data (temperature, humidity, pressure, altitude) into interactive visualizations
"""

import os
import json
import numpy as np
import pandas as pd
from datetime import datetime
from io import BytesIO, StringIO
import plotly.graph_objects as go
from flask import Flask, request, jsonify, send_file
from flask_cors import CORS

app = Flask(__name__)
CORS(app)
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB max file size

# Global data storage
raw_data = []

# Default sample data
SAMPLE_DATA = [
    {'timestamp': 2443,  'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 4443,  'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 6443,  'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 8443,  'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 10443, 'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 12443, 'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 14443, 'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 16443, 'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 18443, 'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 20443, 'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 22443, 'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 3092,  'temp': 182.67, 'humidity': 100.00, 'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 5092,  'temp': 182.67, 'humidity': 92.27,  'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 7092,  'temp': 182.67, 'humidity': 92.27,  'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 9092,  'temp': 182.67, 'humidity': 92.27,  'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 11092, 'temp': 182.67, 'humidity': 92.27,  'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 13092, 'temp': 182.67, 'humidity': 92.27,  'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 15092, 'temp': 182.67, 'humidity': 92.27,  'pressure': 648.32, 'altitude': 3611.33},
    {'timestamp': 17092, 'temp': 182.67, 'humidity': 92.27,  'pressure': 648.32, 'altitude': 3611.33},
]


def initialize_data():
    global raw_data
    raw_data = SAMPLE_DATA.copy()


def get_corrected_data(temp_offset=159.67, humidity_offset=-55):
    corrected = []
    for idx, d in enumerate(raw_data):
        corrected.append({
            'idx': idx,
            'timestamp': d['timestamp'],
            'temp': d['temp'],
            'humidity': d['humidity'],
            'pressure': d['pressure'],
            'altitude': d['altitude'],
            'corrected_temp': d['temp'] - temp_offset,
            'corrected_humidity': max(0, min(100, d['humidity'] + humidity_offset))
        })
    return corrected


def calculate_stats(data, key):
    values = [d[key] for d in data if isinstance(d.get(key), (int, float))]
    if not values:
        return {'mean': 0, 'min': 0, 'max': 0, 'std': 0, 'median': 0}
    values_array = np.array(values)
    return {
        'mean':   float(np.mean(values_array)),
        'min':    float(np.min(values_array)),
        'max':    float(np.max(values_array)),
        'std':    float(np.std(values_array)),
        'median': float(np.median(values_array))
    }


def _hex_to_rgba(hex_color, alpha=1.0):
    hex_color = hex_color.lstrip('#')
    if len(hex_color) != 6:
        return f'rgba(0, 0, 0, {alpha})'
    r, g, b = int(hex_color[0:2], 16), int(hex_color[2:4], 16), int(hex_color[4:6], 16)
    return f'rgba({r}, {g}, {b}, {alpha})'


def create_line_chart(title, x_vals, y_vals, color='#00b4ff'):
    fig = go.Figure()
    fig.add_trace(go.Scatter(
        x=x_vals, y=y_vals,
        mode='lines+markers', name=title,
        line=dict(color=color, width=2),
        fill='tozeroy', fillcolor=_hex_to_rgba(color, 0.15),
        marker=dict(size=5, opacity=0.7),
        hovertemplate=f'<b>{title}</b><br>Value: %{{y:.2f}}<extra></extra>'
    ))
    fig.update_layout(
        plot_bgcolor='#151a21', paper_bgcolor='#151a21',
        font=dict(color='#e0e4e8', family='system-ui'),
        margin=dict(t=10, b=30, l=50, r=20),
        hovermode='x unified',
        xaxis=dict(gridcolor='#2d3139', showgrid=True, zeroline=False),
        yaxis=dict(gridcolor='#2d3139', showgrid=True, zeroline=False),
        showlegend=False, height=400
    )
    return fig


def create_comparison_chart(data, x_vals):
    fig = go.Figure()
    fig.add_trace(go.Scatter(
        x=x_vals, y=[d['corrected_temp'] for d in data],
        mode='lines', name='Temperature',
        line=dict(color='#ff6b6b', width=2),
        fill='tozeroy', fillcolor='rgba(255, 107, 107, 0.15)'
    ))
    fig.add_trace(go.Scatter(
        x=x_vals, y=[d['corrected_humidity'] for d in data],
        mode='lines', name='Humidity',
        line=dict(color='#4ecdc4', width=2),
        fill='tozeroy', fillcolor='rgba(78, 205, 196, 0.15)',
        yaxis='y2'
    ))
    fig.add_trace(go.Scatter(
        x=x_vals, y=[d['pressure'] for d in data],
        mode='lines', name='Pressure',
        line=dict(color='#95e1d3', width=2, dash='dash'),
        yaxis='y3'
    ))
    fig.update_layout(
        plot_bgcolor='#151a21', paper_bgcolor='#151a21',
        font=dict(color='#e0e4e8'),
        margin=dict(t=10, b=30, l=50, r=80),
        hovermode='x unified',
        xaxis=dict(gridcolor='#2d3139'),
        yaxis=dict(title='Temp & Humidity', gridcolor='#2d3139'),
        yaxis2=dict(title='Humidity %', overlaying='y', side='right', range=[0, 100]),
        yaxis3=dict(title='Pressure', overlaying='y', side='right', anchor='free', position=0.85),
        height=500
    )
    return fig


# ── Routes ──────────────────────────────────────────────────────────────────

@app.route('/')
def index():
    return send_file(os.path.join(os.path.dirname(__file__), 'index.html'))


@app.route('/api/data')
def get_data():
    temp_offset     = float(request.args.get('temp_offset', 159.67))
    humidity_offset = float(request.args.get('humidity_offset', -55))
    corrected_data  = get_corrected_data(temp_offset, humidity_offset)
    return jsonify({'data': corrected_data, 'count': len(corrected_data)})


@app.route('/api/stats')
def get_stats():
    temp_offset     = float(request.args.get('temp_offset', 159.67))
    humidity_offset = float(request.args.get('humidity_offset', -55))
    data            = get_corrected_data(temp_offset, humidity_offset)
    return jsonify({
        'temperature':  calculate_stats(data, 'corrected_temp'),
        'humidity':     calculate_stats(data, 'corrected_humidity'),
        'pressure':     calculate_stats(data, 'pressure'),
        'data_points':  len(data)
    })


@app.route('/api/charts/<sheet>')
def get_charts(sheet):
    temp_offset     = float(request.args.get('temp_offset', 159.67))
    humidity_offset = float(request.args.get('humidity_offset', -55))
    data   = get_corrected_data(temp_offset, humidity_offset)
    x_vals = [f'S{i}' for i in range(len(data))]
    charts_data = {}

    if sheet == 'overview':
        for key, color, label in [
            ('corrected_temp',     '#ff6b6b', '🌡️ Temperature (Corrected)'),
            ('corrected_humidity', '#4ecdc4', '💧 Humidity (Corrected)'),
            ('pressure',           '#95e1d3', '🔽 Pressure'),
            ('altitude',           '#f38181', '📍 Altitude'),
        ]:
            fig = create_line_chart(label, x_vals, [d[key] for d in data], color)
            charts_data[f'chart{list(["corrected_temp","corrected_humidity","pressure","altitude"]).index(key)+1}'] = json.loads(fig.to_json())

    elif sheet in ('temperature', 'humidity', 'pressure'):
        key_map = {'temperature': ('corrected_temp', '#ff6b6b'), 'humidity': ('corrected_humidity', '#4ecdc4'), 'pressure': ('pressure', '#95e1d3')}
        key, color = key_map[sheet]
        fig = create_line_chart(sheet.capitalize(), x_vals, [d[key] for d in data], color)
        charts_data['chart1'] = json.loads(fig.to_json())

    elif sheet == 'comparison':
        fig = create_comparison_chart(data, x_vals)
        charts_data['chart1'] = json.loads(fig.to_json())

    return jsonify(charts_data)


@app.route('/api/export')
def export_csv():
    temp_offset     = float(request.args.get('temp_offset', 159.67))
    humidity_offset = float(request.args.get('humidity_offset', -55))
    data = get_corrected_data(temp_offset, humidity_offset)
    csv_data = 'Timestamp(ms),Temperature(C),Humidity(%),Pressure(hPa),Altitude(m)\n'
    for d in data:
        csv_data += f"{d['timestamp']},{d['corrected_temp']:.2f},{d['corrected_humidity']:.2f},{d['pressure']},{d['altitude']}\n"
    buf = BytesIO(csv_data.encode('utf-8'))
    buf.seek(0)
    return send_file(buf, mimetype='text/csv', as_attachment=True,
                     download_name=f"BME280_corrected_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv")


@app.route('/api/upload', methods=['POST'])
def upload_csv():
    global raw_data
    if 'file' not in request.files:
        return jsonify({'error': 'No file provided'}), 400
    file = request.files['file']
    if not file.filename or not file.filename.endswith('.csv'):
        return jsonify({'error': 'File must be a CSV'}), 400
    try:
        df = pd.read_csv(StringIO(file.stream.read().decode('utf-8')))
        aliases = {
            'Timestamp(ms)': 'timestamp', 'timestamp': 'timestamp',
            'Temperature(C)': 'temp',    'temperature': 'temp',
            'Humidity(%)': 'humidity',   'humidity': 'humidity',
            'Pressure(hPa)': 'pressure', 'pressure': 'pressure',
            'Altitude(m)': 'altitude',   'altitude': 'altitude',
        }
        df = df.rename(columns=aliases)
        required = ['timestamp', 'temp', 'humidity', 'pressure', 'altitude']
        missing  = [c for c in required if c not in df.columns]
        if missing:
            return jsonify({'error': f'Missing columns: {missing}'}), 400
        raw_data = df[required].dropna().to_dict('records')
        return jsonify({'success': True, 'message': f'Loaded {len(raw_data)} data points', 'data_points': len(raw_data)})
    except Exception as e:
        return jsonify({'error': str(e)}), 400


@app.route('/api/reset', methods=['POST'])
def reset_data():
    global raw_data
    raw_data = SAMPLE_DATA.copy()
    return jsonify({'success': True})


if __name__ == '__main__':
    initialize_data()
    app.run(debug=True, host='localhost', port=5000)
