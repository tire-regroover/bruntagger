#include "bruntagger.h"
#include "mp3.h"
#include "tagsmodel.h"
#include "picsmodel.h"
#include "itemdelegates.h"

#include <string>
#include <boost/regex.hpp>
#include <kmimetype.h>

#include <QtGui>
#include <QDebug>

namespace bruntagger {

const int bruntagger::kStatusTimeout = 30000;


void bruntagger::restoreDefaultState() {}

void bruntagger::restoreDefaultHeader() {
    QHeaderView *hheader = ui.tableView->horizontalHeader();

    hheader->moveSection(hheader->visualIndex(TagsModel::Artist), 0);
    hheader->moveSection(hheader->visualIndex(TagsModel::Date), 1);
    hheader->moveSection(hheader->visualIndex(TagsModel::Disc), 2);
    hheader->moveSection(hheader->visualIndex(TagsModel::Album), 3);
    hheader->moveSection(hheader->visualIndex(TagsModel::Track), 4);
    hheader->moveSection(hheader->visualIndex(TagsModel::Title), 5);
    hheader->moveSection(hheader->visualIndex(TagsModel::Pics), 6);
    hheader->moveSection(hheader->visualIndex(TagsModel::Path), 7);

    hheader->hideSection(TagsModel::File);
    hheader->hideSection(TagsModel::BitRate);
    hheader->hideSection(TagsModel::Ext);
    hheader->hideSection(TagsModel::Modified);
    hheader->hideSection(TagsModel::Genre);
    hheader->hideSection(TagsModel::Composer);
    hheader->hideSection(TagsModel::HasId3V1);
    hheader->hideSection(TagsModel::Length);

    hheader->setSortIndicator(TagsModel::Path, Qt::AscendingOrder);
    auto_resize_cols_ = true;
}


bruntagger::bruntagger(QWidget *parent) :
    QMainWindow(parent) {
    ui.setupUi(this);
    applyStyle();

    first_show_ = true;
    auto_resize_cols_ = false;

    prev_dirs_index_ = 0;

    ui.actionResetView->setIcon(QIcon::fromTheme("view-file-columns"));
    ui.actionExit->setIcon(QIcon::fromTheme("application-exit"));
    ui.actionAbout->setIcon(QIcon::fromTheme("help-about"));
    ui.actionOpenDir->setIcon(QIcon::fromTheme("document-open-folder"));
    ui.actionBack->setIcon(QIcon::fromTheme("go-previous"));
    ui.actionBack->setEnabled(false);
    ui.actionForward->setIcon(QIcon::fromTheme("go-next"));
    ui.actionForward->setEnabled(false);
    ui.actionUpDir->setIcon(QIcon::fromTheme("go-up"));
    ui.actionToggleRecurse->setIcon(QIcon::fromTheme("view-list-tree"));
    ui.actionToggleShowDirs->setIcon(QIcon::fromTheme("view-multiple-objects"));
    ui.actionChangeDir->setIcon(QIcon::fromTheme("go-down"));
    ui.actionRefresh->setIcon(QIcon::fromTheme("view-refresh"));
    ui.actionClear->setIcon(QIcon::fromTheme("edit-clear-list"));
    ui.actionRegex->setIcon(QIcon::fromTheme("edit-find-replace"));
    ui.actionSave->setIcon(QIcon::fromTheme("document-save"));
    ui.actionDiscardChanges->setIcon(QIcon::fromTheme("document-revert"));
    ui.actionFilter->setIcon(QIcon::fromTheme("view-filter"));
    ui.actionEditPresets->setIcon(QIcon::fromTheme("document-edit"));
    ui.actionStop->setIcon(QIcon::fromTheme("process-stop"));
    ui.actionStop->setVisible(false);


    ui.actionLaunchFolder->setIcon(QIcon::fromTheme("document-open-folder"));
    ui.actionExtractPic->setIcon(QIcon::fromTheme("document-save"));

    ui.toolBarDir->insertSeparator(ui.actionToggleRecurse);
    ui.toolBarDir->insertWidget(ui.actionChangeDir, ui.lineDir);
    ui.toolBarDir->insertWidget(ui.actionChangeDir, ui.comboFileFilter);

    insertToolBarBreak(ui.toolBarFilter);

    ui.toolBarFilter->insertWidget(ui.actionFilter, ui.lineFilter);
    ui.toolBarFilter->insertWidget(ui.actionFilter, ui.comboFilterTarget);

    ui.toolBarReplace->insertWidget(ui.actionEditPresets, ui.comboPresets);
    ui.toolBarReplace->insertSeparator(ui.actionClear);
    ui.toolBarReplace->insertWidget(ui.actionRegex, ui.comboInput);
    ui.toolBarReplace->insertWidget(ui.actionRegex, ui.comboFind);
    ui.toolBarReplace->insertWidget(ui.actionRegex, ui.comboReplace);
    ui.toolBarReplace->insertWidget(ui.actionRegex, ui.comboRegexTarget);
    ui.toolBarReplace->insertSeparator(ui.actionSave);


    ui.splitterMainV->setCollapsible(0, false);
    ui.splitterMainV->setCollapsible(1, true);


    splash_ = new QSplashScreen(QPixmap(), Qt::Dialog);
    splash_->setWindowModality(Qt::WindowModal);



    model_ = new TagsModel(this);

    proxy_model_ = new QSortFilterProxyModel(this);
    proxy_model_->setSourceModel(model_);
    proxy_model_->setDynamicSortFilter(true);
    proxy_model_->setFilterCaseSensitivity(Qt::CaseInsensitive);


    ui.tableView->setModel(proxy_model_);

    pics_model_ = new PicsModel(this);
    ui.picsView->setModel(pics_model_);



    ui.tableView->setItemDelegateForColumn(TagsModel::Length, new DurationDelegate(this));
    ui.tableView->setItemDelegateForColumn(TagsModel::Modified, new DateTimeDelegate(this));

    QDirModel *dir_model = new QDirModel(ui.lineDir);
    dir_model->setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    dir_model->setSorting(QDir::DirsFirst);

    QStringList file_filters;
    file_filters << "*.mp3" << "*";

    QCompleter *dir_completer = new QCompleter(dir_model, ui.lineDir);
    dir_completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui.lineDir->setCompleter(dir_completer);

    ui.comboFileFilter->addItems(file_filters);

    ui.comboFilterTarget->addItems(model_->columns());

    QStringList inputs(model_->columns());
    inputs.replaceInStrings(QRegExp("^(.*)$"), "%\\1");
    for (QStringList::iterator it = inputs.begin(); it != inputs.end(); ++it)
        *it = it->toLower().replace(" ", "");
    inputs.append("%row");
    inputs.sort();
    inputs.insert(0, "");

    QStringList finds;
    finds.insert(0, "");

    QStringList replaces;
    replaces.insert(0, "");
    replaces << "\\1" << "\\2" << "\\3" << "\\4" << "\\5";

    ui.comboInput->addItems(inputs);
    ui.comboFind->addItems(finds);
    ui.comboReplace->addItems(replaces);


    QStringList regex_targets;
    for (int i = 0; i < model_->columnCount(); i++)
        if (!model_->read_only_columns().contains(i))
            regex_targets.append(model_->column(i));

    ui.comboRegexTarget->addItems(regex_targets);

    addPreset(
        "organize",
        QDir::homePath() + QDir::separator() + "Music" + QDir::separator()
                + "%artist - %date - %album{ (Disc %disc)}" + QDir::separator() + "%track - %title.%ext",
        "/(\\d[^\\d].*)",
        "/0\\1",
        TagsModel::Path);

    addPreset(
        "fix track",
        "%track",
        "([^/]*).*",
        "\\1",
        TagsModel::Track);

    ui.progressBar->setFixedSize(120, 16);
    ui.progressBar->setRange(0, 0);
    ui.progressBar->hide();
    ui.statusbar->addPermanentWidget(ui.progressBar);


    loadSettings();


    ui.comboPresets->addItem("");
    for (int i = presets_.count() - 1; i >= 0; i--) {
        QList<QVariant> preset = presets_.at(i);
        ui.comboPresets->insertItem(1, preset[0].toString());
    }

    for (int i = 0; i < model_->columnCount(); i++) {
        QAction *action = new QAction(tr("Show %1").arg(model_->column(i)), this);
        action->setChecked(!ui.tableView->horizontalHeader()->isSectionHidden(i));
        action->setProperty("column", i);
        header_actions_.append(action);
        connect(action, SIGNAL(triggered()), this, SLOT(showColumn()));
    }


    connect(ui.lineDir, SIGNAL(returnPressed()), ui.actionChangeDir, SLOT(trigger()));
    connect(ui.tableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &))
            , this, SLOT(selectedRowChanged(const QModelIndex &, const QModelIndex &)));
    connect(ui.tableView->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)),
            this, SLOT(hheader_doubleClicked()));
    connect(ui.tableView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
            this, SLOT(hheader_sectionResized()));
    connect(ui.tableView->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint)),
            this, SLOT(hheader_customContextMenuRequested(const QPoint)));
    connect(ui.picsView, SIGNAL(drawMe()), this, SLOT(showPic()));
    connect(ui.picsView, SIGNAL(unDrawMe()), this, SLOT(closePic()));


    QDir::setCurrent(ui.lineDir->text());

    setAcceptDrops(true);
}

