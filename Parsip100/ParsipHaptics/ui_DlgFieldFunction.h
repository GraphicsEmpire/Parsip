/********************************************************************************
** Form generated from reading UI file 'DlgFieldFunction.ui'
**
** Created: Wed Sep 7 15:54:40 2011
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGFIELDFUNCTION_H
#define UI_DLGFIELDFUNCTION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgFieldFunctionEditor
{
public:
    QDialogButtonBox *buttonBox;
    QTabWidget *tabWidget;
    QWidget *tsFieldFunction;
    QScrollArea *scrollAreaOpenGL;
    QWidget *scrollAreaWidgetContents;
    QWidget *tsActions;
    QPushButton *btnRunAll;
    QPushButton *btnRunNext;
    QPushButton *btnClearAll;
    QListWidget *lstActions;
    QPushButton *btnDelete;
    QPushButton *btnStop;
    QLabel *lblRun;
    QPushButton *btnOpen;
    QWidget *tsBlobTree;
    QScrollArea *scrollAreaBlobTree;
    QWidget *scrollAreaWidgetContents_3;

    void setupUi(QDialog *DlgFieldFunctionEditor)
    {
        if (DlgFieldFunctionEditor->objectName().isEmpty())
            DlgFieldFunctionEditor->setObjectName(QString::fromUtf8("DlgFieldFunctionEditor"));
        DlgFieldFunctionEditor->resize(574, 370);
        buttonBox = new QDialogButtonBox(DlgFieldFunctionEditor);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(150, 340, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        tabWidget = new QTabWidget(DlgFieldFunctionEditor);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(0, 0, 571, 341));
        tsFieldFunction = new QWidget();
        tsFieldFunction->setObjectName(QString::fromUtf8("tsFieldFunction"));
        scrollAreaOpenGL = new QScrollArea(tsFieldFunction);
        scrollAreaOpenGL->setObjectName(QString::fromUtf8("scrollAreaOpenGL"));
        scrollAreaOpenGL->setGeometry(QRect(0, 0, 481, 311));
        scrollAreaOpenGL->setMinimumSize(QSize(100, 100));
        scrollAreaOpenGL->setMouseTracking(false);
        scrollAreaOpenGL->setWidgetResizable(true);
        scrollAreaOpenGL->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 479, 309));
        scrollAreaOpenGL->setWidget(scrollAreaWidgetContents);
        tabWidget->addTab(tsFieldFunction, QString());
        tsActions = new QWidget();
        tsActions->setObjectName(QString::fromUtf8("tsActions"));
        btnRunAll = new QPushButton(tsActions);
        btnRunAll->setObjectName(QString::fromUtf8("btnRunAll"));
        btnRunAll->setGeometry(QRect(93, 289, 75, 23));
        btnRunNext = new QPushButton(tsActions);
        btnRunNext->setObjectName(QString::fromUtf8("btnRunNext"));
        btnRunNext->setGeometry(QRect(173, 289, 75, 23));
        btnClearAll = new QPushButton(tsActions);
        btnClearAll->setObjectName(QString::fromUtf8("btnClearAll"));
        btnClearAll->setGeometry(QRect(413, 289, 75, 23));
        lstActions = new QListWidget(tsActions);
        lstActions->setObjectName(QString::fromUtf8("lstActions"));
        lstActions->setGeometry(QRect(0, 0, 561, 281));
        lstActions->setSelectionMode(QAbstractItemView::MultiSelection);
        lstActions->setSelectionBehavior(QAbstractItemView::SelectRows);
        btnDelete = new QPushButton(tsActions);
        btnDelete->setObjectName(QString::fromUtf8("btnDelete"));
        btnDelete->setGeometry(QRect(333, 289, 75, 23));
        btnStop = new QPushButton(tsActions);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        btnStop->setGeometry(QRect(253, 289, 75, 23));
        lblRun = new QLabel(tsActions);
        lblRun->setObjectName(QString::fromUtf8("lblRun"));
        lblRun->setGeometry(QRect(500, 290, 61, 20));
        btnOpen = new QPushButton(tsActions);
        btnOpen->setObjectName(QString::fromUtf8("btnOpen"));
        btnOpen->setGeometry(QRect(10, 288, 75, 23));
        tabWidget->addTab(tsActions, QString());
        tsBlobTree = new QWidget();
        tsBlobTree->setObjectName(QString::fromUtf8("tsBlobTree"));
        scrollAreaBlobTree = new QScrollArea(tsBlobTree);
        scrollAreaBlobTree->setObjectName(QString::fromUtf8("scrollAreaBlobTree"));
        scrollAreaBlobTree->setGeometry(QRect(0, 0, 481, 311));
        scrollAreaBlobTree->setMinimumSize(QSize(100, 100));
        scrollAreaBlobTree->setMouseTracking(false);
        scrollAreaBlobTree->setWidgetResizable(true);
        scrollAreaBlobTree->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        scrollAreaWidgetContents_3 = new QWidget();
        scrollAreaWidgetContents_3->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_3"));
        scrollAreaWidgetContents_3->setGeometry(QRect(0, 0, 479, 309));
        scrollAreaBlobTree->setWidget(scrollAreaWidgetContents_3);
        tabWidget->addTab(tsBlobTree, QString());

        retranslateUi(DlgFieldFunctionEditor);
        QObject::connect(buttonBox, SIGNAL(accepted()), DlgFieldFunctionEditor, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DlgFieldFunctionEditor, SLOT(reject()));

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(DlgFieldFunctionEditor);
    } // setupUi

    void retranslateUi(QDialog *DlgFieldFunctionEditor)
    {
        DlgFieldFunctionEditor->setWindowTitle(QApplication::translate("DlgFieldFunctionEditor", "Modify Field Function", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tsFieldFunction), QApplication::translate("DlgFieldFunctionEditor", "Field Function", 0, QApplication::UnicodeUTF8));
        btnRunAll->setText(QApplication::translate("DlgFieldFunctionEditor", "Run >>>", 0, QApplication::UnicodeUTF8));
        btnRunNext->setText(QApplication::translate("DlgFieldFunctionEditor", "Run Next", 0, QApplication::UnicodeUTF8));
        btnClearAll->setText(QApplication::translate("DlgFieldFunctionEditor", "Delete All", 0, QApplication::UnicodeUTF8));
        btnDelete->setText(QApplication::translate("DlgFieldFunctionEditor", "Delete", 0, QApplication::UnicodeUTF8));
        btnStop->setText(QApplication::translate("DlgFieldFunctionEditor", "Stop", 0, QApplication::UnicodeUTF8));
        lblRun->setText(QApplication::translate("DlgFieldFunctionEditor", "0000/0000", 0, QApplication::UnicodeUTF8));
        btnOpen->setText(QApplication::translate("DlgFieldFunctionEditor", "Open File", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tsActions), QApplication::translate("DlgFieldFunctionEditor", "Actions", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tsBlobTree), QApplication::translate("DlgFieldFunctionEditor", "BlobTree", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgFieldFunctionEditor: public Ui_DlgFieldFunctionEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGFIELDFUNCTION_H
