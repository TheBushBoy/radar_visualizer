#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "PpiProvider.hpp"
#include "RadarBackend.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    auto* ppi = new PpiProvider();
    RadarBackend backend(ppi);

    QQmlApplicationEngine engine;
    engine.addImageProvider("ppi", ppi);
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.loadFromModule("App", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
