# Initial wind tunnel tests

[← Wind tunnel](../README.md) · [Video evidence](videos/README.md) · [Future test-log template](test-log-template.md)

**Status: qualitative preliminary evidence; quantitative validation pending.**

This record covers six supplied MOV clips: IMG_0866, IMG_0878, IMG_0879, IMG_0880, IMG_0894, and IMG_0819. It documents the visible build and initial flow-visualization work without assigning measured performance to the tunnel. Documentation added November 6, 2026; this is not an asserted test date.

## Test-photo gallery

The [test-photo gallery](photos/README.md) contains 23 supplied HEIC files with JPEG previews, grouped separately from the [20 fabrication photos](../design/photos/README.md). These are additional qualitative views; they do not establish separate trials or measured results. Filenames shared with MOV clips do not establish a synchronized capture.

## Questions addressed by the footage

- What does the physical test section and model support look like?
- How is the model car positioned and filmed?
- What visible fog patterns can be recorded for later investigation?

## Preliminary observation record

These are descriptive evidence groups, **not six verified independent trials**. Filenames do not establish chronology, repeatability, or a controlled change in conditions. Observations below are based on sampled frames at approximately 0%, 20%, 40%, 60%, 80%, and 98% of each clip; this is not a frame-by-frame motion analysis.

| Evidence group | Clips | Visible observations | Limits |
| --- | --- | --- | --- |
| Assembly and support | IMG_0866 | Clear-sided test section, white support on a round base, and a perforated panel are visible. | No scale, fan setting, calibrated instrument reading, or verified airflow direction is established. |
| Model and recording setup | IMG_0878, IMG_0879, IMG_0880, IMG_0894 | A small model car sits on the support; a phone on a tripod films the section. Wider views show the surrounding enclosure; IMG_0880 also shows external fans. | Camera position and lighting vary. These clips do not establish controlled comparison conditions or fan operating settings. |
| Fog visualization | IMG_0819 | Early sampled frames are heavily obscured by fog. Later frames show a more localized horizontal band of fog near the model/support; the car becomes clearer toward the end. | Fog concentration, lighting, reflections, and camera perspective limit interpretation. The footage does not quantify velocity or demonstrate laminar flow, a boundary layer, separation, or a measured wake. |

See the [video index](videos/README.md) for individual descriptions, approximate clip durations, previews, and original-file links. Clip duration is media metadata, not a verified steady-state test duration.

## What is not yet recorded

Test date/time, operator, run order, tunnel dimensions, model dimensions and orientation, fan models/settings, airflow direction, fog source and injection method, sensor placement, calibration/reference records, ambient conditions, and synchronized raw measurements were not supplied with the clips. Do not infer them from filenames or repository design proposals.

There are **no measured airspeed, pressure-difference, lift, drag, Reynolds-number, or flow-uniformity results in this record**. No numerical pass/fail criteria were supplied, so none of these observations is labeled a performance pass. The proposed suction-type layout on the lab home page remains a proposal, not a verified schematic of this footage.

The existing example CSV, simulated differential-pressure behavior, and example challenge narratives are not measurements from these videos. BME280 environmental readings alone do not provide lift, drag, or a spatial airspeed survey.

## Next tests — planned, not completed

1. **Document the build.** Add a labeled photograph with dimensions, airflow direction, fan/guard arrangement, model orientation, and sensor positions. Record the actual configuration rather than assuming it matches the proposed diagram.
2. **Establish a baseline.** Record fan-off readings and reference-instrument checks, with units, sensor identifiers, and calibration uncertainty. Keep raw data separate from corrected data.
3. **Repeat visualization under controlled conditions.** Fix the camera, lighting, model position, and fog introduction. Record fan settings and timestamps, change one factor at a time, and repeat each condition. Log obscuration or other limitations.
4. **Survey airspeed.** Use a suitable calibrated measurement method at documented positions in the test section. Define acceptance criteria before collecting data; retain individual readings and uncertainty before summarizing uniformity.
5. **Evaluate aerodynamic loads only with suitable instrumentation.** Establish the measurement method, tare procedure, model reference dimensions, and uncertainty before reporting lift or drag.

Use the [test-log template](test-log-template.md) for each future run, linking its raw files to the corresponding video timestamps. Secure the model and loose parts, keep fan guards in place, and switch power off before adjustments.

## Evidence preservation

The six `-muted.mov` files are preserved byte-for-byte as supplied. Their names are retained; no audio has been restored or inferred. Preview JPEGs are resized stills for navigation only. File sizes and SHA-256 checksums are in [the manifest](videos/manifest.json) and [SHA256SUMS](videos/SHA256SUMS). Video binaries use Git LFS; see [download instructions](videos/README.md#retrieving-the-originals).
