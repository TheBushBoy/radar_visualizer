#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "RadarBackend.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    RadarBackend backend;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.loadFromModule("App", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
