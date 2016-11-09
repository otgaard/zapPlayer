#include "zapPlayer.h"
#include "ui_zapPlayer.h"

zapPlayer::zapPlayer(QWidget *parent) : QDialog(parent), ui(new Ui::zapPlayer) {
    ui->setupUi(this);
}

zapPlayer::~zapPlayer() {
    delete ui;
}
