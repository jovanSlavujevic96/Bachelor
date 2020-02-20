#ifndef BACHELOR_VISUALIZER_OBJECTVISUALIZER_HPP_
#define BACHELOR_VISUALIZER_OBJECTVISUALIZER_HPP_

#include "IVisualizer.hpp"

class ObjectVisualizer : public IVisualizer
{
    std::vector<cv::Rect> m_Rects;
    cv::Scalar m_RectColor, m_TextColor;
    std::string m_SignName;
    const VisualizerType m_VisualizerType;

public:
    ObjectVisualizer(VisualizerType type);
    virtual ~ObjectVisualizer() = default;

    void update(const bachelor::Coordinates& msg, Topics subjTopic) override;
    bool doStuff(void) override;

    void draw(cv::Mat& frame) override;
    VisualizerType getVisualizerType(void) override;
};

#endif //BACHELOR_VISUALIZER_OBJECTVISUALIZER_HPP_