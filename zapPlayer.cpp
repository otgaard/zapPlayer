#include <QDial>
#include <QDebug>
#include <QSpinBox>
#include <QFileDialog>
#include "zapPlayer.h"
#include "ui_zapPlayer.h"
#include "analyser_stream.hpp"
#include "directory_stream.hpp"
#include "controller_stream.hpp"
#include <zapAudio/streams/mp3_stream.hpp>
#include <zapAudio/streams/sine_wave.hpp>
#include <zapAudio/streams/buffered_stream.hpp>

zapPlayer::zapPlayer(QWidget *parent) : QDialog(parent), ui(new Ui::zapPlayer), audio_out_(nullptr,2,44100,1024),
    visualiser_(128) {
    ui->setupUi(this);

    setWindowFlags(Qt::WindowStaysOnTopHint);
    //setWindowOpacity(0.5f);

    for(auto& s : streams_) s = nullptr;

    connect(ui->btnOpenFile, &QPushButton::clicked, this, &zapPlayer::openFile);
    connect(ui->btnOpenFolder, &QPushButton::clicked, this, &zapPlayer::openFolder);
    connect(ui->btnPlay, &QPushButton::clicked, this, &zapPlayer::play);
    connect(ui->btnStop, &QPushButton::clicked, this, &zapPlayer::stop);
    connect(ui->btnSkip, &QPushButton::clicked, this, &zapPlayer::skip_track);
    connect(ui->sldVolume, &QSlider::valueChanged, this, &zapPlayer::volumeChanged);
    connect(ui->btnPause, &QPushButton::clicked, this, &zapPlayer::pause);

    // We want to implement a pulse that feeds the FFT bins to the visualiser
    connect(&sync_, &QTimer::timeout, this, &zapPlayer::sync);

    if(ui->openGLWidget->is_initialised())
        onGLInit();
    else
        connect(ui->openGLWidget, &QZapWidget::onInitialized, this, &zapPlayer::onGLInit);

    connect(ui->cbxVisualisation, SIGNAL(currentIndexChanged(QString)), this, SLOT(moduleChanged(QString)));

    ui->txtFolder->setReadOnly(true);
    ui->txtFilename->setReadOnly(true);

    ui->openGLWidget->set_visualiser(&visualiser_);
    qDebug() << "Disable Fixed Path";
    path_ = "/Users/otgaard/Desktop/Test Music/test.mp3";
    is_folder_ = false;
    play();
}

zapPlayer::~zapPlayer() {
    audio_out_.stop();
    delete ui;
}

void zapPlayer::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
}

void zapPlayer::openFile() {
    path_ = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), QDir::homePath(),
        tr("MP3 Files (*.mp3)"));
    is_folder_ = false;
    ui->txtFilename->setText(path_);
}

void zapPlayer::openFolder() {
    path_ = QFileDialog::getExistingDirectory(nullptr, "Please select a directory to play", QDir::homePath());
    is_folder_ = true;
    ui->txtFolder->setText(path_);
}

void zapPlayer::play() {
    if(audio_out_.is_playing() || audio_out_.is_paused()) audio_out_.stop();

    if(audio_out_.get_stream()) {
        // Shut down and clean up the old stream
        delete streams_[0];
        delete streams_[1];
        delete streams_[2];
    }

    audio_stream<short>* sourcestream_ptr;

    if(is_folder_) {
        auto pathstream_ptr = new directory_stream(path_.toStdString(), 1024);

        pathstream_ptr->on_next_track([this](const std::string& filename) {
            QString file = filename.c_str();
            QMetaObject::invokeMethod(this, "onNextTrack", Qt::QueuedConnection, Q_ARG(QString, file));
        });

        if(!pathstream_ptr->start()) {
            qDebug() << "Error starting directory_stream";
            return;
        }
        sourcestream_ptr = pathstream_ptr;
    } else {
        auto filestream_ptr = new mp3_stream(path_.toStdString(), 1024, nullptr);
        if(!filestream_ptr->start()) {
            qDebug() << "Error starting mp3_stream";
            return;
        }
        sourcestream_ptr = filestream_ptr;
        ui->txtFilename->setText(path_);
    }

    // This is a buffering stream to prevent I/O blocking interfering with audio output
    auto buffer_ptr = new buffered_stream<short>(64*1024, 32*1024, 60, sourcestream_ptr);
    if(!buffer_ptr->start()) {
        qDebug() << "Error starting buffering stream";
        return;
    }

    // This is the FFT stream that produces the FFT transform just before sending the data to audio_output
    auto fft_ptr = new analyser_stream(buffer_ptr);

    // The Controller Stream (Panning, Volume, effects (reverb?)
    auto controller_ptr = new controller_stream<short>(fft_ptr, 44100, 2, 1024);

    streams_[0] = sourcestream_ptr;
    streams_[1] = buffer_ptr;
    streams_[2] = fft_ptr;
    streams_[3] = controller_ptr;

    audio_out_.set_stream(controller_ptr);

    audio_out_.play();
    sync_.start(0);
}

void zapPlayer::stop() {
    sync_.stop();
    audio_out_.stop();
}

void zapPlayer::pause() {
    audio_out_.pause();
    if(audio_out_.is_paused()) ui->btnPause->setText("Resume");
    else                       ui->btnPause->setText("Pause");
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
    visualiser_.update(0.f, .01f);
    ui->openGLWidget->update();
}

void zapPlayer::volumeChanged(int volume) {
    if(auto ptr = dynamic_cast<controller_stream<short>*>(streams_[3])) {
        ptr->set_volume(volume/100.f);
    }
}

void zapPlayer::onGLInit() {
    auto modules = visualiser_.get_visualisations();
    for(const auto& m : modules) ui->cbxVisualisation->addItem(m.c_str());
}

void zapPlayer::moduleChanged(const QString& module) {
    qDebug() << module;
    visualiser_.set_visualisation(module.toStdString());
}

void zapPlayer::onNextTrack(const QString& filename) {
    ui->txtFilename->setText(filename);
}
