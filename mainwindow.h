#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QListWidget>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateAudioSliderPosition();

private slots:
    void resizeEvent(QResizeEvent *event = nullptr) override;
    void moveEvent(QMoveEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void handlePlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void handleSourceChanged(const QUrl &newSource);
    void on_playlist_itemDoubleClicked(QListWidgetItem *item);
    void on_positionChanged(qint64 position);  // 更新滑动条进度
    void on_durationChanged(qint64 duration);  // 设置滑动条最大值
    void keyPressEvent(QKeyEvent *event) override;
    void on_openBtn_clicked();

    void on_playBtn_clicked();

    void on_rateBtn_clicked();

    void on_nextBtn_clicked();

    void on_pushButton_clicked();

    void on_playlistBtn_clicked();

    void on_audioBtn_clicked();

private:
    Ui::MainWindow *ui;

    QMediaPlayer * pPlayer;
    QAudioOutput *audioOutput;
    QVideoWidget *pVideoWidget;
    QListWidget *playlistWidget;
    QSlider *audioSlider;

    QStringList videoFiles;
    int currentIndex = 0;
    QString current_filename;
    double playbackRate = 1.0;
    QMap<QString, QString> videoMap;


};
#endif // MAINWINDOW_H
