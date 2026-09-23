#include "userinfopage.h"
#include "ui_userinfopage.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include "imagecropperdialog.h"
#include "imagecropperlabel.h"
UserInfoPage::UserInfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserInfoPage)
{
    ui->setupUi(this);
}

UserInfoPage::~UserInfoPage()
{
    delete ui;
}

void UserInfoPage::on_upload_btn_clicked()
{
    QString filename=QFileDialog::getOpenFileName(this,tr("Select a picture"),QString(),tr("Picture (*.jpg *.png *.jpeg 8.bmp *.webp)"));

    if(filename.isEmpty())return;

    QPixmap logo;
    if(!(logo.load(filename))){
        QMessageBox::critical(
            this,
            tr("Image Load Error"),
            tr("Error , Please reselect or select other file"),
            QMessageBox::Ok);
        return;
    }

    QPixmap image=ImageCropperDialog::getCroppedImage(filename,600,400,CropperShape::CIRCLE);
    if(!image.isNull()){
        return;
    }

    QPixmap scaledpixmap=image.scaled(ui->head_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->head_lb->setPixmap(scaledpixmap);
    ui->head_lb->setScaledContents(true);

    QString storageDir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(storageDir);
    if(!dir.exists("avatars")){
        if(!dir.mkdir("avatars")){
            qWarning()<<"Error in reading file";
            QMessageBox::warning(
                this,
                tr("Error"),
                tr("Please check permission and disk space")
                );
            return;
        }
    }

    QString path=dir.filePath("avatars/head.png");
    if(!scaledpixmap.save(path,"PNG")){
        QMessageBox::warning(
            this,
            tr("Error"),
            tr("Logo Save Error"));

    }
    else{
        qDebug()<<"Logo save to "<<path;
    }



}