bruntagger::~bruntagger() {
}

void bruntagger::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);
    model_->stop();
    saveSettings();
}

void bruntagger::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    switch (event->type()) {
        case QEvent::LanguageChange:
            ui.retranslateUi(this);
            break;
        default:
            break;
    }
}

void bruntagger::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    if (first_show_) {
        QList<int> old_sizes = ui.splitterMainV->sizes();
        int height = old_sizes.at(0) + old_sizes.at(1);

        QList<int> sizes;
        sizes << height - 166 << 166;

        ui.splitterMainV->setSizes(sizes);
        ui.lineDir->setFocus();

        first_show_ = false;
    }
}

void bruntagger::addPreset(QString title, QString input, QString find, QString replace, TagsModel::Column col) {
    QList<QVariant> preset;
    preset << title;
    preset << input;
    preset << find;
    preset << replace;
    preset << QVariant(col);
    presets_.append(preset);
}

void bruntagger::applyStyle() {
    QFile file(":/qss/bruntagger.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
}


void bruntagger::on_actionResetView_triggered() {
    restoreDefaultState();
    restoreDefaultHeader();
    ui.tableView->resizeColumnsToContents();
    auto_resize_cols_ = false;
}

void bruntagger::on_actionAboutQt_triggered() {
    QMessageBox::aboutQt(this);
}

void bruntagger::on_actionOpenDir_triggered() {
    QString path = QDir::currentPath();

    if (!ui.lineDir->text().isEmpty()) {
        QFileInfo file = QFileInfo(ui.lineDir->text());

        if (file.isDir() && file.exists())
            path = file.absoluteFilePath();
        else if (file.absoluteDir().exists())
            path = file.absolutePath();
    }

    QString selected_dir = QFileDialog::getExistingDirectory(this, tr("Select a folder"), path,
            QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);

    if (!selected_dir.isEmpty()) {
        changeDir(QDir::toNativeSeparators(selected_dir));
    }
}

void bruntagger::on_actionBack_triggered() {
    changeDir(--prev_dirs_index_);
}

void bruntagger::on_actionForward_triggered() {
    changeDir(++prev_dirs_index_);
}

void bruntagger::on_actionUpDir_triggered() {
    QDir dir = QDir::current();
    dir.cdUp();
    changeDir(QDir::toNativeSeparators(dir.absolutePath()));
}

void bruntagger::on_actionChangeDir_triggered() {
    QFileInfo file(ui.lineDir->text());

    if (file.exists()) {
        if (file.isFile()) {
            changeDir(QDir::toNativeSeparators(file.absolutePath()));
        } else if (file.isDir()) {
            changeDir(QDir::toNativeSeparators(file.absoluteFilePath()));
        }
    }
}

void bruntagger::on_actionRefresh_triggered() {
    changeDir(QDir::toNativeSeparators(QDir::currentPath()));
}



void bruntagger::on_lineFilter_textEdited(const QString &text) {
    Q_UNUSED(text);
    if (ui.actionFilter->isChecked())
        filter();
}

void bruntagger::on_lineFilter_returnPressed() {
    ui.actionFilter->setChecked(true);
}

void bruntagger::on_actionFilter_toggled(bool checked) {
    if (checked)
        filter();
    else
        clearFilter();
}

void bruntagger::on_comboPresets_currentIndexChanged(int index) {
    if (index > 0 && index <= presets_.count()) {
        QList<QVariant> preset = presets_.at(index - 1);
        ui.comboInput->lineEdit()->setText(preset[1].toString());
        ui.comboFind->lineEdit()->setText(preset[2].toString());
        ui.comboReplace->lineEdit()->setText(preset[3].toString());

        ui.comboRegexTarget->setCurrentIndex(
            ui.comboRegexTarget->findText(
                model_->column(preset[4].toInt())
            )
        );
    }
}

void bruntagger::on_actionEditPresets_triggered() {

}

void bruntagger::on_actionClear_triggered() {
    ui.comboInput->lineEdit()->setText(QString());
    ui.comboFind->lineEdit()->setText(QString());
    ui.comboReplace->lineEdit()->setText(QString());
}

void bruntagger::on_actionRegex_triggered() {
    proxy_model_->setDynamicSortFilter(false);

    QList<int> updated_rows;
    QModelIndexList selected_indexes = ui.tableView->selectionModel()->selectedIndexes();

    int selected_row = 0;
    for (int i = 0; i < selected_indexes.count(); i++) {
        QString target_text = ui.comboRegexTarget->currentText();
        int target_col = model_->columns().indexOf(target_text);
        QModelIndex index = proxy_model_->mapToSource(selected_indexes.at(i));
        QModelIndex target = model_->index(index.row(), target_col);

        if (!updated_rows.contains(index.row())) {
            Mp3 *mp3 = model_->mp3s().at(index.row());
            if (mp3->isMp3()) {
                QString input = ui.comboInput->lineEdit()->text();
                QString find = ui.comboFind->lineEdit()->text();
                QString replace = ui.comboReplace->lineEdit()->text();


                if (target_col == TagsModel::Path || target_col == TagsModel::File) {
                    input = input.replace(QDir::separator(), "\x01");
                }

                replaceMacros(input, mp3, selected_row);
                replaceMacros(find, mp3, selected_row);
                replaceMacros(replace, mp3, selected_row);

                if (target_col == TagsModel::Path || target_col == TagsModel::File) {
                    input = input.replace(QDir::separator(), "_");
                    input = input.replace("\x01", QDir::separator());
                }


                try {
                    QString result = QString::fromStdString(
                            boost::regex_replace(input.toStdString(), boost::regex(find.toStdString()), replace.toStdString()) );

                    model_->setData(target, result, Qt::EditRole);
                } catch (boost::regex_error) {}

                selected_row++;
            }

            updated_rows.append(index.row());
        }
    }

    proxy_model_->setDynamicSortFilter(true);
    ui.tableView->scrollTo(ui.tableView->currentIndex());
}

void bruntagger::on_actionDiscardChanges_triggered() {
    proxy_model_->setDynamicSortFilter(false);

    model_->undoChanges();
    populatePicsModel(model_->mp3s().at(ui.tableView->currentIndex().row()));

    proxy_model_->setDynamicSortFilter(true);
    ui.tableView->scrollTo(ui.tableView->currentIndex());
}

void bruntagger::on_actionSave_triggered() {
    if (QMessageBox::warning(this, tr("Save files"), tr("Are you sure?"), QMessageBox::Ok | QMessageBox::Cancel)
            == QMessageBox::Ok)
        ui.statusbar->showMessage(tr("%n file(s) saved.", "", model_->save()));
    emit selectedRowChanged(ui.tableView->currentIndex(), QModelIndex());
}

void bruntagger::on_actionStop_triggered() {
    model_->stop();
}

void bruntagger::hheader_customContextMenuRequested(const QPoint &pos) {
    clicked_header_column_ = ui.tableView->horizontalHeader()->logicalIndexAt(pos);
    if (clicked_header_column_ != -1) {
        QMenu menu;
        ui.actionHideColumn->setText(tr("Hide %1").arg(model_->column(clicked_header_column_)));
        menu.addAction(ui.actionHideColumn);
        for (int i = 0; i < model_->columnCount(); i++) {
            if (ui.tableView->horizontalHeader()->isSectionHidden(i))
                menu.addAction(header_actions_.at(i));
        }
        menu.exec(ui.tableView->horizontalHeader()->viewport()->mapToGlobal(pos));
    }
}

void bruntagger::showColumn() {
    int col = sender()->property("column").toInt();

    ui.tableView->horizontalHeader()->moveSection(
            ui.tableView->horizontalHeader()->visualIndex(col),
            ui.tableView->horizontalHeader()->visualIndex(clicked_header_column_));
    ui.tableView->horizontalHeader()->setSectionHidden(col, false);
}

void bruntagger::on_actionHideColumn_triggered() {
    ui.tableView->horizontalHeader()->setSectionHidden(clicked_header_column_, true);
}

void bruntagger::hheader_doubleClicked() {
    proxy_model_->sort(-1);
    ui.tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
}

void bruntagger::hheader_sectionResized() {
    auto_resize_cols_ = false;
}

void bruntagger::populatePicsModel(Mp3 *mp3) {
    if (QFileInfo(mp3->original_path()).exists() && mp3->has_pics()) {
        pics_model_->clear();
        for (int i = 0; i < mp3->pics().count(); i++) {
            QImage pic = mp3->pics().at(i);
            QString text = QString("[%1] %2x%3").arg(mp3->pic_mimes().at(i)).arg(pic.width()).arg(pic.height());
            pics_model_->addPic(pic, text);
        }
    } else {
        pics_model_->clear();
    }
}

void bruntagger::selectedRowChanged(const QModelIndex &current, const QModelIndex &previous) {
    Q_UNUSED(previous);
    if (current.isValid()) {
        int row = proxy_model_->mapToSource(current).row();
        Mp3 *mp3 = model_->mp3s().at(row);

        clicked_file_path_ = mp3->original_path();

        populatePicsModel(mp3);

        if (!ui.actionStop->isVisible())
            showMime(QFileInfo(mp3->original_path()));

        ui.tableView->scrollTo(current);

        qDebug() << mp3->raw() << "\n----------\n";
    }
    else
        clicked_file_path_ = QString();
}

void bruntagger::on_tableView_doubleClicked(const QModelIndex &index) {
    int row = proxy_model_->mapToSource(index).row();
    Mp3 *mp3 = model_->mp3s().at(row);

    if (QFileInfo(mp3->path()).isDir()) {
        changeDir(QDir::toNativeSeparators(mp3->path()));
    }
}

void bruntagger::on_tableView_customContextMenuRequested(const QPoint &pos) {
    QModelIndex index = proxy_model_->mapToSource(ui.tableView->indexAt(pos));
    if (index.isValid()) {
        Mp3 *mp3 = model_->mp3s().at(index.row());

        ui.actionLaunch->setIcon(QFileIconProvider().icon(QFileInfo(mp3->original_path())));

        set_same_ = model_->data(index, Qt::EditRole).toString();

        int column = proxy_model_->mapToSource(ui.tableView->currentIndex()).column();

        ui.actionSetSame->setText(tr("Set selected to \"%1\"").arg(set_same_));

        ui.actionSetSame->setEnabled(ui.tableView->selectionModel()->selectedIndexes().count() > 1
                && !model_->read_only_columns().contains(column));
        ui.actionAddPics->setEnabled(mp3->isMp3());
        ui.actionOpenContaining->setEnabled(!ui.actionStop->isVisible()
                && QFileInfo(mp3->original_path()).absolutePath() != QDir::currentPath());

        QMenu menu;
        menu.addAction(ui.actionSetSame);
        menu.addAction(ui.actionAddPics);
        menu.addAction(ui.actionRemovePics);
        menu.addAction(ui.actionRemoveId3V1);
        menu.addAction(ui.actionOpenContaining);
        menu.addAction(ui.actionLaunch);
        menu.addAction(ui.actionLaunchFolder);

        menu.exec(ui.tableView->viewport()->mapToGlobal(pos));
    }
}

void bruntagger::on_actionLaunch_triggered() {
    QDesktopServices::openUrl("file://" + clicked_file_path_);
}

void bruntagger::on_actionLaunchFolder_triggered() {
    QDesktopServices::openUrl("file://" + QFileInfo(clicked_file_path_).absolutePath());
}

void bruntagger::on_actionOpenContaining_triggered() {
    changeDir(QDir::toNativeSeparators(QFileInfo(clicked_file_path_).absolutePath()));
}

void bruntagger::on_actionSetSame_triggered() {
    proxy_model_->setDynamicSortFilter(false);

    QModelIndexList indexes = ui.tableView->selectionModel()->selectedIndexes();
    for (int i = 0; i < indexes.count(); i++) {
        QModelIndex index = proxy_model_->mapToSource(indexes.at(i));
        Mp3 *mp3 = model_->mp3s().at(index.row());

        if (mp3->isMp3()
                && index.column() != TagsModel::File
                && !model_->read_only_columns().contains(index.column()))
            model_->setData(index, set_same_, Qt::EditRole);
    }

    proxy_model_->setDynamicSortFilter(true);
    ui.tableView->scrollTo(ui.tableView->currentIndex());
}

void bruntagger::on_picsView_customContextMenuRequested(const QPoint &pos) {
    QModelIndex index =  ui.picsView->indexAt(pos);

    QMenu menu;
    ui.actionExtractPic->setEnabled(index.isValid());
    ui.actionRemovePic->setEnabled(index.isValid());
    menu.addAction(ui.actionExtractPic);
    menu.addAction(ui.actionAddPics);
    menu.addAction(ui.actionRemovePic);
    menu.exec(ui.picsView->viewport()->mapToGlobal(pos));
}

void bruntagger::on_actionExtractPic_triggered() {
    Mp3 mp3(clicked_file_path_);

    QString default_filename = mp3.album();
    if (default_filename.isEmpty())
        default_filename = QString::number(ui.picsView->action_row());

    QString mime = mp3.pic_mimes().at(ui.picsView->action_row());
    KMimeType::Ptr kmime = KMimeType::mimeType(mime);

    if (kmime) {
        QString ext = kmime.data()->mainExtension();
        if (ext == ".jpeg")
            ext = ".jpg";
        default_filename.append(ext);
    }
    else {
        default_filename.append(".jpg");
    }

    default_filename.replace("/", "_");
    default_filename.replace(";", "_");
    default_filename.replace("\\", "_");
    default_filename.replace("'", "_");
    default_filename.replace("\"", "_");
    default_filename = QFileInfo(clicked_file_path_).absolutePath() + "/" + default_filename;

    bool loop = true;
    while (loop) {
        QString new_filename = QFileDialog::getSaveFileName(this, tr("Save Pic"), default_filename, tr("All files(*)"),
                NULL, QFileDialog::ReadOnly);
        if (new_filename.isEmpty())
            loop = false;
        else {
            if (mp3.extractPic(ui.picsView->action_row(), new_filename)) {
                ui.statusbar->showMessage(tr("Pic saved."), kStatusTimeout);
                loop = false;
            } else {
                ui.statusbar->showMessage(tr("Error saving pic."), kStatusTimeout);
            }
        }
    }
}


void bruntagger::on_actionAddPics_triggered() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Add Images"), QFileInfo(clicked_file_path_).absolutePath(),
            tr("Image Files(*.png *.jpg *.jpeg *.gif *.bmp);;All files(*)"), NULL, QFileDialog::ReadOnly);
    QList<int> updated_rows;

    if (!filenames.isEmpty()) {
        QModelIndexList selected_indexes = ui.tableView->selectionModel()->selectedIndexes();

        for (int i = 0; i < selected_indexes.count(); i++) {
            QModelIndex index = proxy_model_->mapToSource(selected_indexes.at(i));

            if (!updated_rows.contains(index.row())) {
                Mp3 *mp3 = model_->mp3s().at(index.row());
                if (mp3->isMp3()) {
                    mp3->addPics(filenames);
                    model_->setData(model_->index(index.row(), TagsModel::Pics), mp3->pics_count(), Qt::EditRole);
                    populatePicsModel(mp3);
                }

                updated_rows << index.row();
            }
        }
    }
}

