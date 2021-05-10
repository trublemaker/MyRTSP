/**
 * 李震
 * 我的码云：https://git.oschina.net/git-lizhen
 * 我的CSDN博客：http://blog.csdn.net/weixin_38215395
 * 联系：QQ1039953685
 */

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

//#include <QThread>
//#include <QImage>

#include "wx/wx.h"
#include "SDL.h"

class RTSPMainWnd;
class VideoPlayer : public wxThread
{
public:
	VideoPlayer(RTSPMainWnd* mainwnd) {
		m_mainWnd_ = mainwnd;
	}
    ~VideoPlayer();

   // void setFileName(QString path){mFileName = path;}

	void *Entry(void);

    void sig_GetOneFrame(int); //没获取到一帧图像 就发送此信号

protected:
    void run();

private:
    wxString mFileName;
	RTSPMainWnd* m_mainWnd_;

	SDL_Window *sdl_window;
	SDL_Renderer *sdl_renderer;
	SDL_Texture *sdl_texture;

	SDL_AudioDeviceID m_AudioDevice;
	SDL_AudioSpec wanted_spec, have;

};

#endif // VIDEOPLAYER_H
