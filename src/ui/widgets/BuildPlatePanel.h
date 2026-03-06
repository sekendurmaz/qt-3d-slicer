#pragma once

#include <QWidget>
#include <QRadioButton>
#include <QSpinBox>
#include <QPushButton>
#include <memory>
#include "core/buildplate/BuildPlate.h"

namespace ui {
namespace widgets {

class BuildPlatePanel : public QWidget
{
    Q_OBJECT

public:
    explicit BuildPlatePanel(QWidget* parent = nullptr);

    std::shared_ptr<core::buildplate::BuildPlate> currentPlate() const { return currentPlate_; }

signals:
    void plateCreated(std::shared_ptr<core::buildplate::BuildPlate> plate);

private slots:
    void onPlateTypeChanged();
    void onCreatePlate();

private:
    void setupUI();
    void updateInputsVisibility();

    // UI components
    QRadioButton* rbRectangular_;
    QRadioButton* rbCircular_;

    // Rectangular inputs
    QWidget* rectangularWidget_;
    QSpinBox* spinRectWidth_;
    QSpinBox* spinRectDepth_;
    QSpinBox* spinRectHeight_;

    // Circular inputs
    QWidget* circularWidget_;
    QSpinBox* spinCircDiameter_;
    QSpinBox* spinCircHeight_;

    QPushButton* btnCreatePlate_;

    // Current plate
    std::shared_ptr<core::buildplate::BuildPlate> currentPlate_;
};

} // namespace widgets
} // namespace ui
