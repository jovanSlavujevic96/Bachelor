#include <bachelor/ImageProcessor/StopProcessor.hpp>
#include <cv_bridge/cv_bridge.h> 

#define StopClassifierPath "/home/rtrk/myWS/src/bachelor/cascade/stop_sign.xml"

void StopProcessor::loadCascade(cv::CascadeClassifier *cascade, const int size, const std::string *path)
{
    for(int i=0; i<size; ++i)
    {
        if(!cascade[i].load(path[i]))
        {
            std::cout << "Error while loading cascade from path: " << path[i] << std::endl;
            std::exit(-1);
        }
        std::cout << "Successfully loaded classifier from path: " << path[i] << std::endl;
    }
}

int StopProcessor::resize(cv::Mat &image, const int limit)
{
    int numOfResizing = 0;
    while(image.rows > limit && image.cols > limit)
    {
        cv::resize(image, image, cv::Size(std::round(image.cols/2), std::round(image.rows/2)) );
        ++numOfResizing;
    }
    return numOfResizing;
}

void StopProcessor::crop(cv::Mat &image)
{
    image = image(cv::Rect(0, 0, image.cols, std::round(image.rows/2))).clone();
}

void StopProcessor::redColorSegmentation(const cv::Mat &sample, cv::Mat &result)
{
    cv::Mat hsv;
	cv::cvtColor(sample, hsv, cv::COLOR_BGR2HSV);
    cv::Mat lowerRedHueRange, upperRedHueRange;
	cv::inRange(hsv, cv::Scalar(0, 25, 98), cv::Scalar(11, 39, 144), lowerRedHueRange);
	cv::inRange(hsv, cv::Scalar(156, 30, 79), cv::Scalar(179, 93, 151), upperRedHueRange);
    cv::addWeighted(lowerRedHueRange, 1.0f, upperRedHueRange, 1.0f, 0.0f, result);    
    cv::inRange(hsv, cv::Scalar(0, 25, 60), cv::Scalar(35, 40, 75), lowerRedHueRange);
    cv::addWeighted(result, 1.0f, lowerRedHueRange, 1.0f, 0.0f, result);
    cv::inRange(hsv, cv::Scalar(169, 28, 78), cv::Scalar(173,90, 152), lowerRedHueRange);
    cv::addWeighted(result, 1.0f, lowerRedHueRange, 1.0f, 0.0f, result);
    cv::inRange(hsv, cv::Scalar(157, 28, 58), cv::Scalar(177, 78 , 81 ), lowerRedHueRange);
    cv::addWeighted(result, 1.0f, lowerRedHueRange, 1.0f, 0.0f, result);
    cv::inRange(hsv, cv::Scalar(142, 67, 44), cv::Scalar(165, 102, 104), lowerRedHueRange);
    cv::addWeighted(result, 1.0f, lowerRedHueRange, 1.0f, 0.0f, result);
    cv::inRange(hsv, cv::Scalar(0, 50, 50), cv::Scalar(10, 255, 255), lowerRedHueRange);
    cv::addWeighted(result, 1.0f, lowerRedHueRange, 1.0f, 0.0f, result);
	cv::inRange(hsv, cv::Scalar(160, 50, 50), cv::Scalar(180, 255, 255), lowerRedHueRange);
    cv::addWeighted(result, 1.0f, lowerRedHueRange, 1.0f, 0.0f, result);
    cv::inRange(hsv, cv::Scalar(110, 30, 75), cv::Scalar(160, 75, 120), lowerRedHueRange);
    cv::addWeighted(result, 1.0f, lowerRedHueRange, 1.0f, 0.0f, result);
}

