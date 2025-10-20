#ifndef DRAWINGCANVAS_H
#define DRAWINGCANVAS_H

#include <QWidget>
#include <QVector>
#include <QPoint>
#include <QRect>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include <unordered_map>

struct CustomMatrix {
    bool val[3][3];
    CustomMatrix() { memset(val, 0, sizeof(val)); }
};

class DrawingCanvas : public QWidget {
    Q_OBJECT
public:
    explicit DrawingCanvas(QWidget *parent = nullptr);

    // Main function
    void clearPoints();         // clear every points in canvas
    void paintLines();          // draw line from points
    void segmentDetection();    // segmen detector

protected:
    // For draw and input mouse
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QVector<QPoint> m_points;        // points to draw
    bool isPaintLinesClicked = false;

    // Canvas scale
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Detector members
    QVector<QRect> candidateRects;   // rectangles to draw (purple)
    int candidateRectSize = 6;       // size of purple rectangle

    // Parameters
    int min_occurrence = 2;          // frequency threshold for ideal windows
};

#endif // DRAWINGCANVAS_H
