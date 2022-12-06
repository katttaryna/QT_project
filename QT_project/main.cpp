#include "player.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Player"); // обработка сообщений для консольного приложения
    QCommandLineParser parser; //обработка параметров коммандной строки
    QCommandLineOption customAudioRoleOption("custom-audio-role",
                                                 "Set a custom audio role for the player.",
                                                 "role");
        parser.setApplicationDescription("Qt MultiMedia Player Example");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(customAudioRoleOption);
        parser.addPositionalArgument("url", "The URL(s) to open.");
        parser.process(app);

     Player player;

     if (parser.isSet(customAudioRoleOption))
         player.setCustomAudioRole(parser.value(customAudioRoleOption));

     if (!parser.positionalArguments().isEmpty() && player.isPlayerAvailable()) {
         QList<QUrl> urls;
         for (auto &a: parser.positionalArguments())
             urls.append(QUrl::fromUserInput(a, QDir::currentPath(), QUrl::AssumeLocalFile));
         player.addToPlaylist(urls);
     }

     player.show();
    return app.exec();
}




