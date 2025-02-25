#include "tagsview.h"
#include <QHeaderView>

namespace bruntagger {

TagsView::TagsView(QWidget *parent) : QTableView(parent) {
    horizontalHeader()->setMovable(true);
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
}

TagsView::~TagsView() {
}

}