void bruntagger::on_actionRemovePics_triggered() {
    QList<int> updated_rows;
    QModelIndexList selected_indexes = ui.tableView->selectionModel()->selectedIndexes();

    if (selected_indexes.count() > 0
            && QMessageBox::question(this, tr("Remove pics"), tr("Are you sure?"), QMessageBox::Ok | QMessageBox::Cancel)
        == QMessageBox::Ok) {
        for (int i = 0; i < selected_indexes.count(); i++) {
            QModelIndex index = proxy_model_->mapToSource(selected_indexes.at(i));

            if (!updated_rows.contains(index.row())) {
                Mp3 *mp3 = model_->mp3s().at(index.row());
                if (mp3->isMp3() && mp3->has_pics()) {
                    mp3->removePics();
                    model_->setData(model_->index(index.row(), TagsModel::Pics), mp3->pics_count(), Qt::EditRole);
                }

                if (selected_indexes.at(i).row() == ui.tableView->currentIndex().row())
                    pics_model_->clear();

                updated_rows << index.row();
            }
        }
    }
}

void bruntagger::on_actionRemovePic_triggered() {
    QModelIndex index = proxy_model_->mapToSource(ui.tableView->currentIndex());
    if (index.isValid()) {
        Mp3 *mp3 = model_->mp3s().at(index.row());
        if (mp3->isMp3()) {
            mp3->removePic(ui.picsView->action_row());
            model_->setData(model_->index(index.row(), TagsModel::Pics), mp3->pics_count(), Qt::EditRole);

            pics_model_->removeRow(ui.picsView->action_row());
        }
    }
}

