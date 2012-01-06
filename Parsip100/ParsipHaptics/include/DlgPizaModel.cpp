#include "DlgPizaModel.h"
#include "ui_DlgPizaModel.h"

DlgPizaModelBuilder::DlgPizaModelBuilder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgPizaModelBuilder)
{
    ui->setupUi(this);

    connect(ui->btnPreview, SIGNAL(clicked()), this, SLOT(preview()));
}

DlgPizaModelBuilder::~DlgPizaModelBuilder()
{
    delete ui;
}


void DlgPizaModelBuilder::setValues(int levels, int pillars, float radius, float height)
{
    ui->sbLevels->setValue(levels);
    ui->sbPillars->setValue(pillars);
    ui->sbRadius->setValue(radius);
    ui->sbHeight->setValue(height);
}

void DlgPizaModelBuilder::getValues(int& levels, int& pillars, float& radius, float& height)
{
    levels = ui->sbLevels->value();
    pillars = ui->sbPillars->value();
    radius = ui->sbRadius->value();
    height = ui->sbHeight->value();
}

void DlgPizaModelBuilder::preview()
{
    emit sig_preview(ui->sbLevels->value(), ui->sbPillars->value(), ui->sbRadius->value(), ui->sbHeight->value());
}
