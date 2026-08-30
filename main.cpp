#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

class TimerWindow final : public QMainWindow
{
public:
    TimerWindow()
    {
        setWindowTitle("Timer");
        setFixedSize(540, 260);
        auto *central = new QWidget(this);
        auto *layout = new QVBoxLayout(central);
        layout->setContentsMargins(48, 42, 48, 34);
        layout->setSpacing(28);

        auto *timeLayout = new QHBoxLayout;
        timeLayout->setSpacing(12);
        timeLayout->addStretch();
        addField(timeLayout, hours_, 99);
        addSeparator(timeLayout);
        addField(timeLayout, minutes_, 59);
        addSeparator(timeLayout);
        addField(timeLayout, seconds_, 59);
        timeLayout->addStretch();
        layout->addLayout(timeLayout);

        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(12);
        buttonLayout->addStretch();
        playButton_ = new QPushButton(this);
        playButton_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        playButton_->setToolTip("Start timer");
        playButton_->setAccessibleName("Start timer");
        playButton_->setFixedSize(52, 42);
        acceptButton_ = new QPushButton("Apply", this);
        acceptButton_->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        cancelButton_ = new QPushButton("Cancel", this);
        cancelButton_->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        buttonLayout->addWidget(playButton_);
        buttonLayout->addWidget(acceptButton_);
        buttonLayout->addWidget(cancelButton_);
        buttonLayout->addStretch();
        layout->addLayout(buttonLayout);
        setCentralWidget(central);

        setStyleSheet(R"(
            QMainWindow { background: #ffffff; }
            QLineEdit { background: transparent; color: #111111; border: 2px solid transparent;
                border-radius: 8px; font-size: 48px; font-weight: 500; padding: 5px;
                selection-background-color: #3478f6; selection-color: #ffffff; }
            QLineEdit[editing="true"] { background: #f4f6fa; border-color: #3478f6; }
            QLineEdit[running="true"] { color: #1769e0; }
            QLabel { color: #555b66; font-size: 43px; padding-bottom: 7px; }
            QPushButton { background: #e9ebef; color: #111111; border: none;
                border-radius: 9px; padding: 9px 15px; font-size: 14px; }
            QPushButton:hover { background: #dfe2e8; }
            QPushButton:pressed { background: #d2d6de; }
            QPushButton:disabled { color: #9a9da4; background: #f1f2f4; }
        )");

        connect(playButton_, &QPushButton::clicked, this, [this] { startTimer(); });
        connect(acceptButton_, &QPushButton::clicked, this, [this] { acceptEdit(); });
        connect(cancelButton_, &QPushButton::clicked, this, [this] { cancelEdit(); });
        connect(&timer_, &QTimer::timeout, this, [this] { updateCountdown(); });
        timer_.setInterval(100);
        connect(new QShortcut(QKeySequence(Qt::Key_Return), this), &QShortcut::activated,
                this, [this] { if (editing_) acceptEdit(); });
        connect(new QShortcut(QKeySequence(Qt::Key_Escape), this), &QShortcut::activated,
                this, [this] { if (editing_) cancelEdit(); });

        showDisplay(0);
        setEditMode(false);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (!running_ && !editing_ && event->type() == QEvent::MouseButtonPress &&
            (watched == hours_ || watched == minutes_ || watched == seconds_)) {
            setEditMode(true);
            auto *field = static_cast<QLineEdit *>(watched);
            field->setFocus();
            field->selectAll();
        }
        return QMainWindow::eventFilter(watched, event);
    }

private:
    void addField(QHBoxLayout *layout, QLineEdit *&field, int maximum)
    {
        field = new QLineEdit("00", this);
        field->setAlignment(Qt::AlignCenter);
        field->setFixedWidth(100);
        field->setMaxLength(2);
        field->setValidator(new QIntValidator(0, maximum, field));
        field->setInputMethodHints(Qt::ImhDigitsOnly);
        field->installEventFilter(this);
        layout->addWidget(field);
    }

    void addSeparator(QHBoxLayout *layout)
    {
        auto *separator = new QLabel(":", this);
        separator->setAlignment(Qt::AlignCenter);
        layout->addWidget(separator);
    }

    void setEditMode(bool enabled)
    {
        editing_ = enabled;
        for (auto *field : {hours_, minutes_, seconds_}) {
            field->setReadOnly(!enabled);
            field->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
            field->setProperty("editing", enabled);
            field->style()->unpolish(field);
            field->style()->polish(field);
        }
        playButton_->setVisible(!enabled);
        acceptButton_->setVisible(enabled);
        cancelButton_->setVisible(enabled);
        if (!enabled) centralWidget()->setFocus();
    }

    static int fieldValue(const QLineEdit *field)
    {
        return field->text().isEmpty() ? 0 : field->text().toInt();
    }

    int displayedSeconds() const
    {
        return fieldValue(hours_) * 3600 + fieldValue(minutes_) * 60 + fieldValue(seconds_);
    }

    void showDisplay(int totalSeconds)
    {
        totalSeconds = std::max(0, totalSeconds);
        hours_->setText(QStringLiteral("%1").arg(totalSeconds / 3600, 2, 10, QLatin1Char('0')));
        minutes_->setText(QStringLiteral("%1").arg((totalSeconds / 60) % 60, 2, 10, QLatin1Char('0')));
        seconds_->setText(QStringLiteral("%1").arg(totalSeconds % 60, 2, 10, QLatin1Char('0')));
    }

    void acceptEdit()
    {
        configuredSeconds_ = displayedSeconds();
        remainingSeconds_ = configuredSeconds_;
        showDisplay(remainingSeconds_);
        setEditMode(false);
        playButton_->setEnabled(remainingSeconds_ > 0);
    }

    void cancelEdit()
    {
        showDisplay(remainingSeconds_);
        setEditMode(false);
        playButton_->setEnabled(remainingSeconds_ > 0);
    }

    void startTimer()
    {
        remainingSeconds_ = displayedSeconds();
        if (remainingSeconds_ <= 0) return;
        running_ = true;
        setRunningAppearance(true);
        playButton_->setEnabled(false);
        endTimeMs_ = QDateTime::currentMSecsSinceEpoch() + qint64(remainingSeconds_) * 1000;
        timer_.start();
    }

    void updateCountdown()
    {
        const qint64 millisecondsLeft = endTimeMs_ - QDateTime::currentMSecsSinceEpoch();
        remainingSeconds_ = millisecondsLeft > 0 ? int((millisecondsLeft + 999) / 1000) : 0;
        showDisplay(remainingSeconds_);
        if (millisecondsLeft <= 0) {
            timer_.stop();
            running_ = false;
            setRunningAppearance(false);
            remainingSeconds_ = configuredSeconds_;
            showDisplay(remainingSeconds_);
            playButton_->setEnabled(configuredSeconds_ > 0);
        }
    }

    void setRunningAppearance(bool enabled)
    {
        for (auto *field : {hours_, minutes_, seconds_}) {
            field->setProperty("running", enabled);
            field->style()->unpolish(field);
            field->style()->polish(field);
        }
    }

    QLineEdit *hours_ = nullptr;
    QLineEdit *minutes_ = nullptr;
    QLineEdit *seconds_ = nullptr;
    QPushButton *playButton_ = nullptr;
    QPushButton *acceptButton_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
    QTimer timer_;
    bool editing_ = false;
    bool running_ = false;
    int configuredSeconds_ = 0;
    int remainingSeconds_ = 0;
    qint64 endTimeMs_ = 0;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TimerWindow window;
    window.show();
    return app.exec();
}
