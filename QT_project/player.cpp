#include "player.h"
//#include "ui_player.h"

#include "playercontrols.h"
#include "playlistmodel.h"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QAudioProbe>
#include <QMediaMetaData>
#include <QtWidgets>


Player::Player(QWidget *parent)
        :QWidget(parent)
{
    //объект
    m_player = new QMediaPlayer(this);
    m_player->setAudioRole(QAudio::VideoRole);
    qInfo() << "Supported audio roles: ";
    for (QAudio::Role role : m_player->supportedAudioRoles())
        qInfo() << " " <<role;
    //плейлист
    m_playlist = new QMediaPlaylist();
    m_player->setPlaylist(m_playlist);
    m_player->setVolume(70);

    //записываем сразу сигнал потом слот
    connect(m_player, &QMediaPlayer::durationChanged, this, &Player::durationChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
    // использовать qoverload (вспомогательная функция для перегрузки (одинаковые названия))
    connect(m_player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &Player::metaDataChanged);
    connect(m_playlist , &QMediaPlaylist::currentIndexChanged, this, &Player::playlistPositionChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &Player::statusChanged);
    connect (m_player, &QMediaPlayer::bufferStatusChanged,this ,&Player::bufferingProgress);
    connect(m_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &Player::displayErrorMessage);
   // connect(m_player, &QMediaPlayer::stateChanged, this , &Player::stateChanged);



   m_playlistModel = new PlaylistModel(this);
   m_playlistModel->setPlaylist(m_playlist);


    m_playlistView = new QListView(this);//представляет записи, хранящиеся в моддели в виде списка
    m_playlistView->setModel(m_playlistModel);
    m_playlistView->setCurrentIndex(m_playlistModel->index(m_playlist->currentIndex(), 0));

    connect(m_playlistView, &QAbstractItemView::activated, this, &Player::jump);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0,m_player->duration()/1000);

    m_labelDuration = new QLabel (this);
    connect(m_slider, &QSlider::sliderMoved, this, &Player::seek);

    QPushButton *openButton = new QPushButton(tr("Open"), this);

    connect(openButton, &QPushButton::clicked, this, &Player::open);

    //PlayerControls *controls = new PlayerControls(this);
    controls = new PlayerControls(this);
    controls->setState(m_player->state());
    controls->setVolume(m_player->volume());
    controls->setMuted(controls->isMuted());
    connect(controls, &PlayerControls::play, m_player, &QMediaPlayer::play);
        connect(controls, &PlayerControls::pause, m_player, &QMediaPlayer::pause);
        connect(controls, &PlayerControls::stop, m_player, &QMediaPlayer::stop);
        connect(controls, &PlayerControls::next, m_playlist, &QMediaPlaylist::next);
        connect(controls, &PlayerControls::previous, this, &Player::previousClicked);
        connect(controls, &PlayerControls::changeVolume, m_player, &QMediaPlayer::setVolume);
        connect(controls, &PlayerControls::changeMuting, m_player, &QMediaPlayer::setMuted);
        connect(controls, &PlayerControls::changeRate, m_player, &QMediaPlayer::setPlaybackRate);

        connect(m_player, &QMediaPlayer::stateChanged, controls, &PlayerControls::setState);
        connect(m_player, &QMediaPlayer::volumeChanged, controls, &PlayerControls::setVolume);
        connect(m_player, &QMediaPlayer::mutedChanged, controls, &PlayerControls::setMuted);

        QBoxLayout *displayLayout = new QHBoxLayout;

            displayLayout->addWidget(m_playlistView);

            QBoxLayout *controlLayout = new QHBoxLayout;
            controlLayout->setMargin(0);
            controlLayout->addWidget(openButton);
            controlLayout->addStretch(1);
            controlLayout->addWidget(controls);
            controlLayout->addStretch(1);

            QBoxLayout *layout = new QVBoxLayout;
            layout->addLayout(displayLayout);
            QHBoxLayout *hLayout = new QHBoxLayout;
            hLayout->addWidget(m_slider);
            hLayout->addWidget(m_labelDuration);
            layout->addLayout(hLayout);
            layout->addLayout(controlLayout);


    setLayout(layout);

    if (!isPlayerAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));

        controls->setEnabled(false);
        m_playlistView->setEnabled(false);
        openButton->setEnabled(false);
        m_colorButton->setEnabled(false);

    }

    metaDataChanged();
}

