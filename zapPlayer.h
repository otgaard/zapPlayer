#ifndef ZAPPLAYER_H
#define ZAPPLAYER_H

#include <QDialog>
#include <zapAudio/audio_output.hpp>
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
    void openFolder();

    void play();
    void stop();
    void pause();
    void skip_track();

    void sync();
    void onNextTrack(const QString&);

    void volumeChanged(int volume);

    void onGLInit();
    void moduleChanged(const QString&);

private:
    Ui::zapPlayer* ui;

    audio_output_s16 audio_out_;

    QString path_;
    bool is_folder_;    // Is the path a folder or a file

    audio_stream<short>* streams_[4];
    visualiser visualiser_;

    QTimer sync_;
};

#endif // ZAPPLAYER_H
