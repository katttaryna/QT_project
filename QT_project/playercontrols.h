#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H



#include <QMediaPlayer>
#include <QWidget>
#include <QSlider>

QT_BEGIN_NAMESPACE
class QAbstractButton ;
class QAcstractSlider;
class QComboBox;
QT_END_NAMESPACE

//ДОБАВИТЬ СКОРОСТЬ ВОСПРОИЗВЕДЕНИЯ ЧЕРЕЗ qreal


class PlayerControls : public QWidget
{
    Q_OBJECT

public:
    //explicit предотвращает неявное преобразование типов
    explicit PlayerControls(QWidget *parent = nullptr);  //Создает новый элемент управления медиаплеером с заданным родителем.

   QMediaPlayer::State state() const;
   int volume() const;
   bool isMuted() const;
   qreal playbackRate() const; // скорость воспр изм

public slots:
   void setState(QMediaPlayer::State state);
   void setVolume(int volume);
   void setMuted(bool muted);
   void setPlaybackRate(float rate);


signals:
   void play();
   void pause();
   void stop();
   void next();
   void previous();

   //
   void changeVolume(int volume);
   void changeMuting(bool muting);
   void changeRate(qreal rate);


private slots:
   void playClicked();
   void muteClicked();
   void updateRate();
   //
  void onVolumeSliderValueChanged();


private:
   QMediaPlayer::State m_playerState = QMediaPlayer::StoppedState;
   bool m_playerMuted = false;
   QAbstractButton *m_playButton = nullptr;
   QAbstractButton *m_stopButton = nullptr;
   QAbstractButton *m_nextButton = nullptr;
   QAbstractButton *m_previousButton = nullptr;
   QAbstractButton *m_muteButton = nullptr;
   //
   QSlider *m_volumeSlider = nullptr;
   QComboBox *m_rateBox = nullptr;
};

#endif // PLAYERCONTROLS_H
