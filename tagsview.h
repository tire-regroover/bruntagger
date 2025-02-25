#ifndef TAGSVIEW_H_
#define TAGSVIEW_H_

#include <qtableview.h>

namespace bruntagger {

class TagsView: public QTableView {
public:
    TagsView(QWidget *parent = 0);
    virtual ~TagsView();
};

}

#endif /* TAGSVIEW_H_ */
