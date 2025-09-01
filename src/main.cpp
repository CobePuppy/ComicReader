#include <QApplication>
#include "../include/ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("ComicReader");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ComicReader");
    app.setOrganizationDomain("comicreader.com");
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}
