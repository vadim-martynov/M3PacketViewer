#include "cfiledialog.h"

CFileDialog::CFileDialog()
{

}

void CFileDialog::openFile()
{
    QFileDialog::getOpenFileName(
                this,
                tr("Open Document"),
                QDir::currentPath(),
                tr("Document files (*.doc *.rtf);;All files (*.*)"), 0, QFileDialog::DontUseNativeDialog );

    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open Document"),
                QDir::currentPath(),
                tr("Document files (*.doc *.rtf);;All files (*.*)") );
    if( !filename.isNull() )
    {
        qDebug( filename.toLocal8Bit() );
    }
}

void CFileDialog::openFiles()
{
    QStringList filenames = QFileDialog::getOpenFileNames(
                this,
                tr("Open Document"),
                QDir::currentPath(),
                tr("Documents (*.doc);;All files (*.*)") );
    if( !filenames.isEmpty() )
    {
        qDebug( filenames.join(",").toLocal8Bit() );
    }
}

void CFileDialog::openDir()
{
    QString dirname = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Directory"),
                QDir::currentPath() );
    if( !dirname.isNull() )
    {
        qDebug( dirname.toLocal8Bit() );
    }
}

QString CFileDialog::OpenPrmFile(const QString &dir)
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open Record Map"),
                dir,
                tr("Packet Record Map (*.prm)"), 0, QFileDialog::DontUseNativeDialog );
    return fileName;
}

QString CFileDialog::SaveFileAsPrm(const QString &dir)
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Record Map"),
                dir,
                tr("Packet Record Map (*.prm)"), 0, QFileDialog::DontUseNativeDialog );
    return fileName;
}

QStringList CFileDialog::DeletePrmFiles(const QString &dir)
{
    QStringList filenames = QFileDialog::getOpenFileNames(
                this,
                tr("Select record maps for delete"),
                dir,
                tr("Packet Record Map (*.prm)"), 0, QFileDialog::DontUseNativeDialog );
    return filenames;
}
