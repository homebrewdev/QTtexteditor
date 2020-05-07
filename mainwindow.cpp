#include <QtWidgets>
#include "mainwindow.h"

MainWindow::MainWindow()
    : textEdit(new QPlainTextEdit)
{
    setCentralWidget(textEdit);

    setInit();
    createActions();
    createStatusBar();

    readSettings();

    connect(textEdit->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::documentWasModified);

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest,
            this, &MainWindow::commitData);
#endif

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::setInit(){
    setFont(QFont("Verdana", 12));
    // gray textEdit background
    QPalette p = textEdit->palette();
    p.setColor(QPalette::Base, Qt::lightGray);
    textEdit->setPalette(p);
    //set custom Window Icon
    const QIcon windowIcon = QIcon(":/images/appicon.png");
    setWindowIcon(windowIcon);
}

void MainWindow::closeEvent(QCloseEvent *event)

{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{

    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}
//! zoom the textEdit
void MainWindow::zoomIn()
{
    MainWindow::textEdit->zoomIn(1);
}
//! Zoom out textEdit
void MainWindow::zoomOut()
{
    MainWindow::textEdit->zoomOut(1);
}
//! Change Font Dialog
void MainWindow::changeFont()
{
    bool ok;
    QFont newFont = QFontDialog::getFont(
                    &ok, QFont("Verdana", 12), this);
    if (ok) {
        textEdit->setFont(newFont);
    } else {
        // for features, don`t change font
    }
    //QFontDialog::getFont(0, textEdit))   <-  if we want set font directly to Widget
}
//! Change background color
void MainWindow::changeColor()
{
    const QColor defaultColor = Qt::lightGray;
    // QColor color = QColorDialog::getColor();
    //if (!color.isValid()) return;
    QColorDialog::setStandardColor(0, defaultColor);
    QColorDialog *dialog = new QColorDialog(this);
    dialog->show();
    QObject::connect(dialog,&QDialog::accepted,[=]()
    {
        QColor newColor =  dialog->currentColor();
        QPalette p = textEdit->palette();
        p.setColor(QPalette::Base, newColor);
        textEdit->setPalette(p);
    });
}
    //newColor = QColorDialog::getColor(defaultColor, textEdit, "Select background color", QColorDialog::NoButtons);
    //if (!newColor.isValidColor()) {
    //    QPalette p = textEdit->palette();
    //    p.setColor(QPalette::Base, newColor);
    //    textEdit->setPalette(p);
    //} else {
    //    QPalette p = textEdit->palette();
     //   p.setColor(QPalette::Base, defaultColor);
    //    textEdit->setPalette(p);
    //}

//! for preferences menu
void MainWindow::preferences()
{
    QMessageBox::about(this, tr("About Application"),
             tr("The <b>Application</b> preferences "));
}

void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}

void MainWindow::createActions()
{

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);


    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as", QIcon(":/images/saveas.png"));
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    fileToolBar->addAction(saveAsAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);

    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("Cu&t"), this);

    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, &QAction::triggered, textEdit, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, &QAction::triggered, textEdit, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, textEdit, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar()->addSeparator();

    // my zoom in added
    const QIcon plusIcon = QIcon::fromTheme("zoom-in", QIcon(":/images/plus.png"));
    QAction *zoomInTextEdit = new QAction(plusIcon, tr("&Zoom In"), this);
    zoomInTextEdit->setShortcuts(QKeySequence::ZoomIn);
    zoomInTextEdit->setStatusTip(tr("Text zoom in"));
    connect(zoomInTextEdit, &QAction::triggered, this, &MainWindow::zoomIn);
    editMenu->addAction(zoomInTextEdit);
    editToolBar->addAction(zoomInTextEdit);

    //my zoomOut edit
    const QIcon minusIcon = QIcon::fromTheme("zoom-out", QIcon(":/images/minus.png"));
    QAction *zoomOutTextEdit = new QAction(minusIcon, tr("&Zoom Out"), this);
    zoomOutTextEdit->setShortcuts(QKeySequence::ZoomOut);
    zoomOutTextEdit->setStatusTip(tr("Text zoom out"));

    connect(zoomOutTextEdit, &QAction::triggered, this, &MainWindow::zoomOut);
    editMenu->addAction(zoomOutTextEdit);
    editToolBar->addAction(zoomOutTextEdit);

    // Font change tool bar button
    const QIcon newFont = QIcon::fromTheme("format-text-bold", QIcon(":/images/dialog.png"));
    QAction *newFontAction = new QAction(newFont, tr("&New Font"), this);
    newFontAction->setShortcuts(QKeySequence::Bold);
    newFontAction->setStatusTip(tr("Change Font Dialog"));

    connect(newFontAction, &QAction::triggered, this, &MainWindow::changeFont);
    editMenu->addAction(newFontAction);
    editToolBar->addAction(newFontAction);

    // Background color change tool bar button
    const QIcon newColor = QIcon::fromTheme("view-fullscreen", QIcon(":/images/palitra.png"));
    QAction *newColorAction = new QAction(newColor, tr("&Change Background color"), this);
    newColorAction->setStatusTip(tr("Select background color"));

    connect(newColorAction, &QAction::triggered, this, &MainWindow::changeColor);
    editMenu->addAction(newColorAction);
    editToolBar->addAction((newColorAction));

#endif // !QT_NO_CLIPBOARD

    // set pref menu tool bar
    QMenu *prefMenu = menuBar()->addMenu(tr("&Preferences"));
    QAction *onPreferences = prefMenu->addAction(tr("&Preferences"), this, &MainWindow::preferences);
    onPreferences->setStatusTip(tr("Adjust Preferences"));
    // help menu tool bar
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));


    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

#ifndef QT_NO_CLIPBOARD
    cutAct->setEnabled(false);

    copyAct->setEnabled(false);
    connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool MainWindow::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)

{
    return QFileInfo(fullFileName).fileName();
}

#ifndef QT_NO_SESSIONMANAGER
void MainWindow::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!maybeSave())
            manager.cancel();
    } else {
        // Non-interactive: save without asking
        if (textEdit->document()->isModified())
            save();
    }
}

#endif
