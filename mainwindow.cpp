#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置标题
    setWindowTitle("小小播放器");

    pPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    pVideoWidget = new QVideoWidget(ui->label);

    pVideoWidget->resize(500, 260);
    pPlayer->setAudioOutput(audioOutput);
    pPlayer->setVideoOutput(pVideoWidget);

    playlistWidget = new QListWidget(this);
    playlistWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);  // 无边框浮动窗口
    // 设置 QSS 样式表
    playlistWidget->setStyleSheet(R"(
    QListWidget {
        background-color: rgba(50, 50, 50, 180);  /* 半透明深灰色背景 */
        border: 1px solid rgba(255, 255, 255, 50);  /* 1px 半透明白色边框 */
    }
    QListWidget::item {
        color: white;  /* 列表项文字颜色 */
        padding: 5px;
    }
    QListWidget::item:selected {
        background-color: rgba(0, 120, 215, 150);  /* 选中项的蓝色背景 */
        color: white;
    }
    QListWidget::item:hover {
        background-color: rgba(200, 200, 200, 100);  /* 悬停时的淡灰色背景 */
    }
)");

    playlistWidget->resize(100, 400);  // 设置大小
    playlistWidget->hide();  // 默认隐藏

    audioSlider = new QSlider(Qt::Vertical, this);
    audioSlider->resize(20, 100);
    audioSlider->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);  // 无边框浮动窗口
    audioSlider->setRange(0, 100);
    audioSlider->setValue(50);
    audioSlider->hide();

    connect(pPlayer, &QMediaPlayer::playbackStateChanged,
            this, &MainWindow::handlePlaybackStateChanged);
    connect(pPlayer, &QMediaPlayer::sourceChanged, this, &MainWindow::handleSourceChanged);
    connect(playlistWidget, &QListWidget::itemDoubleClicked,
                    this, &MainWindow::on_playlist_itemDoubleClicked);
    connect(pPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
    connect(pPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
    connect(ui->playslider, &QSlider::sliderMoved, this, [this](int position){
        pPlayer->setPosition(static_cast<qint64>(position));
    });
    connect(ui->playslider, &QSlider::sliderReleased, this, [this](){
        pPlayer->setPosition(static_cast<qint64>(ui->playslider->value()));
        pPlayer->play();
    });
    connect(ui->playslider, &QSlider::sliderPressed, this, [this](){
        pPlayer->pause();
    });
    connect(audioSlider, &QSlider::valueChanged, this, [this](){
        qreal volume = static_cast<qreal>(audioSlider->value()) / 100.0;  // 将整数值映射为浮点数 0.0~1.0
        audioOutput->setVolume(volume);
    });
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateAudioSliderPosition(){
    double audio_x= ui->audioBtn->mapTo(this, QPoint(0, 0)).x();
    double audio_y= ui->audioBtn->mapTo(this, QPoint(0, 0)).y();
    QPoint audio_pos = mapToGlobal(QPoint(audio_x+ui->audioBtn->width()/2-10, audio_y-100));
    audioSlider->move(audio_pos);
}
void MainWindow::moveEvent(QMoveEvent *event){
    QMainWindow::moveEvent(event);
    double w= this->frameGeometry().width();
    QPoint pos = pVideoWidget->mapToGlobal(QPoint(w - playlistWidget->width()-20, 6));  // 偏移以避免完全遮盖视频
    playlistWidget->move(pos);
    updateAudioSliderPosition();
}
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);  // 调用父类的 resizeEvent
    // 根据 label 的当前大小调整 VideoWidget 的大小
    if (pVideoWidget) {
        pVideoWidget->resize(ui->label->size());
    }
    double w= this->frameGeometry().width();
    double h= this->frameGeometry().height();
    QPoint pos = pVideoWidget->mapToGlobal(QPoint(w - playlistWidget->width()-20, 6));  // 偏移以避免完全遮盖视频
    playlistWidget->move(pos);
    playlistWidget->resize(playlistWidget->width(), h - 160);

    updateAudioSliderPosition();
}
void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);  // 调用父类的 showEvent

    // 在这里获取 label 的最终大小并调整 VideoWidget 的大小
    pVideoWidget->resize(ui->label->size());
    pVideoWidget->show();
    updateAudioSliderPosition();
}

void MainWindow::on_positionChanged(qint64 position)
{
    ui->playslider->setValue(static_cast<int>(position));
}

