#include <QDial>
#include <QDebug>
#include <QSpinBox>
#include <QFileDialog>
#include "zapPlayer.h"
#include "ui_zapPlayer.h"
#include "analyser_stream.hpp"
#include <zapAudio/base/mp3_stream.hpp>
#include <zapAudio/base/sine_wave.hpp>
#include <zapAudio/base/buffered_stream.hpp>

zapPlayer::zapPlayer(QWidget *parent) : QDialog(parent), ui(new Ui::zapPlayer), output_(nullptr,2,44100,1024) {
    ui->setupUi(this);

    connect(ui->btnOpenFile, &QPushButton::clicked, this, &zapPlayer::openFile);
    connect(ui->btnPlay, &QPushButton::clicked, this, &zapPlayer::play);
    connect(ui->btnStop, &QPushButton::clicked, this, &zapPlayer::stop);
    connect(ui->btnSineWave, &QPushButton::clicked, this, &zapPlayer::sineWave);
    connect(ui->spinHertz, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &zapPlayer::changeHertz);
    connect(ui->dialHertz, static_cast<void(QDial::*)(int)>(&QDial::valueChanged), this, &zapPlayer::changeHertz);

    // We want to implement a pulse that feeds the FFT bins to the visualiser
    connect(&sync_, &QTimer::timeout, this, &zapPlayer::sync);

    ui->openGLWidget->set_visualiser(&visualiser_);

}

zapPlayer::~zapPlayer() {
    output_.stop();
    delete ui;
}

void zapPlayer::changeHertz(int hertz) {
    if(auto ptr = dynamic_cast<sine_wave<short>*>(streams_[0])) {
        if(ui->spinHertz->value() != hertz) ui->spinHertz->setValue(hertz);
        if(ui->dialHertz->value() != hertz) ui->dialHertz->setValue(hertz);
        ptr->set_hertz(hertz);
    }
}

void zapPlayer::showEvent(QShowEvent* event) {
    qDebug() << "showEvent";
    QDialog::showEvent(event);
}

void zapPlayer::openFile() {
    filename_ = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), QDir::homePath(),
        tr("MP3 Files (*.mp3)"));
}

void zapPlayer::play() {
    if(output_.is_playing() || output_.is_paused()) output_.stop();

    if(output_.get_stream()) {
        // Shut down and clean up the old stream
        delete streams_[0];
        delete streams_[1];
        delete streams_[2];
    }

    // This is the file I/O stream that converts the file into samples
    auto stream_ptr = new mp3_stream(filename_.toStdString(), 1024, nullptr);
    if(!stream_ptr->start()) {
        qDebug() << "Error starting mp3_stream";
        return;
    }

    // This is a buffering stream to prevent I/O blocking interfering with audio output
    auto buffer_ptr = new buffered_stream<short>(64*1024, 32*1024, 60, stream_ptr);
    if(!buffer_ptr->start()) {
        qDebug() << "Error starting buffering stream";
        return;
    }

    // This is the FFT stream that produces the FFT transform just before sending the data to audio_output
    auto fft_ptr = new analyser_stream(buffer_ptr);

    streams_[0] = stream_ptr;
    streams_[1] = buffer_ptr;
    streams_[2] = fft_ptr;

    output_.set_stream(fft_ptr);

    output_.play();
    sync_.start(1.f/60*1000);
}

void zapPlayer::sineWave() {
    if(output_.is_playing() || output_.is_paused()) output_.stop();

    if(output_.get_stream()) {
        // Shut down and clean up the old stream
        delete streams_[0];
        delete streams_[1];
        delete streams_[2];
    }

    auto stream_ptr = new sine_wave<short>(ui->spinHertz->value());

    // This is a buffering stream to prevent I/O blocking interfering with audio output
    auto buffer_ptr = new buffered_stream<short>(64*1024, 32*1024, 60, stream_ptr);
    if(!buffer_ptr->start()) {
        qDebug() << "Error starting buffering stream";
        return;
    }

    // This is the FFT stream that produces the FFT transform just before sending the data to audio_output
    auto fft_ptr = new analyser_stream(buffer_ptr);

    streams_[0] = stream_ptr;
    streams_[1] = buffer_ptr;
    streams_[2] = fft_ptr;

    output_.set_stream(fft_ptr);

    output_.play();
    sync_.start(1.f/60*1000);
}

void zapPlayer::stop() {
    sync_.stop();
    output_.stop();
}

void zapPlayer::pause() {

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
