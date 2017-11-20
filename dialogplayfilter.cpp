#include "dialogplayfilter.h"
#include "ui_dialogplayfilter.h"

DialogPlayFilter::DialogPlayFilter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlayFilter)
{
    ui->setupUi(this);


    QStringList packetTypes;
    packetTypes.append("Phone (t1)");
    packetTypes.append("Radio (t2)");
    packetTypes.append("Terminal (t3)");
    packetTypes.append("Record (t0:1)");
    packetTypes.append("IntTermCommon (t0:2)");
    packetTypes.append("IntTermRadio (t0:3)");
    packetTypes.append("Permanent (t0:4)");
    packetTypes.append("TermExtensions (t0:5)");
    packetTypes.append("DivertGroups (t0:6)");

    foreach (QString entry, packetTypes)
    {
        chPacketTypes.append(new QCheckBox(entry));
        ui->groupType->layout()->addWidget(chPacketTypes.last());
        //chPacketTypes.last()->setEnabled(false);
        chPacketTypes.last()->setChecked(true);
        connect(chPacketTypes.last(),SIGNAL(toggled(bool)),this,SLOT(TypeSelected()));
    }


    //Init lan check
    QStringList netLabels;
    netLabels.append(tr("LAN A"));
    netLabels.append(tr("LAN B"));
    foreach (QString entry, netLabels)
    {
        chNets.append(new QCheckBox(entry));
        ui->groupLan->layout()->addWidget(chNets.last());
        //chNets.last()->setEnabled(false);
        chNets.last()->setChecked(true);
        connect(chNets.last(),SIGNAL(toggled(bool)),this,SLOT(NetChanged()));
    }
    QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);


    ui->groupLan->layout()->addItem(spacer);



}

DialogPlayFilter::~DialogPlayFilter()
{
    delete ui;
}


void DialogPlayFilter::TypeSelected()
{

}

void DialogPlayFilter::NetChanged()
{

}

