// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/kevaaddkeydialog.h>
#include <qt/forms/ui_kevaaddkeydialog.h>

#include <qt/kevatablemodel.h>
#include <qt/kevadialog.h>

#include <QPushButton>

KevaAddKeyDialog::KevaAddKeyDialog(QWidget *parent, QString &nameSpace) :
    QDialog(parent),
    ui(new Ui::KevaAddKeyDialog)
{
    ui->setupUi(this);
    this->nameSpace = nameSpace;
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(create()));
    connect(ui->keyText, SIGNAL(textChanged(const QString &)), this, SLOT(onKeyChanged(const QString &)));
    connect(ui->valueText, SIGNAL(textChanged()), this, SLOT(onValueChanged()));
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

KevaAddKeyDialog::~KevaAddKeyDialog()
{
    delete ui;
}

void KevaAddKeyDialog::create()
{
    KevaDialog* dialog = (KevaDialog*)this->parentWidget();
    std::string keyText  = ui->keyText->text().toStdString();
    std::string valueText  = ui->valueText->toPlainText().toStdString();
    std::string ns = nameSpace.toStdString();
    if (!dialog->addKeyValue(ns, keyText, valueText)) {
        QDialog::close();
        return;
    }
    dialog->showNamespace(nameSpace);
    QDialog::close();
}

void KevaAddKeyDialog::cancel()
{
    QDialog::close();
}

void KevaAddKeyDialog::onKeyChanged(const QString& key)
{
    // Calculate current length
    int keyTextLength   = key.length();
    int valueTextLength = ui->valueText->toPlainText().length();

    // Update counter value
    ui->keyCounter->setText(
        QString::number(
            keyTextLength
        ) + "/" + QString::number(
            255
        )
    );

    // Update counter palette
    QPalette keyCounterPalette = ui->keyCounter->palette();

    keyCounterPalette.setColor(
        QPalette::WindowText,
        keyTextLength > 255 ? Qt::red : Qt::darkGreen
    );

    ui->keyCounter->setPalette(
        keyCounterPalette
    );

    // Update button status
    bool enabled = keyTextLength > 0    && valueTextLength > 0 &&
                   keyTextLength <= 255 && valueTextLength < MAX_SCRIPT_ELEMENT_SIZE + 1;

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(enabled);
}

void KevaAddKeyDialog::onValueChanged()
{
    // Calculate current length
    int keyTextLength   = ui->keyText->text().length();
    int valueTextLength = ui->valueText->toPlainText().length();

    // Update counter value
    ui->valueCounter->setText(
        QString::number(
            valueTextLength
        ) + "/" + QString::number(
            MAX_SCRIPT_ELEMENT_SIZE
        )
    );

    // Update counter palette
    QPalette valueCounterPalette = ui->valueCounter->palette();

    valueCounterPalette.setColor(
        QPalette::WindowText,
        valueTextLength > MAX_SCRIPT_ELEMENT_SIZE ? Qt::red : Qt::darkGreen
    );

    ui->valueCounter->setPalette(
        valueCounterPalette
    );

    // Update button status
    bool enabled = keyTextLength > 0    && valueTextLength > 0 &&
                   keyTextLength <= 255 && valueTextLength < MAX_SCRIPT_ELEMENT_SIZE + 1;

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(enabled);
}
