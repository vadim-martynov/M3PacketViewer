#ifndef DIALOGPLAYFILTER_H
#define DIALOGPLAYFILTER_H

#include <QDialog>
#include <QList>
#include <QString>
#include <QCheckBox>

namespace Ui {
class DialogPlayFilter;
}

class DialogPlayFilter : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlayFilter(QWidget *parent = 0);
    ~DialogPlayFilter();

private slots:
    void TypeSelected();
    void NetChanged();

private:
    Ui::DialogPlayFilter *ui;



    QList<QCheckBox *> chPacketTypes;
    QList<QCheckBox *> chNets;


};

#endif // DIALOGPLAYFILTER_H