void bruntagger::showPic() {
    QPixmap pixmap = QPixmap::fromImage(pics_model_->pics().at(ui.picsView->action_row()));
    if (pixmap.height() > QApplication::desktop()->height() || pixmap.width() > QApplication::desktop()->width())
        pixmap = pixmap.scaled(QApplication::desktop()->size(), Qt::KeepAspectRatio);
    splash_->setPixmap(pixmap);
    splash_->show();
}

void bruntagger::closePic() {
    splash_->close();
}

void bruntagger::on_actionRemoveId3V1_triggered() {
    QList<int> updated_rows;
    QModelIndexList selected_indexes = ui.tableView->selectionModel()->selectedIndexes();

    if (selected_indexes.count() > 0) {
        for (int i = 0; i < selected_indexes.count(); i++) {
            QModelIndex index = proxy_model_->mapToSource(selected_indexes.at(i));

            if (!updated_rows.contains(index.row())) {
                Mp3 *mp3 = model_->mp3s().at(index.row());
                if (mp3->isMp3() && mp3->hasId3V1()) {
                    model_->setData(model_->index(index.row(), TagsModel::HasId3V1), false, Qt::EditRole);
                    mp3->removeId3V1();
                }

                updated_rows << index.row();
            }
        }
    }
}

void bruntagger::changeDir(int index) {
    if (index < 0) {
        index = 0;
    } else if (index >= prev_dirs_.count()) {
        index = prev_dirs_.count() - 1;
    }

    prev_dirs_index_ = index;

    changeDir(prev_dirs_.at(prev_dirs_index_), false);
}

