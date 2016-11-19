#include <QDebug>
#include <QFileDialog>
#include "zapPlayer.h"
#include "ui_zapPlayer.h"
#include <zapAudio/base/mp3_stream.hpp>

zapPlayer::zapPlayer(QWidget *parent) : QDialog(parent), ui(new Ui::zapPlayer), output_(nullptr) {
    ui->setupUi(this);

    connect(ui->btnOpenFile, &QPushButton::clicked, this, &zapPlayer::openFile);
    connect(ui->btnPlay, &QPushButton::clicked, this, &zapPlayer::play);
    connect(ui->btnStop, &QPushButton::clicked, this, &zapPlayer::stop);
}

zapPlayer::~zapPlayer() {
    output_.stop();
    delete ui;
}

void zapPlayer::openFile() {
    filename_ = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), QDir::homePath(),
        tr("MP3 Files (*.mp3)"));
}

void zapPlayer::play() {
    if(output_.get_stream()) {
        // Shut down and clean up the old stream
        delete output_.get_stream();
    }

    auto stream_ptr = new mp3_stream(filename_.toStdString(), 1024, nullptr);
    if(!stream_ptr->start()) {
        qDebug() << "Error starting mp3_stream";
        return;
    }

    output_.set_stream(stream_ptr);

    output_.play();
}

void zapPlayer::stop() {
    output_.stop();
}

void zapPlayer::pause() {

}