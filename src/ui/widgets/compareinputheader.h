#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

namespace mergeqt::ui {

class CompareInputHeader : public QWidget
{
    Q_OBJECT

public:
    explicit CompareInputHeader(const QString &leftLabelText,
                                const QString &rightLabelText,
                                QWidget *parent = nullptr);

    void setLabelTexts(const QString &leftLabelText, const QString &rightLabelText);
    void retranslateUi();

    [[nodiscard]] QLineEdit *leftPathEdit() const;
    [[nodiscard]] QLineEdit *rightPathEdit() const;
    [[nodiscard]] QPushButton *compareButton() const;
    [[nodiscard]] QPushButton *reloadButton() const;
    [[nodiscard]] QPushButton *swapButton() const;

Q_SIGNALS:
    void leftBrowseRequested();
    void rightBrowseRequested();
    void compareRequested();
    void reloadRequested();
    void swapRequested();

private:
    QLabel *m_leftLabel = nullptr;
    QLabel *m_rightLabel = nullptr;
    QLineEdit *m_leftPathEdit = nullptr;
    QLineEdit *m_rightPathEdit = nullptr;
    QPushButton *m_leftBrowseButton = nullptr;
    QPushButton *m_rightBrowseButton = nullptr;
    QPushButton *m_compareButton = nullptr;
    QPushButton *m_reloadButton = nullptr;
    QPushButton *m_swapButton = nullptr;
    QString m_leftLabelText;
    QString m_rightLabelText;
};

} // namespace mergeqt::ui
