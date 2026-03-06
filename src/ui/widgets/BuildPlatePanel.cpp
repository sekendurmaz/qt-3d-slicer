#include "BuildPlatePanel.h"
#include "core/buildplate/RectangularPlate.h"
#include "core/buildplate/CircularPlate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QButtonGroup>

namespace ui {
namespace widgets {

BuildPlatePanel::BuildPlatePanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    updateInputsVisibility();
}

void BuildPlatePanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Group box
    QGroupBox* groupBox = new QGroupBox("🏗️ Build Plate Settings");
    QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);

    // Plate type selection
    QLabel* labelType = new QLabel("Type:");
    labelType->setStyleSheet("font-weight: bold;");
    groupLayout->addWidget(labelType);

    QHBoxLayout* typeLayout = new QHBoxLayout();
    rbRectangular_ = new QRadioButton("🔲 Rectangular");
    rbCircular_ = new QRadioButton("⭕ Circular");
    rbRectangular_->setChecked(true);

    QButtonGroup* typeGroup = new QButtonGroup(this);
    typeGroup->addButton(rbRectangular_);
    typeGroup->addButton(rbCircular_);

    typeLayout->addWidget(rbRectangular_);
    typeLayout->addWidget(rbCircular_);
    typeLayout->addStretch();
    groupLayout->addLayout(typeLayout);

    groupLayout->addSpacing(10);

    // Rectangular inputs
    rectangularWidget_ = new QWidget();
    QVBoxLayout* rectLayout = new QVBoxLayout(rectangularWidget_);
    rectLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* labelRect = new QLabel("📐 Rectangular Dimensions:");
    labelRect->setStyleSheet("font-weight: bold; color: #2196F3;");
    rectLayout->addWidget(labelRect);

    QHBoxLayout* widthLayout = new QHBoxLayout();
    widthLayout->addWidget(new QLabel("Width:"));
    spinRectWidth_ = new QSpinBox();
    spinRectWidth_->setRange(50, 500);
    spinRectWidth_->setValue(200);
    spinRectWidth_->setSuffix(" mm");
    widthLayout->addWidget(spinRectWidth_);
    rectLayout->addLayout(widthLayout);

    QHBoxLayout* depthLayout = new QHBoxLayout();
    depthLayout->addWidget(new QLabel("Depth:"));
    spinRectDepth_ = new QSpinBox();
    spinRectDepth_->setRange(50, 500);
    spinRectDepth_->setValue(200);
    spinRectDepth_->setSuffix(" mm");
    depthLayout->addWidget(spinRectDepth_);
    rectLayout->addLayout(depthLayout);

    QHBoxLayout* rectHeightLayout = new QHBoxLayout();
    rectHeightLayout->addWidget(new QLabel("Height:"));
    spinRectHeight_ = new QSpinBox();
    spinRectHeight_->setRange(1, 20);
    spinRectHeight_->setValue(5);
    spinRectHeight_->setSuffix(" mm");
    rectHeightLayout->addWidget(spinRectHeight_);
    rectLayout->addLayout(rectHeightLayout);

    groupLayout->addWidget(rectangularWidget_);

    // Circular inputs
    circularWidget_ = new QWidget();
    QVBoxLayout* circLayout = new QVBoxLayout(circularWidget_);
    circLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* labelCirc = new QLabel("⭕ Circular Dimensions:");
    labelCirc->setStyleSheet("font-weight: bold; color: #FF9800;");
    circLayout->addWidget(labelCirc);

    QHBoxLayout* diameterLayout = new QHBoxLayout();
    diameterLayout->addWidget(new QLabel("Diameter:"));
    spinCircDiameter_ = new QSpinBox();
    spinCircDiameter_->setRange(50, 500);
    spinCircDiameter_->setValue(200);
    spinCircDiameter_->setSuffix(" mm");
    diameterLayout->addWidget(spinCircDiameter_);
    circLayout->addLayout(diameterLayout);

    QHBoxLayout* circHeightLayout = new QHBoxLayout();
    circHeightLayout->addWidget(new QLabel("Height:"));
    spinCircHeight_ = new QSpinBox();
    spinCircHeight_->setRange(1, 20);
    spinCircHeight_->setValue(5);
    spinCircHeight_->setSuffix(" mm");
    circHeightLayout->addWidget(spinCircHeight_);
    circLayout->addLayout(circHeightLayout);

    groupLayout->addWidget(circularWidget_);

    groupLayout->addSpacing(10);

    // Create button
    btnCreatePlate_ = new QPushButton("✅ Create Plate");
    btnCreatePlate_->setMinimumHeight(35);
    btnCreatePlate_->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");
    groupLayout->addWidget(btnCreatePlate_);

    mainLayout->addWidget(groupBox);
    mainLayout->addStretch();

    // Connections
    connect(rbRectangular_, &QRadioButton::toggled, this, &BuildPlatePanel::onPlateTypeChanged);
    connect(rbCircular_, &QRadioButton::toggled, this, &BuildPlatePanel::onPlateTypeChanged);
    connect(btnCreatePlate_, &QPushButton::clicked, this, &BuildPlatePanel::onCreatePlate);
}

void BuildPlatePanel::updateInputsVisibility()
{
    bool isRectangular = rbRectangular_->isChecked();
    rectangularWidget_->setVisible(isRectangular);
    circularWidget_->setVisible(!isRectangular);
}

void BuildPlatePanel::onPlateTypeChanged()
{
    updateInputsVisibility();
}

void BuildPlatePanel::onCreatePlate()
{

    if (rbRectangular_->isChecked())
    {
        float width = spinRectWidth_->value();
        float depth = spinRectDepth_->value();
        float height = spinRectHeight_->value();

        currentPlate_ = std::make_shared<core::buildplate::RectangularPlate>(width, depth, height);
    }
    else
    {
        float diameter = spinCircDiameter_->value();
        float height = spinCircHeight_->value();

        currentPlate_ = std::make_shared<core::buildplate::CircularPlate>(diameter, height);
    }

    emit plateCreated(currentPlate_);
}

} // namespace widgets
} // namespace ui
