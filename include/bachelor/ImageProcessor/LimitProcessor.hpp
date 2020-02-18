#ifndef BACHELOR_IMAGEPROCESSOR_LIMITPROCESSOR_HPP
#define BACHELOR_IMAGEPROCESSOR_LIMITPROCESSOR_HPP

#include "IImageProcessor.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/text/ocr.hpp>

class LimitProcessor : public IImageProcessor
{
private:
    cv::CascadeClassifier m_SpeedClassifier;
    //cv::CascadeClassifier m_LimitRecognizeClassifier[2];
    cv::Mat m_Frame, m_ImageMask;
    bool m_SpeedLimitDetected;
    int m_LimitValue;
    cv::Ptr<cv::text::OCRTesseract> m_OCR;
    //std::map<int, cv::Scalar> m_ColorMap;
    //const int m_NumOfClassifiers;
    //const int m_PossibleLimitValues[5];

    void loadCascade(cv::CascadeClassifier *cascade, const int size, const std::string *path);
    void resize(cv::Mat &image, const float resizeFactor);
    void createMask(const cv::Mat &image);
    cv::Mat makeROI(const cv::Mat &image) const;
    void redColorSegmentation(const cv::Mat &sample, cv::Mat1b &result);
    std::vector<cv::Rect> getRedContours(const cv::Mat1b &hueImage) const;
    void preprocessContours(const cv::Mat &image, std::vector<cv::Rect> &contours);
    std::vector<cv::Rect> getDetectedSpeedLimitContours(const cv::Mat &image, const std::vector<cv::Rect> &contours);
    void resize(std::vector<cv::Rect> &contours, const float resizeFactor);
    cv::Mat approximateCircle(cv::Mat binaryMask, int dilation_elem);
    std::vector<cv::Mat> getTextImagesForOCR(const cv::Mat &image, std::vector<cv::Rect> &contours);
    std::vector<bool> getRecognizedLimits(const std::vector<cv::Mat> &images);

    //std::vector<cv::Rect> getSpeedLimitValues(const cv::Mat &image, const std::vector<cv::Rect> &contours);
    void drawLocations(cv::Mat &image, const std::vector<cv::Rect> &contours,
        const cv::Scalar colorText, const cv::Scalar colorEdge, const std::string text);

public:
    LimitProcessor();
    virtual ~LimitProcessor() = default;

    void setFrame(const sensor_msgs::Image &Frame) override;
    sensor_msgs::Image getProcessedFrame(void) const override;
    bool getDetection(void) const override;
    std::string getResult(void) const override;
    std::vector<std::vector<int>> getCoordinates(void) const override;
};

#endif //BACHELOR_IMAGEPROCESSOR_SPEEDLIMITPROCESSOR_HPP