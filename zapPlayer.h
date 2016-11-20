#ifndef ZAPPLAYER_H
#define ZAPPLAYER_H

#include <QDialog>
#include <zapAudio/base/audio_output.hpp>
#include <QTimer>
#include "visualiser.hpp"

namespace Ui {
class zapPlayer;
}

class zapPlayer : public QDialog {
    Q_OBJECT

public:
    explicit zapPlayer(QWidget* parent=nullptr);
    ~zapPlayer();

    virtual void showEvent(QShowEvent* event);

public slots:
    void openFile();
    void play();
    void stop();
    void pause();

    void sync();

private:
    Ui::zapPlayer* ui;
    audio_output_s16 output_;
    QString filename_;
    audio_stream<short>* streams_[3];
    QTimer sync_;
    visualiser visualiser_;
};

#endif // ZAPPLAYER_H