std::vector<cv::Rect> StopProcessor::getRedContours(const cv::Mat1b &hueImage) const
{    
    cv::Mat blured;
	cv::GaussianBlur(hueImage, blured, cv::Size(9,9), 2,2);

    std::vector<cv::Rect> redRects;   
    std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(blured, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point> > contours_poly(contours.size() );
	for(int i = 0; i < contours.size(); ++i)
    {
		cv::approxPolyDP((cv::Mat)contours[i], contours_poly[i], 3, true);
    }
    for(int i = 0; i < contours_poly.size(); ++i)
	{
    	if (cv::contourArea(contours_poly[i]) >= 500 && contours_poly[i].size() >= 8 && contours_poly[i].size() <= 50 )
        {
            redRects.push_back(cv::boundingRect(contours_poly[i]) );
        }
    }
    return redRects; 
}

std::vector<cv::Rect> StopProcessor::getDetectedStopContours(const cv::Mat &image, std::vector<cv::Rect> &contours)
{
    for(int i=0; i<contours.size(); ++i)
    {
        auto *contour = &contours[i];
        auto limit = std::round(contour->width/4);
        for(int rectIncrVal = limit; rectIncrVal >= 1; --rectIncrVal)
        {
            if( (contour->x - rectIncrVal) >= 0 && (contour->y - rectIncrVal) >= 0 && 
                (contour->x + contour->width  + 2*rectIncrVal) <= image.cols && 
                (contour->y + contour->height + 2*rectIncrVal) <= image.rows   )
            {
                contour->x -= rectIncrVal;
                contour->y -= rectIncrVal;
                contour->width  += 2*rectIncrVal;
                contour->height += 2*rectIncrVal;	//increase size of cropped rectangle from the frame
                break;
            }
        }
    }
    std::vector<cv::Rect> stopRect;
    for(int i=0; i<contours.size(); ++i)
	{
		std::vector<cv::Rect> stopFound;
        m_StopClassifier.detectMultiScale( image(contours[i]), stopFound, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(25, 25));
		if( !stopFound.empty() )
		{
			stopFound[0].x +=  contours[i].x;
			stopFound[0].y +=  contours[i].y;
        
			stopRect.push_back( stopFound[0]);
		}	
	}
    return stopRect;
}

std::vector<cv::Mat> StopProcessor::getTextImagesForOCR(const int numOfResizing, std::vector<cv::Rect> &contours)
{   
    std::vector<cv::Mat> images;
    cv::Mat tmpFrame = m_Frame.image.clone();
    StopProcessor::redColorSegmentation(tmpFrame, tmpFrame);
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1.5f, 1));
    for(int i=0; i<contours.size(); ++i)
    {
        for(int j=0; j<numOfResizing; ++j)
        {
            contours[i].x *= 2;
            contours[i].y *= 2;
            contours[i].width *= 2;
            contours[i].height *= 2;
        }
        cv::Mat tmpCrop = tmpFrame(contours[i]).clone();
        
        cv::floodFill(tmpCrop, cv::Point(0,0), cv::Scalar(255));
        cv::erode(tmpCrop, tmpCrop, element);
        
        images.push_back(tmpCrop);
    }
    return images;
}

std::vector<bool> StopProcessor::getDetectionPerRectFromOCR(const std::vector<cv::Mat> &images)
{
    std::vector<bool> detection(images.size() );
    int i=0;
    for(auto image : images)
    {
        detection[i] = false;
        std::string word;
        m_OCR->run(image, word, NULL, NULL, NULL, cv::text::OCR_LEVEL_WORD );
        
        bool STOP[4] = {false};
        int incr = 0;
        for(auto letter : word)
        {
            if( (letter == 's' || letter == 'S' || letter == '$') && !STOP[0])
            {
                STOP[0] = true;
                ++incr;  
            }
            if( (letter == 't' || letter == 'T') && !STOP[1] )
            {
                STOP[1] = true;
                ++incr; 
            }
            if( (letter == 'o' || letter == 'O' || letter == '0') && !STOP[2])
            {
                STOP[2] = true;
                ++incr;
            }
            if( (letter == 'p' || letter == 'P') && !STOP[3])
            {
                STOP[3] = true;
                ++incr;
            }
            if(incr >= 2) 
            {
                detection[i] = true;
                break;
            }  
        }
        ++i;
    }
    return detection;
}

