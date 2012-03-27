#ifndef DLGPIZAMODEL_H
#define DLGPIZAMODEL_H

#include <QDialog>

namespace Ui {
    class DlgPizaModelBuilder;
}

class DlgPizaModelBuilder : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPizaModelBuilder(QWidget *parent = 0);
    ~DlgPizaModelBuilder();

    void setValues(int levels, int pillars, float radius, float height);
    void getValues(int& levels, int& pillars,
                   int& xTowers, int& yTowers,
                   float& radius, float& height);

public slots:
    void preview();
signals:
    void sig_preview(int levels, int pillars,
                     int xTowers, int yTowers,
                     float radius, float height);
private:
    Ui::DlgPizaModelBuilder *ui;
};

#endif // DLGPIZAMODEL_H
