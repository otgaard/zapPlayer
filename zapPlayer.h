#ifndef ZAPPLAYER_H
#define ZAPPLAYER_H

#include <QDialog>

namespace Ui {
class zapPlayer;
}

class zapPlayer : public QDialog
{
    Q_OBJECT

public:
    explicit zapPlayer(QWidget *parent = 0);
    ~zapPlayer();

private:
    Ui::zapPlayer *ui;
};

#endif // ZAPPLAYER_H
