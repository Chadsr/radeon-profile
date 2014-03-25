// copyright marazmista @ 29.12.2013

// this file contains functions for handle gui events, clicks and others

#include "radeon_profile.h"
#include "ui_radeon_profile.h"

#include <QTimer>
#include <QColorDialog>
#include <QClipboard>

bool closeFromTrayMenu;

//===================================
// === GUI events === //
// == menu forcePowerLevel
void radeon_profile::forceAuto() {
    setValueToFile(forcePowerLevelFilePath,"auto");
}

void radeon_profile::forceLow() {
    setValueToFile(forcePowerLevelFilePath,"low");
}

void radeon_profile::forceHigh() {
    setValueToFile(forcePowerLevelFilePath,"high");
}

// == buttons for forcePowerLevel
void radeon_profile::on_btn_forceAuto_clicked()
{
    forceAuto();
}

void radeon_profile::on_btn_forceHigh_clicked()
{
    forceHigh();
}

void radeon_profile::on_btn_forceLow_clicked()
{
    forceLow();
}

// == others
void radeon_profile::on_btn_dpmBattery_clicked() {
    setValueToFile(dpmStateFilePath,"battery");
}

void radeon_profile::on_btn_dpmBalanced_clicked() {
    setValueToFile(dpmStateFilePath,"balanced");
}

void radeon_profile::on_btn_dpmPerformance_clicked() {
    setValueToFile(dpmStateFilePath,"performance");
}

void radeon_profile::resetMinMax() { minT = 0; maxT = 0; }

void radeon_profile::changeTimeRange() {
    rangeX = ui->timeSlider->value();
}

void radeon_profile::on_cb_showFreqGraph_clicked(bool checked)
{
    ui->plotColcks->setVisible(checked);
}

void radeon_profile::on_cb_showTempsGraph_clicked(bool checked)
{
    ui->plotTemp->setVisible(checked);
}

void radeon_profile::on_cb_showVoltsGraph_clicked(bool checked)
{
    ui->plotVolts->setVisible(checked);
}

void radeon_profile::resetGraphs() {
        ui->plotColcks->yAxis->setRange(startClocksScaleL,startClocksScaleH);
        ui->plotVolts->yAxis->setRange(startVoltsScaleL,startVoltsScaleH);
        ui->plotTemp->yAxis->setRange(10,20);
}

void radeon_profile::showLegend(bool checked) {
        ui->plotColcks->legend->setVisible(checked);
        ui->plotVolts->legend->setVisible(checked);
        ui->plotColcks->replot();
        ui->plotVolts->replot();
}

void radeon_profile::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange && ui->cb_minimizeTray->isChecked()) {
        if(isMinimized())
            this->hide();

        event->ignore();
    }
    if (event->type() == QEvent::Close && ui->cb_closeTray) {
        this->hide();
        event->ignore();
    }
}

void radeon_profile::gpuChanged()
{
    switch (cardDriver) {
    case XORG: {
        figureOutGPUDataPaths(ui->combo_gpus->currentText()); // resolve paths for newly selected card

        // do initial stuff once again for new card
        testSensor();
        getModuleInfo();
        getPowerMethod();
        getCardConnectors();
        break;
    }
    case FGLRX: {
        timerEvent();
        break;
    }
    }
}

void radeon_profile::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger: {
        if (isHidden()) {
            this->setWindowFlags(Qt::Window);
            showNormal();
        } else hide();
        break;
    }
    }
}

void radeon_profile::closeEvent(QCloseEvent *e) {
    if (ui->cb_closeTray->isChecked() && !closeFromTrayMenu) {
        this->hide();
        e->ignore();
    }
    saveConfig();
}

void radeon_profile::closeFromTray() {
    closeFromTrayMenu = true;
    this->close();
}

void radeon_profile::on_spin_lineThick_valueChanged(int arg1)
{
    setupGraphsStyle();
}

void radeon_profile::on_spin_timerInterval_valueChanged(double arg1)
{
    timer->setInterval(arg1*1000);
}

void radeon_profile::on_cb_graphs_clicked(bool checked)
{
    ui->mainTabs->setTabEnabled(1,checked);
}

void radeon_profile::on_cb_gpuData_clicked(bool checked)
{
    ui->cb_graphs->setEnabled(checked);
    ui->cb_stats->setEnabled(checked);

    if (ui->cb_stats->isChecked())
        ui->tabs_systemInfo->setTabEnabled(3,checked);

    if (ui->cb_graphs->isChecked())
        ui->mainTabs->setTabEnabled(1,checked);

    if (!checked) {
        ui->list_currentGPUData->clear();
        ui->list_currentGPUData->addItem("GPU data is disabled.");
    }
}

void radeon_profile::refreshBtnClicked() {
    switch (cardDriver) {
    case XORG: {
        getGLXInfo();
        getCardConnectors();
        getModuleInfo();
        break;
    }
    case FGLRX: {
        fglrxGetGLXInfo();
        break;
    }
    }


}

void radeon_profile::on_graphColorsList_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QColor c = QColorDialog::getColor(item->backgroundColor(1));
    if (c.isValid()) {
        item->setBackgroundColor(1,c);
        // apply colors
        setupGraphsStyle();
    }
}

void radeon_profile::on_cb_stats_clicked(bool checked)
{
    ui->tabs_systemInfo->setTabEnabled(3,checked);

    // reset stats data
    statsTickCounter = 1;
    if (!checked)
        resetStats();
}

void radeon_profile::copyGlxInfoToClipboard() {
    QString clip;
    for (int i = 0; i < ui->list_glxinfo->count(); i++)
        clip += ui->list_glxinfo->item(i)->text() + "\n";

    QApplication::clipboard()->setText(clip);
}

void radeon_profile::resetStats() {
    statsTickCounter = 1;
    pmStats.clear();
    ui->list_stats->clear();
}
//========