void bruntagger::changeDir(QString dir, bool append) {
    dir = QFileInfo(dir).canonicalFilePath();

    if (append) {
        if (prev_dirs_.count() == 0 || prev_dirs_.at(prev_dirs_index_) != dir) {
            for (int i = 0; i < prev_dirs_.count() - prev_dirs_index_ - 1; i++) {
                prev_dirs_.removeLast();
            }
            prev_dirs_.append(dir);
            prev_dirs_index_ = prev_dirs_.count() - 1;
        }
    }

    ui.lineDir->setText(dir);
    QDir::setCurrent(dir);

    ui.statusbar->showMessage("Loading...");

    static_cast<PicsModel*>(ui.picsView->model())->clear();

    ui.tableView->setCurrentIndex(QModelIndex());
    ui.progressBar->reset();
    ui.progressBar->show();

    for (int i = 0; i < ui.toolBarDir->actions().count(); i++)
        ui.toolBarDir->actions().at(i)->setEnabled(false);

    ui.actionRefresh->setVisible(false);
    ui.actionStop->setVisible(true);
    ui.actionStop->setEnabled(true);

    model_->go(ui.lineDir->text(), ui.comboFileFilter->currentText(),
            ui.actionToggleRecurse->isChecked(), ui.actionToggleShowDirs->isChecked());

    ui.tableView->setCurrentIndex(model_->index(0, 0));
    ui.tableView->setFocus();

    if (auto_resize_cols_) {
        ui.tableView->resizeColumnsToContents();
        auto_resize_cols_ = false;
    }

    ui.progressBar->hide();

    for (int i = 0; i < ui.toolBarDir->actions().count(); i++)
        ui.toolBarDir->actions().at(i)->setEnabled(true);

    ui.actionRefresh->setVisible(true);
    ui.actionStop->setVisible(false);

    ui.actionBack->setEnabled(true);
    ui.actionForward->setEnabled(true);

    if (prev_dirs_index_ == 0) {
        ui.actionBack->setEnabled(false);
        if (prev_dirs_.count() < 2) {
            ui.actionForward->setEnabled(false);
        }
    } else if (prev_dirs_index_ == prev_dirs_.count() - 1) {
        ui.actionForward->setEnabled(false);
    }

    ui.statusbar->showMessage(tr("%n item(s).", "", model_->rowCount()));
}

