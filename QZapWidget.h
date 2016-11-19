#ifndef QZAPWIDGET_H
#define QZAPWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class QZapWidget : public QOpenGLWidget, protected QOpenGLFunctions {
public:
    QZapWidget(QWidget* parent=nullptr);
    virtual ~QZapWidget();

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

protected:

private:

};

#endif // QZAPWIDGET_H
