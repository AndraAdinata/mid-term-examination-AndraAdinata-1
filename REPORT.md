# Segment Detection Report
 
**Date:** October 20, 2025  

---

## 🎯 Objectives
1. **Analyze** and find the correct window size for identifying a segment.  
2. **Report** all appropriate window patterns for identifying a segment (dump all non-empty windows).  
3. **Develop** an automated algorithm that detects all potential segment candidates and visualizes them on the canvas.

---

## 🧩 Summary of Findings
After testing and analyzing various window configurations, a **3×3 window** proved to be the most efficient for detecting small local structures such as lines, corners, and endpoints.

Each 3×3 window is represented as a **9-bit binary pattern**, where `1` indicates a non-white pixel.  
The system records all non-empty patterns and counts how often each appears.  
Frequent patterns with the center pixel set (`bit 4 = 1`) are considered **“ideal windows”**, representing likely segment features.

---

## 📊 Data Dump & Reporting
During execution, the program automatically produces:

1. **`report_patterns.csv`** – Contains all non-empty 3×3 patterns and their frequencies.  
   Columns: `pattern_bin`, `pattern_dec`, `count`, `layout`  
   Example layout: `0 1 0; 0 0 0; 0 0 0`

2. **`candidate_coords.csv`** – Contains the coordinates and pattern IDs of all detected candidates.

These files can be used for deeper analysis, verification, or retraining of segment detection heuristics.

---

## ⚙️ Algorithm Overview
The `segmentDetection()` function performs the following steps:

1. Capture the canvas using `grab()` and convert it to a `QImage`.  
2. Slide a **3×3 window** across every pixel (excluding image borders).  
3. Convert each local window into a 9-bit integer key (`0–511`).  
4. Store and count each non-empty pattern.  
5. Select **ideal patterns** based on:
   - Center pixel is set (`bit 4 = 1`).
   - Occurrence frequency ≥ configurable threshold (`min_occurrence = 2` by default).
6. Mark all matched positions with small **purple rectangles** on the canvas.  
7. Output the CSV reports for visualization and debugging.

---

## 🖼️ Visual Output
When `segmentDetection()` runs:
- Detected candidate areas are marked with **purple squares**.
- Blue dots and red lines remain as user-drawn input.
- You can visually inspect the detected windows directly on the canvas.

---

## 🧠 Rationale for 3×3 Window
- Captures the smallest meaningful neighborhood for local features.  
- Minimizes computational load while preserving structure details.  
- Can be easily extended to 5×5 or 7×7 by updating pattern encoding.