void bruntagger::showMime(const QFileInfo &file) {
    QString mime = "[" + KMimeType::findByFileContent(file.absoluteFilePath())->name() + "] " + file.absoluteFilePath();
    ui.statusbar->showMessage(mime, kStatusTimeout);
}

void bruntagger::filter() {
    proxy_model_->setFilterKeyColumn(ui.comboFilterTarget->currentIndex());
    proxy_model_->setFilterRegExp(ui.lineFilter->text());
}

void bruntagger::clearFilter() {
    proxy_model_->setFilterRegExp(QString());
}

void bruntagger::loadSettings() {
    QSettings settings;

    resize(settings.value("size", QSize(1100, 735)).toSize());

    if (!restoreState(settings.value("state").toByteArray()))
        restoreDefaultState();

    if (!ui.tableView->horizontalHeader()->restoreState(settings.value("hheader/state").toByteArray())) {
        restoreDefaultHeader();
    }

    auto_resize_cols_ = settings.value("auto_resize_cols_", auto_resize_cols_).toBool();

    ui.lineDir->setText(settings.value("lineDir/text", QDir::toNativeSeparators(QDir::currentPath())).toString());
    ui.actionToggleRecurse->setChecked(settings.value("actionToggleRecurse/checked", ui.actionToggleRecurse->isChecked()).toBool());
    ui.actionToggleShowDirs->setChecked(settings.value("actionToggleShowDirs/checked", ui.actionToggleShowDirs->isChecked()).toBool());
    ui.comboFileFilter->setCurrentIndex(settings.value("comboFileFilter/currentIndex", ui.comboFileFilter->currentIndex()).toInt());
}

