#include <QDial>
#include <QDebug>
#include <QSpinBox>
#include <QFileDialog>
#include "zapPlayer.h"
#include "ui_zapPlayer.h"
#include "analyser_stream.hpp"
#include "directory_stream.hpp"
#include <zapAudio/base/mp3_stream.hpp>
#include <zapAudio/base/sine_wave.hpp>
#include <zapAudio/base/buffered_stream.hpp>

zapPlayer::zapPlayer(QWidget *parent) : QDialog(parent), ui(new Ui::zapPlayer), audio_out_(nullptr,2,44100,1024),
    visualiser_(128) {
    ui->setupUi(this);

    connect(ui->btnOpenFile, &QPushButton::clicked, this, &zapPlayer::openFile);
    connect(ui->btnPlay, &QPushButton::clicked, this, &zapPlayer::play);
    connect(ui->btnStop, &QPushButton::clicked, this, &zapPlayer::stop);
    connect(ui->btnSkip, &QPushButton::clicked, this, &zapPlayer::skip_track);

    // We want to implement a pulse that feeds the FFT bins to the visualiser
    connect(&sync_, &QTimer::timeout, this, &zapPlayer::sync);

    ui->openGLWidget->set_visualiser(&visualiser_);

}

zapPlayer::~zapPlayer() {
    audio_out_.stop();
    delete ui;
}

void zapPlayer::showEvent(QShowEvent* event) {
    qDebug() << "showEvent";
    QDialog::showEvent(event);
}

void zapPlayer::openFile() {
    /*
    filename_ = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), QDir::homePath(),
        tr("MP3 Files (*.mp3)"));
    */
    filename_ = QFileDialog::getExistingDirectory(nullptr, "Please select a directory to play", QDir::homePath());
}

void zapPlayer::play() {
    if(audio_out_.is_playing() || audio_out_.is_paused()) audio_out_.stop();

    if(audio_out_.get_stream()) {
        // Shut down and clean up the old stream
        delete streams_[0];
        delete streams_[1];
        delete streams_[2];
    }

    auto pathstream_ptr = new directory_stream(filename_.toStdString(), 1024);
    if(!pathstream_ptr->start()) {
        qDebug() << "Error starting directory_stream";
        return;
    }

    /*
    // This is the file I/O stream that converts the file into samples
    auto stream_ptr = new mp3_stream(filename_.toStdString(), 1024, nullptr);
    if(!stream_ptr->start()) {
        qDebug() << "Error starting mp3_stream";
        return;
    }
    */

    // This is a buffering stream to prevent I/O blocking interfering with audio output
    auto buffer_ptr = new buffered_stream<short>(64*1024, 32*1024, 60, pathstream_ptr);
    if(!buffer_ptr->start()) {
        qDebug() << "Error starting buffering stream";
        return;
    }

    // This is the FFT stream that produces the FFT transform just before sending the data to audio_output
    auto fft_ptr = new analyser_stream(buffer_ptr);

    streams_[0] = pathstream_ptr;
    streams_[1] = buffer_ptr;
    streams_[2] = fft_ptr;

    audio_out_.set_stream(fft_ptr);

    audio_out_.play();
    sync_.start(1.f/60*1000);
}

void zapPlayer::stop() {
    sync_.stop();
    audio_out_.stop();
}

void zapPlayer::pause() {

}

void zapPlayer::skip_track() {
    if(auto ptr = dynamic_cast<directory_stream*>(streams_[0])) {
        ptr->skip_track();
    }
}

void zapPlayer::sync() {
    analyser_stream::fft_buffer_t buf(128);
    auto ptr = static_cast<analyser_stream*>(streams_[2]);
    auto len = ptr->copy_bins(buf, 128);
    if(len != 128) {
        qDebug() << "Mismatch";
    }

    visualiser_.set_frequency_bins(buf);
    visualiser_.update(0,0);
    ui->openGLWidget->update();
}
