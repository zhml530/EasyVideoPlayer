#include "VideoPlayOper.h"
#include "OpenGLRender.h"
#include "DecodecVideo.h"
#include "AudioPlayerThread.h"
#include "VideoRenderBase.h"
#include <QTimer>
VideoPlayOper::VideoPlayOper(QObject* parent)
{
    /*m_pTimer = new QTimer(this);
    m_pTimer->setInterval(30);
    QObject::connect(m_pTimer, &QTimer::timeout, this, &VideoPlayOper::onTimeout);*/

    m_pAudioPlayerThread = new AudioPlayerThread(this);
}

VideoPlayOper::~VideoPlayOper()
{

}

void VideoPlayOper::setVideoRender(VideoRenderBase* render)
{
    m_pRender = render;
}

void VideoPlayOper::setVideoDecodec(DecodecVideo* codec)
{
    m_pCodec = codec;
    m_pCodec->setAudioPlayer(m_pAudioPlayerThread);
    m_pAudioPlayerThread->setDecodec(m_pCodec);

    //QObject::connect(m_pCodec, &DecodecVideo::updateDisplay, this, &VideoPlayOper::onTimeout);
	QObject::connect(m_pCodec, &DecodecVideo::updateDisplay, this, &VideoPlayOper::onTimeout2);
}

void VideoPlayOper::play(const QString& name)
{
    bool result = m_pCodec->openVideoFile(name);
    if (!result)
        return;
    
    // Reset VBO
    const DecodecVideo::DecodecVideInfo& videoInfo = m_pCodec->getCurrentInfo();
    m_pRender->rebindVBO(videoInfo.width, videoInfo.height);

    // Reset Audio Info
    int sampleRato, sampleSize, channelSize;
    m_pCodec->getAudioOutputInfos(sampleRato, sampleSize, channelSize);
    m_pAudioPlayerThread->setSampleInfo(sampleRato, sampleSize, channelSize);

    m_pCodec->start();
    //m_pTimer->start();
    m_pAudioPlayerThread->start();
}

void VideoPlayOper::seek(qreal time)
{
	m_pCodec->seekVideo(time);
}

void VideoPlayOper::onTimeout(qreal time)
{
    uchar* pImageData = nullptr;
    m_pCodec->getFrameData(pImageData);
    if (pImageData == nullptr)
        return;

    // set Video Data
    const DecodecVideo::DecodecVideInfo& videoInfo = m_pCodec->getCurrentInfo();
    if (!videoInfo.isYUV420P)
        m_pRender->setRGBData(pImageData, videoInfo.width, videoInfo.height);
    else
    {
        uchar* yuvData[4] = { 0 };
        yuvData[0] = pImageData;
        yuvData[1] = pImageData + videoInfo.width * videoInfo.height;
        yuvData[2] = pImageData + videoInfo.width * videoInfo.height + (videoInfo.width / 2 * videoInfo.height / 2);
        m_pRender->setYUVData(yuvData, videoInfo.width, videoInfo.height);
    }

	//emit updateDisplayInfos(time);
}

void VideoPlayOper::onTimeout2(void)
{
	AVFrame* frame = nullptr;
	m_pCodec->getFrameData(frame);
	if (frame == nullptr)
		return;

	m_pRender->setYUVData(frame->data, frame->width, frame->height);
	m_pCodec->freeFrame(frame);
}