// 设置滑动条的最大值（视频总时长）
void MainWindow::on_durationChanged(qint64 duration)
{
    ui->playslider->setRange(0, static_cast<int>(duration));
}

void MainWindow::handlePlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        ui->playBtn->setText("暂停");
        break;
    case QMediaPlayer::PausedState:
        ui->playBtn->setText("播放");
        break;
    case QMediaPlayer::StoppedState:
        break;
    }
}
void MainWindow::handleSourceChanged(const QUrl &newSource)
{
    current_filename = newSource.fileName();
    QList<QListWidgetItem *> items = playlistWidget->findItems(current_filename, Qt::MatchExactly);
    if (!items.isEmpty()) {
        QListWidgetItem *item = items.first();  // 获取匹配的第一项
        playlistWidget->setCurrentItem(item);  // 设置为当前选中项
        item->setSelected(true);  // 高亮显示
    }
}

void MainWindow::on_openBtn_clicked()
{
    QStringList files_list = QFileDialog::getOpenFileNames(
        this, "选择视频文件", "D:/",
        "MP4 Files (*.mp4);;All Files (*.*)");

    if (!files_list.isEmpty()) {
        videoFiles = files_list;  // 存储选择的文件路径
        currentIndex = 0;  // 初始化当前播放索引
        for (const QString &filePath : files_list) {
            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();  // 获取文件名

            playlistWidget->addItem(fileName);  // 添加文件名到列表
            videoMap[fileName] = filePath;  // 存储文件名与路径的映射
        }
    }

}


void MainWindow::on_playBtn_clicked()
{
    if (pPlayer->source().isEmpty()&&!videoFiles.isEmpty() && currentIndex < videoFiles.size()) {
        QString currentFile = videoFiles[currentIndex];  // 获取当前文件路径
        pPlayer->setSource(QUrl::fromLocalFile(currentFile));  // 设置媒体源
        pPlayer->setPlaybackRate(playbackRate);
    }
    if(pPlayer->isPlaying()){
        pPlayer->pause();
    }
    else {
        pPlayer->play();
    }
}



void MainWindow::on_rateBtn_clicked()
{
    if (playbackRate == 1.0) {
        playbackRate = 1.5;
    } else if (playbackRate == 1.5) {
        playbackRate = 2.0;
    } else if (playbackRate == 2.0) {
        playbackRate = 0.5;
    } else {
        playbackRate = 1.0;
    }
    // 更新播放器的播放速度
    pPlayer->setPlaybackRate(playbackRate);
    ui->rateBtn->setText("倍速：" + QString::number(playbackRate));
}


void MainWindow::on_nextBtn_clicked()
{
    if (!videoFiles.isEmpty()) {
        currentIndex = (currentIndex + 1)%videoFiles.size();
        QString currentFile = videoFiles[currentIndex];  // 获取当前文件路径
        pPlayer->setSource(QUrl::fromLocalFile(currentFile));  // 设置媒体源
        pPlayer->setPlaybackRate(playbackRate);
        pPlayer->play();
    }
}


void MainWindow::on_pushButton_clicked()
{
    if (!videoFiles.isEmpty()) {
        currentIndex = (currentIndex + videoFiles.size() - 1)%videoFiles.size();
        QString currentFile = videoFiles[currentIndex];  // 获取当前文件路径
        pPlayer->setSource(QUrl::fromLocalFile(currentFile));  // 设置媒体源
        pPlayer->setPlaybackRate(playbackRate);
        pPlayer->play();
    }
}


void MainWindow::on_playlistBtn_clicked()
{
    if (playlistWidget->isVisible()) {
        playlistWidget->hide();
    } else {
        playlistWidget->show();
    }
}
void MainWindow::on_playlist_itemDoubleClicked(QListWidgetItem *item)
{
    QString fileName = item->text();  // 获取双击的文件名
    QString filePath = videoMap.value(fileName);  // 查找文件路径
    currentIndex = playlistWidget->row(item);
    if (!filePath.isEmpty()) {
        pPlayer->setSource(QUrl::fromLocalFile(filePath));  // 设置视频源
        pPlayer->play();  // 播放视频
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Space) {  // 空格键用于播放/暂停
        if (pPlayer->isPlaying()) {
            pPlayer->pause();
        } else {
            pPlayer->play();
        }
    }
}

void MainWindow::on_audioBtn_clicked()
{
    if(audioSlider->isVisible()){
        audioSlider->hide();
    }
    else{
        audioSlider->show();
    }
}

