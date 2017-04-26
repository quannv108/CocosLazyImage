/****************************************************************************
 Copyright (c) 2016 QuanNguyen
 
 http://quannguyen.info
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __Funny__LazyImageLoader__
#define __Funny__LazyImageLoader__

#include "cocos2d.h"
#include "network/CCDownloader.h"

#define EVENT_LAZY_IMAGE_DONE   "lziml"

class ImageLoaderEvent : public cocos2d::EventCustom {
public:
    ImageLoaderEvent()
    : EventCustom(EVENT_LAZY_IMAGE_DONE)
    , _texture(nullptr)
    {}
    CC_SYNTHESIZE_PASS_BY_REF(std::string, _url, URL);
    CC_SYNTHESIZE(cocos2d::Texture2D *, _texture, Texture);
};

class LazyImageLoader  {
protected:
    LazyImageLoader();
    
    virtual ~LazyImageLoader();
    
    virtual bool init();
    
public:
    static LazyImageLoader* getInstance();
    
public:
    /** load new image
     *  @return true if it will load in lazy
     *  @return false if no loader will be init
     */
    bool loadImage(const std::string& url);
    std::string pathForLoadedImage(const std::string& url);
    std::string convertURLToFilePath(const std::string& url);
    bool replace(std::string& str, const std::string& from, const std::string& to);
    std::vector<std::string> split(const std::string& str, char delimiter);
    
private:
    std::vector<std::string> _loadersIdentifier;
    std::string _writablePath;
    std::string _writePathPrefix;
    bool _useOwnFolder;
    cocos2d::network::Downloader *_downloader;

    void onDownloadTaskDone(const cocos2d::network::DownloadTask& task);
    void onDownloadTaskFailed(const cocos2d::network::DownloadTask& task,
                              int errorCode,
                              int errorCodeInternal,
                              const std::string& errorStr);
    
    void reportLoadDone(const std::string& url, cocos2d::Texture2D *tex);
    
private:
    void createDirectoryForPath(const std::string& path);
};

#endif  /* defined(__Funny__LazyImageLoader__) */
