# 🌬️ Aerodynamics Research Lab

A high-school project notebook: build a wind tunnel, learn from measurements, and explore an upcoming CubeSat idea.

| Start here | What you will find | Status |
| --- | --- | --- |
| [🌬️ Wind tunnel](01_wind_tunnel_design/README.md) | Parts, sensor code, airflow ideas, and next steps | Build files and six preliminary videos available; quantitative validation pending |
| [🛰️ CubeSat](02_cubesat/README.md) | A simple mission-planning space | Upcoming; mission and hardware undecided |
| [📚 Research papers & future plans](03_research/README.md) | Reading notes, paper ideas, and small next steps | No papers committed yet |

## The lab at a glance

```mermaid
mindmap
  root((Research Lab))
    Wind tunnel
      Build and sketch
      Record sensor readings
      Check airflow
      Share photos and graphs
    Upcoming CubeSat
      Pick a question
      Sketch the systems
      Try a ground prototype
    Research and future plans
      Read and explain
      Collect evidence
      Write a short paper
```

**Text map:** wind tunnel → build and measure; CubeSat → plan and prototype; research → read, explain, and share evidence.

## How air could move through the tunnel

This is a **proposed suction-type layout**, not a verified drawing of the current build. Dimensions, fan position, and final assembly still need to be recorded.

```mermaid
flowchart LR
  A["Room air"] --> B["Honeycomb / straightener"]
  B --> C["Contraction<br/>narrows the passage"]
  C --> D["Test section<br/>model and measurements"]
  D --> E["Diffuser<br/>widens the passage"]
  E --> F["Fan with guard"]
  F --> G["Air out"]
```

**Airflow in words:** air enters → gets straighter → speeds up in a narrower passage → passes the test model → slows in a wider passage → leaves through the fan.

The honeycomb helps reduce sideways flow. The test section is where we observe the air and record measurements. [Explore the parts and measurement ideas →](01_wind_tunnel_design/README.md)

## What is actually here?

- [3D-print files](01_wind_tunnel_design/design/3D_parts/) for tunnel components, honeycomb parts, and supports.
- [Arduino sensor code](01_wind_tunnel_design/code/src-arduino/) and a differential-pressure simulation.
- A [CSV graphing tool](01_wind_tunnel_design/code/feature-csv-data-grapher/README.md) with example data.
- A [data-correction app](01_wind_tunnel_design/code/feature-data-correction/README.md). Its setup and correction values need checking before use.

## Wind tunnel: initial tests and video evidence

Six supplied MOV clips document the physical setup and preliminary flow visualization, including a mounted model car and visible fog. They are **qualitative preliminary evidence**, not calibrated aerodynamic measurements.

- [Initial test record and limitations](01_wind_tunnel_design/tests/README.md)
- [Six-video index, previews, and downloads](01_wind_tunnel_design/tests/videos/README.md)
- [Repeatable test-log template](01_wind_tunnel_design/tests/test-log-template.md)

Test dates, fan settings, dimensions, calibration records, and synchronized sensor data were not supplied with these clips. No airspeed, lift, drag, Reynolds number, or flow-uniformity results are claimed. Example and simulated readings elsewhere in the repository remain demonstration data.

## Next small steps

1. Add a labeled build photo and the real tunnel dimensions.
2. Check sensor readings against a reference, then save a short test log.
3. Pick one CubeSat question and draw a first system sketch.
4. Explain one useful paper in five sentences.

[Research ideas and paper space →](03_research/README.md)

[Visit AeroAspirant](https://www.aeroaspirant.org/) · [MIT license](LICENSE)

*Structure refreshed: October 11, 2026.*
