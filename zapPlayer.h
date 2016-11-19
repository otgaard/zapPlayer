#ifndef ZAPPLAYER_H
#define ZAPPLAYER_H

#include <QDialog>
#include <zapAudio/base/audio_output.hpp>

namespace Ui {
class zapPlayer;
}

class zapPlayer : public QDialog {
    Q_OBJECT

public:
    explicit zapPlayer(QWidget* parent=nullptr);
    ~zapPlayer();

public slots:
    void openFile();
    void play();
    void stop();
    void pause();

private:
    Ui::zapPlayer* ui;
    audio_output_s16 output_;
    QString filename_;
};

#endif // ZAPPLAYER_H
