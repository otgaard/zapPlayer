#include <QDebug>
#include <third_party/include/zap/engine/engine.hpp>
#include "QZapWidget.h"

QZapWidget::QZapWidget(QWidget* parent) : QOpenGLWidget(parent), context_initialised_(false), vis_ptr_(nullptr) {
}

QZapWidget::~QZapWidget() = default;

void QZapWidget::initializeGL() {
    zap::engine::init();

    QOpenGLFunctions::initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);

    if(vis_ptr_ && !vis_ptr_->initialise()) {
        qDebug() << "Error initialising visualiser";
        return;
    }
    context_initialised_ = true;
    emit onInitialized();
}

visualiser* QZapWidget::get_visualiser() const {
    return vis_ptr_;
}

void QZapWidget::set_visualiser(visualiser* vis_ptr) {
    vis_ptr_ = vis_ptr;
    if(context_initialised_ && !vis_ptr_->is_initialised()) vis_ptr_->initialise();
}

void QZapWidget::paintGL() {
    glClearColor(.25f, .25f, .25f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(.5f);
    vis_ptr_->draw();
}

void QZapWidget::resizeGL(int w, int h) {
    vis_ptr_->resize(w, h);
}

void QZapWidget::mousePressEvent(QMouseEvent* event) {

}

void QZapWidget::mouseMoveEvent(QMouseEvent* event) {

}

void QZapWidget::wheelEvent(QWheelEvent* event) {

}