void bruntagger::saveSettings() {
    QSettings settings;

    settings.setValue("size", size());
    settings.setValue("state", saveState());

    settings.setValue("hheader/state", ui.tableView->horizontalHeader()->saveState());
    settings.setValue("auto_resize_cols_", auto_resize_cols_);

    settings.setValue("lineDir/text", QDir::toNativeSeparators(QDir::currentPath()));
    settings.setValue("actionToggleRecurse/checked", ui.actionToggleRecurse->isChecked());
    settings.setValue("actionToggleShowDirs/checked", ui.actionToggleShowDirs->isChecked());
    settings.setValue("comboFileFilter/currentIndex", ui.comboFileFilter->currentIndex());
}

void bruntagger::replaceMacros(QString &str, Mp3 *mp3, int row) {
    std::string sstr = str.toStdString();
    int loops = 0;
    while (loops++ < 1000 && boost::regex_match(sstr, boost::regex(".*\\{.*%[^\\}]*\\}.*"))) {
        boost::smatch where;
        if (boost::regex_match(sstr, where, boost::regex(".*(\\{.*%[^\\}]*\\}).*"))) {
            boost::smatch what;
            if (boost::regex_match(sstr, what, boost::regex(".*\\{(.*%[^\\}]*)\\}.*"))) {
                boost::smatch who;
                if (boost::regex_match(static_cast<std::string>(what[1]), who, boost::regex(".*(%[a-z]+).*"))) {
                    QString box_out = QString::fromStdString(static_cast<std::string>(where[1]));
                    QString box_in = QString::fromStdString(static_cast<std::string>(what[1]));
                    QString tag = QString::fromStdString(static_cast<std::string>(who[1]));

                    QString field = "";

                    if (tag == "%file") {
                        field = mp3->filename();
                    } else if (tag == "%ext") {
                        field = QFileInfo(mp3->filename()).suffix();
                    } else if (tag == "%bitrate") {
                        field = mp3->bitrate();
                    } else if (tag == "%track") {
                        field = mp3->track();
                    } else if (tag == "%title") {
                        field = mp3->title();
                    } else if (tag == "%artist") {
                        field = mp3->artist();
                    } else if (tag == "%composer") {
                        field = mp3->composer();
                    } else if (tag == "%album") {
                        field = mp3->album();
                    } else if (tag == "%disc") {
                        field = mp3->disc();
                    } else if (tag == "%date") {
                        field = mp3->date();
                    } else if (tag == "%genre") {
                        field = mp3->genre();
                    }

                    if (field.trimmed() != "") {
                        str = str.replace(box_out, box_in.replace(tag, field));
                    } else {
                        str = str.replace(box_out, "");
                    }
                }
            }
        }
        sstr = str.toStdString();
    }

    if (loops == 1000) {
        qDebug() << "replaceMacros() crapped out.";
    }

    str = str.replace("%album", mp3->album(), Qt::CaseInsensitive);
    str = str.replace("%artist", mp3->artist(), Qt::CaseInsensitive);
    str = str.replace("%bitrate", mp3->bitrate(), Qt::CaseInsensitive);
    str = str.replace("%composer", mp3->composer(), Qt::CaseInsensitive);
    str = str.replace("%date", mp3->date(), Qt::CaseInsensitive);
    str = str.replace("%disc", mp3->disc(), Qt::CaseInsensitive);
    str = str.replace("%ext", QFileInfo(mp3->filename()).suffix(), Qt::CaseInsensitive);
    str = str.replace("%file", mp3->filename(), Qt::CaseInsensitive);
    str = str.replace("%genre", mp3->genre(), Qt::CaseInsensitive);
    str = str.replace("%hasid3v1", mp3->hasId3V1() ? "True" : "False", Qt::CaseInsensitive);
    str = str.replace("%length", mp3->length().toString(), Qt::CaseInsensitive);
    str = str.replace("%modified", mp3->modified().toString(), Qt::CaseInsensitive);
    str = str.replace("%path", mp3->path(), Qt::CaseInsensitive);
    str = str.replace("%pics", QString::number(mp3->pics_count()), Qt::CaseInsensitive);
    str = str.replace("%row", QString::number(row + 1), Qt::CaseInsensitive);
    str = str.replace("%title", mp3->title(), Qt::CaseInsensitive);
    str = str.replace("%track", mp3->track(), Qt::CaseInsensitive);
}

void bruntagger::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void bruntagger::dropEvent(QDropEvent *event) {
    if (event->mimeData()->urls().length() == 1) {
        QFileInfo fi(event->mimeData()->urls().at(0).toLocalFile());
        if (fi.isDir()) {
            changeDir(fi.absoluteFilePath());
        } else {
            changeDir(fi.absolutePath());
        }
    }
}

void bruntagger::on_actionAbout_triggered() {
    QMessageBox::about(this, tr("About"),
            tr("%1\nversion %2\nby %3").arg(APP_NAME, APP_VERSION, APP_COMPANY));
}

}
