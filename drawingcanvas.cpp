#include "drawingcanvas.h"
#include <QApplication>
#include <QPixmap>
#include <QDebug>
#include <fstream>

DrawingCanvas::DrawingCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumSize(this->WINDOW_WIDTH, this->WINDOW_HEIGHT);
    setStyleSheet("background-color: white; border: 1px solid gray;");
}

void DrawingCanvas::clearPoints() {
    m_points.clear();
    candidateRects.clear();
    update();
}

void DrawingCanvas::paintLines() {
    isPaintLinesClicked = true;
    update();
}

// Helper: convert 3x3 local boolean to 9-bit int (row-major)
static int patternFromWindow(bool w[3][3]) {
    int pat = 0;
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            int bit = w[r][c] ? 1 : 0;
            int idx = r * 3 + c; // bit position (0..8)
            pat |= (bit << idx);
        }
    }
    return pat;
}

void DrawingCanvas::segmentDetection() {
    QPixmap pixmap = this->grab();
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);

    qDebug() << "Image width:" << image.width();
    qDebug() << "Image height:" << image.height();

    std::unordered_map<int, int> patternCount;
    std::unordered_map<int, std::vector<QPoint>> patternLocations;

    // Iterate excluding 1-pixel border
    for (int i = 1; i < image.width() - 1; ++i) {
        for (int j = 1; j < image.height() - 1; ++j) {
            bool local_window[3][3] = {false};
            bool anySet = false;

            for (int m = -1; m <= 1; m++) {
                for (int n = -1; n <= 1; n++) {
                    QRgb rgbValue = image.pixel(i + m, j + n);
                    bool set = (rgbValue != 0xffffffff); // white = background
                    local_window[m + 1][n + 1] = set;
                    anySet = anySet || set;
                }
            }

            if (!anySet) continue; // skip empty windows

            int pat = patternFromWindow(local_window);
            patternCount[pat] += 1;
            patternLocations[pat].push_back(QPoint(i, j));
        }
    }

    // Dump CSV of patterns
    {
        std::ofstream ofs("report_patterns.csv");
        ofs << "pattern_bin,pattern_dec,count,layout\n";
        for (auto &kv : patternCount) {
            int pat = kv.first;
            int cnt = kv.second;
            // render binary string
            std::string bin = "";
            for (int b = 0; b < 9; b++) bin += ((pat >> b) & 1) ? '1' : '0';

            // Layout 3x3
            char layout[64] = {0};
            int idx = 0;
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    layout[idx++] = ((pat >> (r * 3 + c)) & 1) ? '1' : '0';
                    layout[idx++] = (c == 2 && r < 2) ? ';' : ' ';
                }
            }
            layout[idx] = 0;

            ofs << bin << "," << pat << "," << cnt << ",\"" << layout << "\"\n";
        }
        ofs.close();
    }

    // Select ideal patterns by heuristics:
    std::vector<int> idealPatterns;
    for (auto &kv : patternCount) {
        int pat = kv.first;
        int cnt = kv.second;
        // Center bit index = 4
        bool centerSet = ((pat >> 4) & 1); 
        if (!centerSet) continue; // only center-set windows
        if (cnt >= min_occurrence) idealPatterns.push_back(pat);
    }

    // Build candidate rect list from chosen patterns
    candidateRects.clear();
    for (int pat : idealPatterns) {
        auto &locs = patternLocations[pat];
        for (const QPoint &p : locs) {
            int half = candidateRectSize / 2;
            QRect r(p.x() - half, p.y() - half, candidateRectSize, candidateRectSize);
            candidateRects.append(r);
        }
    }

    // Save coordinates of candidates for review
    {
        std::ofstream ofs("candidate_coords.csv");
        ofs << "x,y,pattern_dec\n";
        for (int pat : idealPatterns) {
            for (const QPoint &p : patternLocations[pat]) {
                ofs << p.x() << "," << p.y() << "," << pat << "\n";
            }
        }
        ofs.close();
    }

    update(); // Trigger repaint to show purple rectangles
}

void DrawingCanvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw points
    QPen pen(Qt::blue, 5);
    painter.setPen(pen);
    painter.setBrush(QBrush(Qt::blue));
    for (const QPoint &point : std::as_const(m_points)) {
        painter.drawEllipse(point, 3, 3);
    }

    // Draw lines
    if (isPaintLinesClicked) {
        pen.setColor(Qt::red);
        pen.setWidth(4);
        painter.setPen(pen);
        for (int i = 0; i < m_points.size() - 1; i += 2) {
            painter.drawLine(m_points[i], m_points[i + 1]);
        }
        isPaintLinesClicked = false;
    }

    // Draw purple candidate rectangles
    if (!candidateRects.isEmpty()) {
        QPen p2(Qt::magenta);
        p2.setWidth(1);
        painter.setPen(p2);
        painter.setBrush(Qt::NoBrush);
        for (const QRect &r : std::as_const(candidateRects)) {
            painter.drawRect(r);
        }
    }
}

void DrawingCanvas::mousePressEvent(QMouseEvent *event) {
    m_points.append(event->pos());
    update();
}
