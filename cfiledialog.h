#ifndef CFILEDIALOG_H
#define CFILEDIALOG_H


#include <QApplication>

#include <QFileDialog>
class CFileDialog : public QWidget
{

public:
    CFileDialog();

    void openFile();
    void openFiles();
    void openDir();

    QString SaveFileAsPrm(const QString &dir);
    QString OpenPrmFile(const QString &dir);
    QStringList DeletePrmFiles(const QString &dir);

};




#endif // CFILEDIALOG_H