Player::~Player()
{
}

bool Player::isPlayerAvailable() const
{
    return m_player->isAvailable();
}

void Player::open()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Files"));
    QStringList supportedMimeTypes = m_player->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty()) {
        supportedMimeTypes.append("audio/x-m3u"); // MP3 playlists
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        addToPlaylist(fileDialog.selectedUrls());
}

static bool isPlaylist(const QUrl &url) // ".m3u" playlists.
{
    if (!url.isLocalFile())
        return false;
    const QFileInfo fileInfo(url.toLocalFile());
    return fileInfo.exists() && !fileInfo.suffix().compare(QLatin1String("m3u"), Qt::CaseInsensitive);
}

void Player::addToPlaylist(const QList<QUrl> &urls)
{
    for (auto &url: urls) {
        if (isPlaylist(url))
            m_playlist->load(url);
        else
            m_playlist->addMedia(url);
    }
}

void Player::setCustomAudioRole(const QString &role)
{
    m_player->setCustomAudioRole(role);
}

void Player::durationChanged(qint64 duration)   //время
{
    m_duration = duration / 1000;
    m_slider->setMaximum(m_duration);
}

void Player::positionChanged(qint64 progress)
{
    if (!m_slider->isSliderDown())
        m_slider->setValue(progress / 1000);

    updateDurationInfo(progress / 1000);
}

void Player::metaDataChanged()
{
    if (m_player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                .arg(m_player->metaData(QMediaMetaData::AlbumArtist).toString())
                .arg(m_player->metaData(QMediaMetaData::Title).toString()));

        if (m_coverLabel) {
            QUrl url = m_player->metaData(QMediaMetaData::CoverArtUrlLarge).value<QUrl>();

            m_coverLabel->setPixmap(!url.isEmpty()
                    ? QPixmap(url.toString())
                    : QPixmap());
        }
    }
}

void Player::previousClicked()
{
    if (m_player->position() <= 5000)
        m_playlist->previous();
    else
        m_player->setPosition(0);
}

void Player::jump(const QModelIndex &index)
{
    if (index.isValid()) { //isValid если индекс принадлежит модели и имеет неотрицательные номера возвращает истину
        m_playlist->setCurrentIndex(index.row());
        m_player->play();
    }
}

void Player::playlistPositionChanged(int currentItem)
{
    m_playlistView->setCurrentIndex(m_playlistModel->index(currentItem, 0));
}

void Player::seek(int seconds)
{
    m_player->setPosition(seconds * 1000);
}

void Player::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);

    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(tr("Buffering %1%").arg(m_player->bufferStatus()));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Stalled %1%").arg(m_player->bufferStatus()));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage();
        break;
    }
}



void Player::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if (status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void Player::bufferingProgress(int progress)
{
    if (m_player->mediaStatus() == QMediaPlayer::StalledMedia) // медиа статус содержит статус текущего медиапотока
        setStatusInfo(tr("Stalled %1%").arg(progress));
    else
        setStatusInfo(tr("Buffering %1%").arg(progress));
}
void Player::setTrackInfo(const QString &info)
{
    m_trackInfo = info;

    if (m_statusBar) {
        m_statusBar->showMessage(m_trackInfo);
        m_statusLabel->setText(m_statusInfo);
    } else {
        if (!m_statusInfo.isEmpty())
            setWindowTitle(QString("%1 | %2").arg(m_trackInfo).arg(m_statusInfo));
        else
            setWindowTitle(m_trackInfo);// верхняя информационная строка
    }
}

void Player::setStatusInfo(const QString &info)
{
    m_statusInfo = info;

    if (m_statusBar) {
        m_statusBar->showMessage(m_trackInfo);
        m_statusLabel->setText(m_statusInfo);
    } else {
        if (!m_statusInfo.isEmpty())
            setWindowTitle(QString("%1 | %2").arg(m_trackInfo).arg(m_statusInfo));
        else
            setWindowTitle(m_trackInfo);
    }
}

void Player::displayErrorMessage()
{
    setStatusInfo(m_player->errorString());
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || m_duration) {
        QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60,
            currentInfo % 60, (currentInfo * 1000) % 1000);
        QTime totalTime((m_duration / 3600) % 60, (m_duration / 60) % 60,
            m_duration % 60, (m_duration * 1000) % 1000);
        QString format = "mm:ss";
        if (m_duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    m_labelDuration->setText(tStr);
}


