#ifndef QZAPWIDGET_H
#define QZAPWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "visualiser.hpp"

class QZapWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    QZapWidget(QWidget* parent=nullptr);
    virtual ~QZapWidget();

    bool is_initialised() const { return context_initialised_; }

    visualiser* get_visualiser() const;
    void set_visualiser(visualiser* vis_ptr);

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

signals:
    void onInitialized();

protected:

private:
    bool context_initialised_;
    visualiser* vis_ptr_;
};

#endif // QZAPWIDGET_H