void StopProcessor::setMessages(const std::vector<bool>& detection, const std::vector<cv::Rect>& contours)
{
    if(detection.empty() )
    {
        m_Coordinates.coordinates.push_back( {cv::Point(0,0),cv::Point(0,0)} );
        m_Detection.text[0] = "NO STOP";
        return;
    }

    for(int i=0; i<contours.size(); ++i)
    {
        if(detection[i])
        {
            m_Coordinates.coordinates.push_back({cv::Point(contours[i].x, contours[i].y), cv::Point((contours[i].width+contours[i].x), (contours[i].height+contours[i].y)) });
        }
    }
    m_Detection.text[0] = "STOP";
}

void StopProcessor::drawLocations(cv::Mat &image, const std::vector<bool> &detection, const std::vector<cv::Rect> &contours,
    const cv::Scalar &colorEdge = cv::Scalar(0, 0, 255), const cv::Scalar &colorText = cv::Scalar(255, 0, 255), const std::string &text = "STOP")
{
    cv::Mat helpImage = image.clone();
	for(unsigned int i=0; i<contours.size(); ++i)
    {
        if(detection[i] )
	    {
            cv::rectangle(image, contours[i], colorEdge, -1);
        }
    }
	cv::addWeighted(helpImage, 0.8, image, 0.2, 0, image);
	for(unsigned int i = 0 ; i <contours.size(); ++i) 
    {
        if(detection[i] )
        {
            cv::rectangle(image, contours[i], colorEdge, 3);
            cv::putText(image, text, cv::Point(contours[i].x+1, (contours[i].width+contours[i].y+18)), 
                cv::FONT_HERSHEY_DUPLEX, 0.7f, colorText, 1);
        }
    }
}

StopProcessor::StopProcessor() :
    m_Detected{false}
{
    m_Detection.text = std::vector<std::string>(1);
    const std::string Path = StopClassifierPath;
    StopProcessor::loadCascade(&m_StopClassifier, 1, &Path);
    this->m_OCR = cv::text::OCRTesseract::create(NULL, "eng", "STOP", 1, 6);
}

void StopProcessor::setFrame(const IMessage* frame)
{
    m_Coordinates.coordinates.clear();
    m_Frame = *(static_cast<const ImageMessage*>(frame));

    auto helpImage = m_Frame.image.clone();
    auto numOfResizing = StopProcessor::resize(helpImage, 600); //resize image for faster performance!
    StopProcessor::crop(helpImage); //use only upper half of the image(frame) for faster performance!
    cv::Mat1b redHueImage;  //binary mask
    StopProcessor::redColorSegmentation(helpImage, redHueImage);  //set red hue binary mask
    auto contours = StopProcessor::getRedContours(redHueImage);
    contours = StopProcessor::getDetectedStopContours(helpImage, contours);
    auto images = StopProcessor::getTextImagesForOCR(numOfResizing, contours);
    auto detection = StopProcessor::getDetectionPerRectFromOCR(images);
    StopProcessor::setMessages(detection, contours);
    
    StopProcessor::drawLocations(m_Frame.image, detection, contours);
}

Topic StopProcessor::getWatchdogTopic(void) const
{
    return ImHere_StopDet;
}

Topic StopProcessor::getCoordinateTopic(void) const
{
    return Coord_StopDet;
}

Topic StopProcessor::getECUTopic(void) const
{
    return ECU_StopDet;
}

const IMessage* StopProcessor::getCoordinateMessage(void) const
{
    return &m_Coordinates;
}

const IMessage* StopProcessor::getProcFrameMessage(void) const
{
    return &m_Frame;
}

const IMessage* StopProcessor::getDetectionMessage(void) const
{
    return &m_Detection;
}