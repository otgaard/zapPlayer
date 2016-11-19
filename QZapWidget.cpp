#include "QZapWidget.h"

QZapWidget::QZapWidget(QWidget* parent) : QOpenGLWidget(parent) {
}

QZapWidget::~QZapWidget() = default;

void QZapWidget::initializeGL() {
    QOpenGLFunctions::initializeOpenGLFunctions();
}

void QZapWidget::paintGL() {
    glClearColor(.25f, .25f, .25f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void QZapWidget::resizeGL(int w, int h) {

}

void QZapWidget::mousePressEvent(QMouseEvent* event) {

}

void QZapWidget::mouseMoveEvent(QMouseEvent* event) {

}

void QZapWidget::wheelEvent(QWheelEvent* event) {

}
