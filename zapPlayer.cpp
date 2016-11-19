#include <QDebug>
#include <QFileDialog>
#include "zapPlayer.h"
#include "ui_zapPlayer.h"
#include <zapAudio/base/mp3_stream.hpp>

zapPlayer::zapPlayer(QWidget *parent) : QDialog(parent), ui(new Ui::zapPlayer), output_(nullptr) {
    ui->setupUi(this);

    connect(ui->btnOpenFile, &QPushButton::clicked, this, &zapPlayer::openFile);
}

zapPlayer::~zapPlayer() {
    output_.stop();
    delete ui;
}

void zapPlayer::openFile() {
    auto filename = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), QDir::homePath(),
        tr("MP3 Files (*.mp3)"));
    if(!filename.isEmpty()) {
        auto stream_ptr = new mp3_stream(filename.toStdString(), 1024, nullptr);
        if(!stream_ptr->start()) {
            qDebug() << "Error starting mp3_stream";
            return;
        }

        output_.set_stream(stream_ptr);
        output_.play();
    }
}

void zapPlayer::play() {

}

void zapPlayer::stop() {

}

void zapPlayer::pause() {

}