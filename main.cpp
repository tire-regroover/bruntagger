#include "bruntagger.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName(APP_COMPANY);
    QCoreApplication::setOrganizationDomain(APP_COMPANY_DOMAIN);

    QApplication app(argc, argv);

    QString locale = QLocale::system().name();

    QTranslator translator;
    translator.load(locale);
    app.installTranslator(&translator);

    bruntagger::bruntagger window;
    window.show();
    return app.exec();
}
